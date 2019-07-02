#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "mysql.h"
#include "xx_object.h"

int main(int argc, char* argv[]) {

	auto&& ms = mysql_init(nullptr);
	assert(ms);
	{
		auto&& r = mysql_real_connect(ms, "192.168.1.215", "root", "1", "test", 3306, nullptr, CLIENT_MULTI_STATEMENTS);
		assert(r);
	}


	auto&& stmt = mysql_stmt_init(ms);
	assert(stmt);
	{
		std::string sql = "select ?, ?";
		auto&& r = mysql_stmt_prepare(stmt, (char*)sql.data(), (unsigned long)sql.size());
		assert(!r);
	}
	auto&& numParams = mysql_stmt_param_count(stmt);
	assert(numParams == 2);

	int c1 = 0;

	std::array<char, 50> c2;
	unsigned long c2Len = 0;
	my_bool c2IsNull = 0;

	std::vector<MYSQL_BIND> parmBinds;
	parmBinds.resize(numParams);
	memset((char*)parmBinds.data(), 0, sizeof(MYSQL_BIND) * parmBinds.size());

	parmBinds[0].buffer_type = MYSQL_TYPE_LONG;
	parmBinds[0].buffer = (char*)& c1;
	parmBinds[0].is_null = 0;
	parmBinds[0].length = 0;

	parmBinds[1].buffer_type = MYSQL_TYPE_STRING;
	parmBinds[1].buffer = c2.data();
	parmBinds[1].buffer_length = (unsigned long)c2.size();
	parmBinds[1].is_null = &c2IsNull;
	parmBinds[1].length = &c2Len;

	auto&& r1 = mysql_stmt_bind_param(stmt, parmBinds.data());
	assert(!r1);

	c1 = 123;
	c2 = { "asdfqwerzxcv" };
	c2Len = (unsigned long)strlen(c2.data());
	c2IsNull = 0;

	//auto&& res = mysql_stmt_result_metadata(stmt);
	//assert(res);
	//auto&& numCols = mysql_num_fields(res);
	//assert(numCols == 2);

	auto&& r3 = mysql_stmt_execute(stmt);
	assert(!r3);

	int resC1 = 0;
	std::array<char, 50> resC2;

	std::vector<my_bool> resIsNulls;
	std::vector<unsigned long> resLens;
	std::vector<my_bool> resErrors;
	resLens.resize(2);
	resIsNulls.resize(2);
	resErrors.resize(2);

	std::vector<MYSQL_BIND> resBinds;
	resBinds.resize(2);
	memset((char*)resBinds.data(), 0, sizeof(MYSQL_BIND) * resBinds.size());

	resBinds[0].buffer_type = MYSQL_TYPE_LONG;
	resBinds[0].buffer = (char*)& resC1;
	resBinds[0].is_null = &resIsNulls[0];
	resBinds[0].length = &resLens[0];
	resBinds[0].error = &resErrors[0];

	resBinds[1].buffer_type = MYSQL_TYPE_STRING;
	resBinds[1].buffer = (char*)resC2.data();
	parmBinds[1].buffer_length = (unsigned long)resC2.size();
	resBinds[1].is_null = &resIsNulls[1];
	resBinds[1].length = &resLens[1];
	resBinds[1].error = &resErrors[1];

	auto&& r4 = mysql_stmt_bind_result(stmt, resBinds.data());
	assert(!r4);

	auto&& r5 = mysql_stmt_store_result(stmt);
	assert(!r5);

	auto&& numRows = mysql_stmt_num_rows(stmt);
	assert(numRows == 1);

	//auto&& affectedRows = mysql_stmt_affected_rows(stmt);
	//assert(affectedRows == 1);

	int status;
	while (true) {
		status = mysql_stmt_fetch(stmt);
		if (status == 1 || status == MYSQL_NO_DATA) break;
		xx::CoutN("c1 = ", resC1, "c2 = ", resC2.data());
	}

	//mysql_free_result(res);
	mysql_stmt_close(stmt);


	return 0;
}

