#include "mysql.h"
#include "xx_object.h"

namespace xx {
	namespace MySql {

		// string 转为各种长度的 有符号整数. Out 取值范围： int8~64
		template <typename OutType>
		void StringToInteger(char const* in, OutType& out) {
			auto in_ = in;
			if (*in_ == '0') {
				out = 0;
				return;
			}
			bool b;
			if (*in_ == '-') {
				b = true;
				++in_;
			}
			else b = false;
			OutType r = *in_ - '0';
			char c;
			while ((c = *(++in_))) r = r * 10 + (c - '0');
			out = b ? -r : r;
		}

		// string (不能有减号打头) 转为各种长度的 无符号整数. Out 取值范围： uint8, uint16, uint32, uint64
		template <typename OutType>
		void StringToUnsignedInteger(char const* in, OutType& out) {
			assert(in);
			auto in_ = in;
			if (*in_ == '0') {
				out = 0;
				return;
			}
			OutType r = *(in_)-'0';
			char c;
			while ((c = *(++in_))) r = r * 10 + (c - '0');
			out = r;
		}

		void FromStringCore(char const* in, uint8_t& out) { StringToUnsignedInteger(in, out); }
		void FromStringCore(char const* in, uint16_t& out) { StringToUnsignedInteger(in, out); }
		void FromStringCore(char const* in, uint32_t& out) { StringToUnsignedInteger(in, out); }
		void FromStringCore(char const* in, uint64_t& out) { StringToUnsignedInteger(in, out); }
		void FromStringCore(char const* in, int8_t& out) { StringToInteger(in, out); }
		void FromStringCore(char const* in, int16_t& out) { StringToInteger(in, out); }
		void FromStringCore(char const* in, int32_t& out) { StringToInteger(in, out); }
		void FromStringCore(char const* in, int64_t& out) { StringToInteger(in, out); }
		void FromStringCore(char const* in, double& out) { out = strtod(in, nullptr); }
		void FromStringCore(char const* in, float& out) { out = (float)strtod(in, nullptr); }
		void FromStringCore(char const* in, bool& out) { out = (in[0] == '1' || in[0] == 'T' || in[0] == 't'); }

		void FromString(uint8_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(uint16_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(uint32_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(uint64_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(int8_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(int16_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(int32_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(int64_t& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(double& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(float& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(bool& dstVar, char const* s) { FromStringCore(s, dstVar); }
		void FromString(std::string& dstVar, char const* s) { dstVar = s; }

		struct Row {
			MYSQL_ROW data = nullptr;
			// todo: more protect

			inline bool IsDBNull(int const& colIdx) const {
				return !data[colIdx];
			}
			inline char const* ReadString(int const& colIdx) const {
				return data[colIdx];
			}

			inline int ReadInt32(int const& colIdx) const {
				int val = 0;
				FromString(val, data[colIdx]);
				return val;
			}

			inline int64_t ReadInt64(int const& colIdx) const {
				int64_t val = 0;
				FromString(val, data[colIdx]);
				return val;
			}

			inline double ReadDouble(int const& colIdx) const {
				double val = 0;
				FromString(val, data[colIdx]);
				return val;
			}

			inline std::pair<char const*, int> ReadText(int const& colIdx) const {
				// todo: 流读取
				return std::make_pair(nullptr, 0);
			}

			inline std::pair<char const*, int> ReadBlob(int const& colIdx) const {
				// todo: 流读取
				return std::make_pair(nullptr, 0);
			}
		};

		struct Reader {
			MYSQL_RES* res = nullptr;
			uint32_t numFields = 0;
			uint64_t numRows = 0;
			// todo: numResults
			// todo: currentResultIndex
			// todo: currentRowIndex
			// todo: mysql_affected_rows()
			void Foreach(std::function<bool(Row const& row)> h) {
				Row row;

				while ((row.data = mysql_fetch_row(res))) {
					if (!h(row)) break;
				}
			}
			// todo: next result
			~Reader() {
				if (res) {
					mysql_free_result(res);
					res = nullptr;
				}
			}
		};

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
			void Query(char const(&sql)[len]) {
				if (!ctx) {
					lastError = "connection is closed.";
					throw - 7;
				}
				if (mysql_real_query(ctx, sql, (unsigned long)len - 1)) {
					lastError = mysql_error(ctx);
					throw - 3;
				}
			}

			void Query(std::string const& sql) {
				if (!ctx) {
					lastError = "connection is closed.";
					throw - 7;
				}
				if (mysql_real_query(ctx, (char*)sql.data(), (unsigned long)sql.size())) {
					lastError = mysql_error(ctx);
					throw - 3;
				}
			}

			void Exec(char const* const& sql, std::function<void(Reader& r)>&& h = nullptr) {
				if (!ctx) {
					lastError = "connection is closed.";
					throw - 7;
				}
				if (mysql_query(ctx, sql)) {
					lastError = mysql_error(ctx);
					throw - 3;
				}
				Reader r;
				r.res = mysql_store_result(ctx);
				if (!r.res) {
					lastError = mysql_error(ctx);
					throw - 4;
				}
				r.numFields = mysql_num_fields(r.res);
				r.numRows = mysql_num_rows(r.res);
				// mysql_fetch_fields()
				// mysql_more_results()
				// mysql_next_result()
				h(r);
			}
		};
	}
}

int main(int argc, char* argv[]) {
	xx::MySql::Connection conn;
	try {
		conn.Open("192.168.1.215", 3306, "root", "1", "test");
		conn.Exec("select 1; select 2;"/*"show tables;"*/, [](xx::MySql::Reader& reader) {
			xx::CoutN("reader.numFields = ", reader.numFields, ", reader.numRows = ", reader.numRows);
			reader.Foreach([&](xx::MySql::Row const& row) {
				for (uint32_t i = 0; i < reader.numFields; ++i) {
					xx::CoutN(row.ReadInt32(i));
				}
				return true;
				});
			});
	}
	catch (int const& e) {
		std::cout << conn.lastError << std::endl;
		return e;
	}
	return 0;
}
