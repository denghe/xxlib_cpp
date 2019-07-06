#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#ifdef _WIN32
#include "mysql.h"
#else
#include <mariadb/mysql.h>
#endif
#include "xx_object.h"


struct Info {
	unsigned int numFields = 0;
	my_ulonglong numRows = 0;
	MYSQL_FIELD* fields = nullptr;
};

struct Reader {
	Info& info;
	MYSQL_ROW data = nullptr;
	unsigned long* lengths = nullptr;

	Reader(Info& info)
		: info(info) {
	}

	inline bool IsDBNull(int const& colIdx) const {
		return !data[colIdx];
	}

	inline int ReadInt32(int const& colIdx) const {
		return atoi(data[colIdx]);
	}

	inline int64_t ReadInt64(int const& colIdx) const {
		return atoll(data[colIdx]);
	}

	inline double ReadDouble(int const& colIdx) const {
		return atof(data[colIdx]);
	}

	inline std::string ReadString(int const& colIdx) const {
		return std::string(data[colIdx], lengths[colIdx]);
	}

	// todo: BBuffer?

	// todo: more Read<T> for easy use
};

// mysql 数据库连接对象
struct Connection {
	MYSQL* ctx = nullptr;
	std::string lastError;

	operator bool() {
		return ctx;
	}

	Connection() = default;
	~Connection() {
		Close();
	}

	void Open(char const* const& host, int const& port, char const* const& username, char const* const& password, char const* const& db) {
		if (ctx) {
			lastError = "connection is opened.";
			throw - 1;
		}
		ctx = mysql_init(nullptr);
		if (!ctx) {
			lastError = "mysql_init failed.";
			throw - 2;
		}
		if (!mysql_real_connect(ctx, host, username, password, db, port, nullptr, CLIENT_MULTI_STATEMENTS)) {	// todo: 关 SSL 的参数
			lastError = mysql_error(ctx);
			throw - 3;
		}
	}

	void Close() {
		if (ctx) {
			mysql_close(ctx);
			ctx = nullptr;
		}
	}

	int Ping() {
		if (!ctx) return 0;
		return mysql_ping(ctx);
	}

	// 执行一段 SQL 脚本. 后续使用 Fetch 检索返回结果( 如果有的话 )
	void Execute(char const* const& sql, unsigned long const& len) {
		if (!ctx) {
			lastError = "connection is closed.";
			throw - 1;
		}
		if (mysql_real_query(ctx, sql, len)) {
			lastError = mysql_error(ctx);
			throw - 2;
		}
	}

	template<size_t len>
	void Execute(char const(&sql)[len]) {
		Execute(sql, (unsigned long)(len - 1));
	}

	void Execute(std::string const& sql) {
		Execute((char*)sql.data(), (unsigned long)sql.size());
	}


	// 填充一个结果集, 并产生相应回调. 返回 是否存在下一个结果集
	// infoHandler 返回 true 将继续对每行数据发起 rowHandler 调用. 返回 false, 将终止调用
	bool Fetch(std::function<bool(Info&)>&& infoHandler, std::function<bool(Reader&)>&& rowHandler) {
		if (!ctx) {
			lastError = "connection is closed.";
			throw - 1;
		}

		Info info;

		// 存储结果集
		auto&& res = mysql_store_result(ctx);

		// 有结果集
		if (res) {
			// 确保 res 在出这层 {} 时得到回收
			xx::ScopeGuard sgResult([&] {
				mysql_free_result(res);
				});

			// 各种填充
			info.numFields = mysql_num_fields(res);
			info.numRows = mysql_num_rows(res);
			info.fields = mysql_fetch_fields(res);

			// 如果确定要继续读, 且具备读函数, 就遍历并调用
			if (rowHandler && (!infoHandler || (infoHandler && infoHandler(info)))) {
				Reader reader(info);
				while ((reader.data = mysql_fetch_row(res))) {
					reader.lengths = mysql_fetch_lengths(res);
					if (!reader.lengths) {
						lastError = mysql_error(ctx);
						throw - 2;
					}
					if (!rowHandler(reader)) break;
				}
			}
		}
		// 没有结果有两种可能：1. 真没有. 2. 有但是内存爆了网络断了之类. 用获取字段数量方式推断是否应该返回结果集
		else  if (!mysql_field_count(ctx)) {
			// numRows 存储受影响行数
			info.numRows = mysql_affected_rows(ctx);
			if (infoHandler) {
				infoHandler(info);
			}
		}
		else {
			// 有结果集 但是出错
			lastError = mysql_error(ctx);
			throw - 3;
		}

		// 0: 有更多结果集.  -1: 没有    >0: 出错
		auto&& n = mysql_next_result(ctx);
		if (!n) return true;
		else if (n == -1) return false;
		else {
			lastError = mysql_error(ctx);
			throw - 4;
		}
		return false;
	};
};

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	Connection conn;
	try {
		conn.Open("192.168.1.230", 3306, "root", "Abc123", "test");
		auto&& t = std::chrono::steady_clock::now();
		int s;
		for (int i = 0; i < 10000; ++i) {
			conn.Execute("select 123");
			conn.Fetch(nullptr, [&](Reader& r) {
				s = r.ReadInt32(0);
				return true;
				});
		}
		xx::CoutN(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count());
		xx::CoutN(s);
	}
	catch (int const& e) {
		std::cout << conn.lastError << std::endl;
		return e;
	}
	return 0;
}