namespace xx {
	namespace MySql {
		//struct Row {
		//	MYSQL_ROW data = nullptr;
		//	unsigned long* lengths = nullptr;
		//	// todo: more protect
		//
		//	inline bool IsDBNull(int const& colIdx) const {
		//		return !data[colIdx];
		//	}
		//	inline char const* ReadString(int const& colIdx) const {
		//		return data[colIdx];
		//	}
		//
		//	inline int ReadInt32(int const& colIdx) const {
		//		return atoi(data[colIdx]);
		//	}
		//
		//	inline int64_t ReadInt64(int const& colIdx) const {
		//		return atoll(data[colIdx]);
		//	}
		//
		//	inline double ReadDouble(int const& colIdx) const {
		//		return atof(data[colIdx]);
		//	}
		//
		//	inline std::pair<char const*, int> ReadText(int const& colIdx) const {
		//		// todo: 流读取
		//		return std::make_pair(nullptr, 0);
		//	}
		//
		//	inline std::pair<char const*, int> ReadBlob(int const& colIdx) const {
		//		// todo: 流读取
		//		return std::make_pair(nullptr, 0);
		//	}
		//};
		//
		//struct Reader {
		//	MYSQL_RES* res = nullptr;
		//	uint32_t numFields = 0;
		//	uint64_t numRows = 0;
		//	// todo: numResults
		//	// todo: currentResultIndex
		//	// todo: currentRowIndex
		//	// todo: mysql_affected_rows()
		//	void Foreach(std::function<bool(Row const& row)> h) {
		//		Row row;
		//		while ((row.data = mysql_fetch_row(res))) {
		//			row.lengths = mysql_fetch_lengths(res);
		//			if (!row.lengths) {
		//				// todo: error handle
		//				break;
		//			}
		//			if (!h(row)) break;
		//		}
		//	}
		//	// todo: next result
		//	~Reader() {
		//		if (res) {
		//			mysql_free_result(res);
		//			res = nullptr;
		//		}
		//	}
		//};

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
					throw - 5;
				}
				ctx = mysql_init(nullptr);
				if (!ctx) {
					lastError = "mysql_init failed.";
					throw - 9;
				}
				if (!mysql_real_connect(ctx, host, username, password, db, port, nullptr, CLIENT_MULTI_STATEMENTS)) {	// todo: 关 SSL 的参数
					lastError = mysql_error(ctx);
					throw - 2;
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

			template<size_t len>
			void Call(char const(&sql)[len]) {
				if (!ctx) {
					lastError = "connection is closed.";
					throw - 7;
				}
				if (mysql_real_query(ctx, sql, (unsigned long)len - 1)) {
					lastError = mysql_error(ctx);
					throw - 3;
				}
			}

			void Call(std::string const& sql) {
				if (!ctx) {
					lastError = "connection is closed.";
					throw - 7;
				}
				if (mysql_real_query(ctx, (char*)sql.data(), (unsigned long)sql.size())) {
					lastError = mysql_error(ctx);
					throw - 3;
				}
			}

			//void Exec(char const* const& sql, std::function<void(Reader& r)>&& h = nullptr) {
			//	if (!ctx) {
			//		lastError = "connection is closed.";
			//		throw - 7;
			//	}
			//	if (mysql_query(ctx, sql)) {
			//		lastError = mysql_error(ctx);
			//		throw - 3;
			//	}
			//	Reader r;
			//	r.res = mysql_store_result(ctx);
			//	if (!r.res) {
			//		lastError = mysql_error(ctx);
			//		throw - 4;
			//	}
			//	r.numFields = mysql_num_fields(r.res);
			//	r.numRows = mysql_num_rows(r.res);
			//	// mysql_fetch_fields()
			//	// mysql_more_results()
			//	// mysql_next_result()
			//	h(r);
			//}
		};

		// 只适合 new[], delete[]. 不可移动, 复制. 避免指针指向无效内存地址.
		struct MYSQL_BIND_ex {
			MYSQL_BIND bind;
			union {
				int64_t i64 = 0;
				int i32;
				double d;
				unsigned long len;	// buf's data length
			};
			my_bool isNull = false;
			Connection* conn = nullptr;	// 需要 new 后填充

			MYSQL_BIND_ex() {
				static_assert(!&(*(MYSQL_BIND_ex*)0).bind);	// 确保 bind 成员和 this 指针头部一致
				memset(&bind, 0, sizeof(bind));
			}
			MYSQL_BIND_ex(MYSQL_BIND_ex const&) = delete;
			MYSQL_BIND_ex& operator=(MYSQL_BIND_ex const&) = delete;

			~MYSQL_BIND_ex() {
				Clear();
			}

			void Clear() {
				if (bind.buffer_type >= MYSQL_TYPE_TINY_BLOB && bind.buffer_type <= MYSQL_TYPE_STRING && bind.buffer) {
					free(bind.buffer);
				}
			}

			template<typename T>
			inline void Init(unsigned long const& bufLen = 0) {
				Clear();
				memset(&bind, 0, sizeof(bind));

				if constexpr (std::is_same_v<T, int>) {
					bind.buffer_type = MYSQL_TYPE_LONG;
					bind.buffer = (char*)& i32;
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					bind.buffer_type = MYSQL_TYPE_LONGLONG;
					bind.buffer = (char*)& i64;
				}
				else if constexpr (std::is_same_v<T, double>) {
					bind.buffer_type = MYSQL_TYPE_DOUBLE;
					bind.buffer = (char*)& d;
				}
				// more

				else if constexpr (std::is_same_v<T, std::optional<int>>) {
					bind.buffer_type = MYSQL_TYPE_LONG;
					bind.buffer = (char*)& i32;
					bind.is_null = (char*)& isNull;
				}
				else if constexpr (std::is_same_v<T, std::optional<int64_t>>) {
					bind.buffer_type = MYSQL_TYPE_LONGLONG;
					bind.buffer = (char*)& i64;
					bind.is_null = (char*)& isNull;
				}
				else if constexpr (std::is_same_v<T, std::optional<double>>) {
					bind.buffer_type = MYSQL_TYPE_DOUBLE;
					bind.buffer = (char*)& d;
					bind.is_null = (char*)& isNull;
				}
				// more

				else {
					if (!bufLen) {
						conn->lastError = "Init: zero bufLen";
						throw - 1;
					}
					Clear();
					memset(&bind, 0, sizeof(bind));

					bind.buffer = malloc(bufLen);
					if (!bind.buffer) {
						conn->lastError = "buf malloc failed. not enough memory. bufLen = ";
						conn->lastError += std::to_string(bufLen);
						throw - 2;
					}

					bind.buffer_length = (unsigned long)bufLen;
					bind.length = &len;
					len = 0;
					isNull = false;

					if constexpr (std::is_same_v<T, std::string>) {
						bind.buffer_type = MYSQL_TYPE_STRING;
					}
					else if constexpr (std::is_same_v<T, std::optional<std::string>>) {
						bind.buffer_type = MYSQL_TYPE_STRING;
						bind.is_null = &isNull;
					}
					else {
						static_assert(false, "unsupport type");
					}
				}
			}

			template<typename T>
			inline void SetValue(T const& v) {
				if constexpr (std::is_same_v<T, int>) {
					if (bind.buffer_type != MYSQL_TYPE_LONG) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					i32 = v;
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					if (bind.buffer_type != MYSQL_TYPE_LONGLONG) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					i64 = v;
				}
				else if constexpr (std::is_same_v<T, double>) {
					if (bind.buffer_type != MYSQL_TYPE_DOUBLE) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					d = v;
				}
				// more

				else if constexpr (std::is_same_v<T, std::optional<int>>) {
					if (bind.buffer_type != MYSQL_TYPE_LONG) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					if (v.has_value()) {
						i32 = v.value();
						isNull = false;
					}
					else {
						i32 = 0;
						isNull = true;
					}
				}
				else if constexpr (std::is_same_v<T, std::optional<int64_t>>) {
					if (bind.buffer_type != MYSQL_TYPE_LONGLONG) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					if (v.has_value()) {
						i64 = v.value();
						isNull = false;
					}
					else {
						i64 = 0;
						isNull = true;
					}
				}
				else if constexpr (std::is_same_v<T, std::optional<double>>) {
					if (bind.buffer_type != MYSQL_TYPE_DOUBLE) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					if (v.has_value()) {
						d = v.value();
						isNull = false;
					}
					else {
						d = 0;
						isNull = true;
					}
				}
				// more

				else if constexpr (std::is_same_v<T, std::string>) {
					if (bind.buffer_type != MYSQL_TYPE_STRING) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					len = v.size();
					if (len > bind.buffer_length) {
						len = 0;
						conn->lastError.clear();
						xx::Append(conn->lastError, "SetValue failed. buffer_length( ", bind.buffer_length, " ) < v.size( ", v.size(), " ).");
						throw - 2;
					}
					memcpy(bind.buffer, v.data(), len);
				}
				else if constexpr (std::is_same_v<T, std::optional<std::string>>) {
					if (bind.buffer_type != MYSQL_TYPE_STRING) {
						conn->lastError = "SetValue failed. buffer_type is ";
						conn->lastError += std::to_string(bind.buffer_type);
						throw - 1;
					}
					if (v.has_value()) {
						len = v.value().size();
						if (len > bind.buffer_length) {
							len = 0;
							conn->lastError.clear();
							xx::Append(conn->lastError, "SetValue failed. buffer_length( ", bind.buffer_length, " ) < v.size( ", v.value().size(), " ).");
							throw - 2;
						}
						memcpy(bind.buffer, v.value().data(), len);
						isNull = false;
					}
					else {
						len = 0;
						isNull = true;
					}
				}
				// more

				else {
					conn->lastError = "Init: unsupoorted data type";
					throw - 1;
				}
			}
		};

		struct Binds {
			Connection& conn;
			MYSQL_BIND_ex* binds = nullptr;
			size_t bindsLen = 0;

			Binds(Connection& conn) : conn(conn) {}
			Binds() = delete;
			Binds(Binds const&) = delete;
			Binds& operator=(Binds const&) = delete;

			Binds(Binds&& o) noexcept
				: conn(o.conn) {
				std::swap(this->binds, o.binds);
				std::swap(this->bindsLen, o.bindsLen);
			}
			~Binds() {
				Clear();
			}

			void Clear() {
				if (binds) {
					delete[] binds;
					binds = nullptr;
					bindsLen = 0;
				}
			}

			Binds& operator=(Binds&& o) noexcept {
				std::swap(this->conn, o.conn);
				std::swap(this->binds, o.binds);
				std::swap(this->bindsLen, o.bindsLen);
				return *this;
			}

			void Resize(size_t const& len) {
				Clear();
				if (len) {
					binds = new MYSQL_BIND_ex[len]();
					bindsLen = len;
					for (size_t i = 0; i < len; ++i) {
						binds[i].conn = &conn;
					}
				}
			}

			template<typename T>
			inline void Init(int const& parmIdx, unsigned long const& bufLen) {
				if (!binds || parmIdx >= bindsLen) {
					conn.lastError.clear();;
					xx::Append(conn.lastError, "parmIdx out of range. bindsLen = ", bindsLen, ", parmIdx = ", parmIdx);
					throw - 1;
				}
				binds[parmIdx].Init<T>(bufLen);
			}

			template<typename T>
			inline void SetValue(int const& parmIdx, T const& v) {
				if (!binds || parmIdx >= bindsLen) {
					conn.lastError.clear();;
					xx::Append(conn.lastError, "parmIdx out of range. bindsLen = ", bindsLen, ", parmIdx = ", parmIdx);
					throw - 1;
				}
				binds[parmIdx].SetValue<T>(v);
			}

			// 检查是否有参数没初始化或者填充. 有返回下标。没有找到返回 -1
			inline int Verify() {
				for (int i = 0; i < (int)bindsLen; ++i) {
					auto&& bind = binds[i].bind;
					if (!bind.buffer) {
						return i;
					}
				}
				return -1;
			}
		};

		struct Query {
			Connection& conn;
			MYSQL_STMT* stmt = nullptr;
			Binds parms;
			Binds rtvs;

			operator bool() {
				return stmt;
			}
			Query(Connection& conn)
				: conn(conn)
				, parms(conn)
				, rtvs(conn)
			{
			}

			void Clear() {
				if (stmt) {
					mysql_stmt_close(stmt);
					stmt = nullptr;
				}
			}

			~Query() {
				Clear();
			}

			inline void SetQuery(char const* const& sql, unsigned long const& sqlLen) {
				Clear();
				stmt = mysql_stmt_init(conn.ctx);
				if (!stmt) {
					conn.lastError = "stat init failed.";
					throw - 1;
				}
				if (mysql_stmt_prepare(stmt, sql, sqlLen)) {
					Clear();
					conn.lastError = mysql_stmt_error(stmt);
					throw - 2;
				}
				auto&& numParams = mysql_stmt_param_count(stmt);
				parms.Resize(numParams);
			}

			// todo
		};
	}
}



//int main(int argc, char* argv[]) {
//	xx::MySql::Connection conn;
//	try {
//		conn.Open("192.168.1.215", 3306, "root", "1", "test");
//		conn.Exec("select 1; select 2;"/*"show tables;"*/, [](xx::MySql::Reader& reader) {
//			xx::CoutN("reader.numFields = ", reader.numFields, ", reader.numRows = ", reader.numRows);
//			reader.Foreach([&](xx::MySql::Row const& row) {
//				for (uint32_t i = 0; i < reader.numFields; ++i) {
//					xx::CoutN(row.ReadInt32(i));
//				}
//				return true;
//				});
//			});
//	}
//	catch (int const& e) {
//		std::cout << conn.lastError << std::endl;
//		return e;
//	}
//	return 0;
//}



//// string 转为各种长度的 有符号整数. Out 取值范围： int8~64
//template <typename OutType>
//void StringToInteger(char const* in, OutType& out) {
//	auto in_ = in;
//	if (*in_ == '0') {
//		out = 0;
//		return;
//	}
//	bool b;
//	if (*in_ == '-') {
//		b = true;
//		++in_;
//	}
//	else b = false;
//	OutType r = *in_ - '0';
//	char c;
//	while ((c = *(++in_))) r = r * 10 + (c - '0');
//	out = b ? -r : r;
//}

//// string (不能有减号打头) 转为各种长度的 无符号整数. Out 取值范围： uint8, uint16, uint32, uint64
//template <typename OutType>
//void StringToUnsignedInteger(char const* in, OutType& out) {
//	assert(in);
//	auto in_ = in;
//	if (*in_ == '0') {
//		out = 0;
//		return;
//	}
//	OutType r = *(in_)-'0';
//	char c;
//	while ((c = *(++in_))) r = r * 10 + (c - '0');
//	out = r;
//}

//void FromStringCore(char const* in, uint8_t& out) { StringToUnsignedInteger(in, out); }
//void FromStringCore(char const* in, uint16_t& out) { StringToUnsignedInteger(in, out); }
//void FromStringCore(char const* in, uint32_t& out) { StringToUnsignedInteger(in, out); }
//void FromStringCore(char const* in, uint64_t& out) { StringToUnsignedInteger(in, out); }
//void FromStringCore(char const* in, int8_t& out) { StringToInteger(in, out); }
//void FromStringCore(char const* in, int16_t& out) { StringToInteger(in, out); }
//void FromStringCore(char const* in, int32_t& out) { StringToInteger(in, out); }
//void FromStringCore(char const* in, int64_t& out) { StringToInteger(in, out); }
//void FromStringCore(char const* in, double& out) { out = strtod(in, nullptr); }
//void FromStringCore(char const* in, float& out) { out = (float)strtod(in, nullptr); }
//void FromStringCore(char const* in, bool& out) { out = (in[0] == '1' || in[0] == 'T' || in[0] == 't'); }

//void FromString(uint8_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(uint16_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(uint32_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(uint64_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(int8_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(int16_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(int32_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(int64_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(double& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(float& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(bool& dstVar, char const* s) { FromStringCore(s, dstVar); }
//void FromString(std::string& dstVar, char const* s) { dstVar = s; }