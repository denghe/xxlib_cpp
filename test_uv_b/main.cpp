#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "mysql.h"
#include "xx_object.h"

//int main(int argc, char* argv[]) {
//
//	//std::array<char, 7> s{ "卧槽" };
//	//auto&& s2 = "卧槽";
//	//xx::CoutN(xx::ArrayInfo<decltype(s)>::size);
//	//xx::CoutN(xx::ArrayInfo<decltype(s2)>::size);
//	//xx::CoutN(typeid(xx::ArrayInfo_t<decltype(s)>).name());
//	//xx::CoutN(typeid(xx::ArrayInfo_t<decltype(s2)>).name());
//	//return 0;
//
//	auto&& ms = mysql_init(nullptr);
//	assert(ms);
//	{
//		auto&& r = mysql_real_connect(ms, "192.168.1.215", "root", "1", "test", 3306, nullptr, CLIENT_MULTI_STATEMENTS);
//		assert(r);
//	}
//
//
//	auto&& stmt = mysql_stmt_init(ms);
//	assert(stmt);
//	{
//		std::string sql = "select ?, ?";
//		auto&& r = mysql_stmt_prepare(stmt, (char*)sql.data(), (unsigned long)sql.size());
//		assert(!r);
//	}
//	auto&& numParams = mysql_stmt_param_count(stmt);
//	assert(numParams == 2);
//
//	int c1 = 0;
//
//	std::array<char, 50> c2;
//	unsigned long c2Len = 0;
//	my_bool c2IsNull = 0;
//
//	std::vector<MYSQL_BIND> parmBinds;
//	parmBinds.resize(numParams);
//	memset((char*)parmBinds.data(), 0, sizeof(MYSQL_BIND) * parmBinds.size());
//
//	parmBinds[0].buffer_type = MYSQL_TYPE_LONG;
//	parmBinds[0].buffer = (char*)& c1;
//	parmBinds[0].is_null = 0;
//	parmBinds[0].length = 0;
//
//	parmBinds[1].buffer_type = MYSQL_TYPE_STRING;
//	parmBinds[1].buffer = c2.data();
//	parmBinds[1].buffer_length = (unsigned long)c2.size();
//	parmBinds[1].is_null = &c2IsNull;
//	parmBinds[1].length = &c2Len;
//
//	auto&& r1 = mysql_stmt_bind_param(stmt, parmBinds.data());
//	assert(!r1);
//
//	c1 = 123;
//	c2 = { "asdfqwerzxcv" };
//	c2Len = (unsigned long)strlen(c2.data());
//	c2IsNull = 0;
//
//	//auto&& res = mysql_stmt_result_metadata(stmt);
//	//assert(res);
//	//auto&& numCols = mysql_num_fields(res);
//	//assert(numCols == 2);
//
//	auto&& r3 = mysql_stmt_execute(stmt);
//	assert(!r3);
//
//	int resC1 = 0;
//	std::array<char, 50> resC2;
//
//	std::vector<my_bool> resIsNulls;
//	std::vector<unsigned long> resLens;
//	std::vector<my_bool> resErrors;
//	resLens.resize(2);
//	resIsNulls.resize(2);
//	resErrors.resize(2);
//
//	std::vector<MYSQL_BIND> resBinds;
//	resBinds.resize(2);
//	memset((char*)resBinds.data(), 0, sizeof(MYSQL_BIND) * resBinds.size());
//
//	resBinds[0].buffer_type = MYSQL_TYPE_LONG;
//	resBinds[0].buffer = (char*)& resC1;
//	resBinds[0].is_null = &resIsNulls[0];
//	resBinds[0].length = &resLens[0];
//	resBinds[0].error = &resErrors[0];
//
//	resBinds[1].buffer_type = MYSQL_TYPE_STRING;
//	resBinds[1].buffer = (char*)resC2.data();
//	parmBinds[1].buffer_length = (unsigned long)resC2.size();
//	resBinds[1].is_null = &resIsNulls[1];
//	resBinds[1].length = &resLens[1];
//	resBinds[1].error = &resErrors[1];
//
//	auto&& r4 = mysql_stmt_bind_result(stmt, resBinds.data());
//	assert(!r4);
//
//	auto&& r5 = mysql_stmt_store_result(stmt);
//	assert(!r5);
//
//	auto&& numRows = mysql_stmt_num_rows(stmt);
//	assert(numRows == 1);
//
//	//auto&& affectedRows = mysql_stmt_affected_rows(stmt);
//	//assert(affectedRows == 1);
//
//	int status;
//	while (true) {
//		status = mysql_stmt_fetch(stmt);
//		if (status == 1 || status == MYSQL_NO_DATA) break;
//		xx::CoutN("c1 = ", resC1, "c2 = ", resC2.data());
//	}
//
//	//mysql_free_result(res);
//	mysql_stmt_close(stmt);
//
//
//	return 0;
//}

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

			void Call(char const* const& sql, unsigned long const& len) {
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
			void Call(char const(&sql)[len]) {
				Call(sql, (unsigned long)(len - 1));
			}

			void Call(std::string const& sql) {
				Call((char*)sql.data(), (unsigned long)sql.size());
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
			union {
				int64_t i64 = 0;
				int i32;
				double d;
				unsigned long len;	// buf's data length
			};
			my_bool isNull = false;
			MYSQL_BIND* bind = nullptr;			// 需要 new 后填充
			std::string* lastError = nullptr;	// 需要 new 后填充

			MYSQL_BIND_ex() = default;
			MYSQL_BIND_ex(MYSQL_BIND_ex const&) = delete;
			MYSQL_BIND_ex& operator=(MYSQL_BIND_ex const&) = delete;

			~MYSQL_BIND_ex() {
				Clear();
			}

			void Clear() {
				if (bind->buffer_type >= MYSQL_TYPE_TINY_BLOB && bind->buffer_type <= MYSQL_TYPE_STRING && bind->buffer) {
					free(bind->buffer);
					bind->buffer = nullptr;
				}
			}

			template<typename T>
			inline void Init() {
				Clear();
				memset(bind, 0, sizeof(MYSQL_BIND));

				// 数字
				if (std::is_arithmetic_v<T>) {
					if constexpr (std::is_same_v<T, int>) {
						bind->buffer_type = MYSQL_TYPE_LONG;
						bind->buffer = (char*)& i32;
					}
					else if constexpr (std::is_same_v<T, int64_t>) {
						bind->buffer_type = MYSQL_TYPE_LONGLONG;
						bind->buffer = (char*)& i64;
					}
					else if constexpr (std::is_same_v<T, double>) {
						bind->buffer_type = MYSQL_TYPE_DOUBLE;
						bind->buffer = (char*)& d;
					}
				}
				// 数组
				else if (xx::IsArray_v<T>) {
					size_t bufLen = 0;
					if constexpr (std::is_same_v<ArrayInfo_t<T>, char>) {
						bind->buffer_type = MYSQL_TYPE_STRING;
						bufLen = ArrayInfo_v<T>;
					}
					// todo: else if  .... , uint8_t    type = blob 
					else goto LabError;

					if (!bufLen) goto LabError;

					bind->buffer = malloc(bufLen);
					if (!bind->buffer) {
						lastError->clear();
						Append(*lastError, "buf malloc failed. not enough memory. bufLen = ", std::to_string(bufLen));
						throw - 2;
					}

					bind->buffer_length = (unsigned long)bufLen;
					bind->length = &len;
					len = 0;
				}

				// 可空 数字 | 数组
				else if (xx::IsOptional_v<T>) {
					bind->is_null = (char*)& isNull;
					isNull = false;

					// 提取子类型
					using CT = xx::ChildType_t<T>;
					// 下面这段代码和上面基本一样, 只是 T 变为 CT

					// 数字
					if (std::is_arithmetic_v<CT>) {
						if constexpr (std::is_same_v<CT, int>) {
							bind->buffer_type = MYSQL_TYPE_LONG;
							bind->buffer = (char*)& i32;
						}
						else if constexpr (std::is_same_v<CT, int64_t>) {
							bind->buffer_type = MYSQL_TYPE_LONGLONG;
							bind->buffer = (char*)& i64;
						}
						else if constexpr (std::is_same_v<CT, double>) {
							bind->buffer_type = MYSQL_TYPE_DOUBLE;
							bind->buffer = (char*)& d;
						}
					}
					// 数组
					else if (xx::IsArray_v<CT>) {
						size_t bufLen = 0;
						if constexpr (std::is_same_v<ArrayInfo_t<CT>, char>) {
							bind->buffer_type = MYSQL_TYPE_STRING;
							bufLen = ArrayInfo_v<CT>;
						}
						// todo: else if  .... , uint8_t    type = blob 
						else goto LabError;

						if (!bufLen) goto LabError;

						bind->buffer = malloc(bufLen);
						if (!bind->buffer) {
							lastError->clear();
							Append(*lastError, "buf malloc failed. not enough memory. bufLen = ", std::to_string(bufLen));
							throw - 2;
						}

						bind->buffer_length = (unsigned long)bufLen;
						bind->length = &len;
						len = 0;
					}
				}
				// 未知
				else goto LabError;
				return;

			LabError:
				lastError->clear();
				Append(*lastError, "Init failed. unknown or bad type: ", typeid(T).name());
				throw - 1;
			}

			template<typename T>
			inline void SetParameter(T const& v) {
				if constexpr (std::is_same_v<T, int>) {
					if (bind->buffer_type != MYSQL_TYPE_LONG) goto LabError;
					i32 = v;
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					if (bind->buffer_type != MYSQL_TYPE_LONGLONG) goto LabError;
					i64 = v;
				}
				else if constexpr (std::is_same_v<T, double>) {
					if (bind->buffer_type != MYSQL_TYPE_DOUBLE) goto LabError;
					d = v;
				}
				// more

				else if constexpr (std::is_same_v<T, std::optional<int>>) {
					if (bind->buffer_type != MYSQL_TYPE_LONG) goto LabError;
					isNull = !v.has_value();
					i32 = isNull ? 0 : v.value();
				}
				else if constexpr (std::is_same_v<T, std::optional<int64_t>>) {
					if (bind->buffer_type != MYSQL_TYPE_LONGLONG) goto LabError;
					isNull = !v.has_value();
					i64 = isNull ? 0 : v.value();
				}
				else if constexpr (std::is_same_v<T, std::optional<double>>) {
					if (bind->buffer_type != MYSQL_TYPE_DOUBLE) goto LabError;
					isNull = !v.has_value();
					d = isNull ? 0 : v.value();
				}
				// more

				else if constexpr (std::is_same_v<T, std::string>) {
					if (bind->buffer_type != MYSQL_TYPE_STRING) goto LabError;

					len = v.size();
					if (len > bind->buffer_length) {
						lastError->clear();
						Append(*lastError, "SetParameter failed. buffer_length = ", bind->buffer_length, ", v.size = ", len);
						throw - 2;
					}
					memcpy(bind->buffer, v.data(), len);
				}
				else if constexpr (std::is_same_v<T, std::optional<std::string>>) {
					if (bind->buffer_type != MYSQL_TYPE_STRING) goto LabError;
					isNull = !v.has_value();
					if (isNull) {
						len = 0;
					}
					else {
						len = v.value().size();
						if (len > bind->buffer_length) {
							lastError->clear();
							Append(lastError, "SetParameter failed. buffer_length( ", bind->buffer_length, " ) < v.size( ", v.value().size(), " ).");
							throw - 2;
						}
						v.value().assign(bind->buffer, len);
					}
				}

				// 增加易用性的一些类型. 适配 literal char[]
				else if constexpr (std::is_array_v<T>) {
					using CT = xx::ChildType_t<T>;
					if constexpr (std::is_same_v<CT, char>) {
						if (bind->buffer_type != MYSQL_TYPE_STRING) goto LabError;
						isNull = false;
						
						len = xx::ArrayInfo_v<T>;
						if (len > bind->buffer_length) {
							lastError->clear();
							Append(*lastError, "SetParameter failed. buffer_length = ", bind->buffer_length, ", v.size = ", len);
							throw - 2;
						}
						memcpy(bind->buffer, v, len);
					}
					else {
						xx::CoutN(typeid(CT).name());
						goto LabError;
					}
				}
				// 适配 nullptr. 只要字段可空, 就可以用这个设置.
				else if constexpr (std::is_null_pointer_v<T>) {
					if (!bind->is_null) goto LabError;
					i64 = 0;
					len = 0;
					isNull = true;
				}
				// more

				else goto LabError;
				return;

			LabError:
				lastError->clear();
				Append(*lastError, "SetParameter failed. Init buffer_type is ", bind->buffer_type);
				throw - 1;
			}

			template<typename T>
			inline void GetValue(T& v) {
				// todo
			}
		};

		struct Binds {
			MYSQL_BIND* bind = nullptr;
			MYSQL_BIND_ex* data = nullptr;
			size_t len = 0;
			std::string* lastError;

			Binds(std::string* const& lastError) : lastError(lastError) {}
			Binds() = delete;
			Binds(Binds const&) = delete;
			Binds& operator=(Binds const&) = delete;

			Binds(Binds&& o) noexcept {
				std::swap(this->bind, o.bind);
				std::swap(this->data, o.data);
				std::swap(this->len, o.len);
				std::swap(this->lastError, o.lastError);
			}
			~Binds() {
				Clear();
			}

			void Clear() {
				if (bind) {
					delete[] bind;
					bind = nullptr;
				}
				if (data) {
					delete[] data;
					data = nullptr;
				}
				len = 0;
			}

			Binds& operator=(Binds&& o) noexcept {
				std::swap(this->bind, o.bind);
				std::swap(this->data, o.data);
				std::swap(this->len, o.len);
				std::swap(this->lastError, o.lastError);
				return *this;
			}

			void Resize(size_t const& len) {
				Clear();
				if (len) {
					bind = new MYSQL_BIND[len]();
					memset(bind, 0, sizeof(MYSQL_BIND) * len);

					data = new MYSQL_BIND_ex[len]();

					this->len = len;
					for (size_t i = 0; i < len; ++i) {
						data[i].bind = &bind[i];
						data[i].lastError = lastError;
					}
				}
			}

			//template<typename T>
			//inline void InitParameter(int const& parmIdx) {
			//	if (!data || parmIdx >= len) {
			//		conn.lastError.clear();;
			//		Append(conn.lastError, "parmIdx out of range. len = ", len, ", parmIdx = ", parmIdx);
			//		throw - 1;
			//	}
			//	data[parmIdx].Init<T>();
			//}

			template<typename T>
			inline void SetParameter(int const& parmIdx, T const& v) {
				if (!data || parmIdx >= len) {
					conn.lastError.clear();;
					Append(conn.lastError, "parmIdx out of range. len = ", len, ", parmIdx = ", parmIdx);
					throw - 1;
				}
				data[parmIdx].SetValue<T>(v);
			}

			// 检查是否有参数没初始化或者填充. 有返回下标。没有找到返回 -1
			inline int Verify() {
				for (int i = 0; i < (int)len; ++i) {
					auto&& bind = data[i].bind;
					if (!bind->buffer) {
						return i;
					}
				}
				return -1;
			}

			template<typename T>
			inline void Read(int const& colIdx, T& v) {
				if (!data || parmIdx >= len) {
					conn.lastError.clear();;
					Append(conn.lastError, "colIdx out of range. len = ", len, ", colIdx = ", colIdx);
					throw - 1;
				}
				data[parmIdx].GetValue<T>(v);
			}

		};

		struct Results;
		struct Query : Binds {
			Connection& conn;
			MYSQL_STMT* stmt = nullptr;
			unsigned long affected_rows;

			operator bool() {
				return stmt;
			}
			Query(Connection& conn)
				: Binds(&conn.lastError)
				, conn(conn) {
			}

			void Clear() {
				if (stmt) {
					mysql_stmt_close(stmt);	// 暂时忽略这个语句错误
					stmt = nullptr;
				}
			}

			~Query() {
				Clear();
			}

			template<typename ...Parms>
			inline void SetQuery(char const* const& sql, unsigned long const& sqlLen) {
				Clear();
				stmt = mysql_stmt_init(conn.ctx);
				if (!stmt) {
					conn.lastError = "stat init failed.";
					throw - 1;
				}
				if (mysql_stmt_prepare(stmt, sql, sqlLen ? sqlLen : (unsigned long)strlen(sql))) {
					conn.lastError = mysql_stmt_error(stmt);
					Clear();
					throw - 2;
				}
				auto&& numParams = mysql_stmt_param_count(stmt);
				if (sizeof...(Parms) != numParams) {
					Clear();
					conn.lastError.clear();
					Append(conn.lastError, "wrong numParms! sizeof...(Parms) = ", sizeof...(Parms), ", numParams = ", numParams);
					throw - 3;
				}
				Resize(numParams);

				int idx = 0;
				std::initializer_list<int> n{ (data[idx++].Init<Parms>(), 0)... };
				(void)n;
			}

			template<size_t len, typename ...Parms>
			inline void SetQuery(char const(&sql)[len]) {
				SetQuery<Parms...>(sql, len - 1);
			}

			template<typename ...Parms>
			inline void SetQuery(std::string const& sql) {
				SetQuery<Parms...>((char*)sql.data(), (unsigned long)sql.size());
			}

			template<typename ...Args>
			inline Query& SetParameters(Args const& ... args) {
				if (sizeof...(Args) != len) {
					conn.lastError.clear();
					Append(conn.lastError, "bad sizeof(args) : ", sizeof...(Args), ". expect ", len);
					throw - 1;
				}
				int idx = 0;
				std::initializer_list<int> n{ (data[idx++].SetParameter<Args>(args), 0)... };
				(void)n;
				return *this;
			}

			// todo: 通过传入回调, 得到 Results 对象. 并继续通过它 检索结果, 跳转结果集. 检索前需要用 类似 Init 的方式，格式化结果集的填充 bind 容器
			Query& Execute(std::function<void(Results& rs)>&& h = nullptr);

			Query& operator()() {
				// todo
				return *this;
			}
		};

		struct Reader : Binds {
			int numCols = 0;			// 字段个数
			int currResultIndex = 0;	// 当前正在读的结果集下标
			using Binds::Binds;

			//// 一组结构检索函数( 可能简化或者增加更细节的查询 )
			//int GetColumnCount();
			//enum_field_types GetColumnDataType(int const& colIdx);
			//char const* GetColumnName(int const& colIdx);
			//int GetColumnIndex(char const* const& colName);
			//int GetColumnIndex(std::string const& colName);

			// todo: colIdx 范围校验, 数据类型校验
			// 一组数据访问函数
			bool IsDBNull(int const& colIdx) {
				return data[colIdx].isNull;
			}
			int ReadInt32(int const& colIdx) {
				return data[colIdx].i32;
			}
			int64_t ReadInt64(int const& colIdx) {
				return data[colIdx].i64;
			}
			double ReadDouble(int const& colIdx) {
				return data[colIdx].d;
			}
			std::string ReadString(int const& colIdx) {
				return std::string((char*)bind[colIdx].buffer, data[colIdx].len);
			}
			//std::pair<char const*, int> ReadText(int const& colIdx);
			//std::pair<char const*, int> ReadBlob(int const& colIdx);

			// 填充
			//template<typename T>
			//void Read(int const& colIdx, T& outVal);
			//template<typename T>
			//void Read(char const* const& colName, T& outVal);
			//template<typename T>
			//void Read(std::string const& colName, T& outVal);

		//	// 一次填充多个
		//	template<typename...Args>
		//	void Reads(Args& ...args);
		//protected:
		//	template<typename Arg, typename...Args>
		//	void ReadsCore(int& colIdx, Arg& arg, Args& ...args);
		//	void ReadsCore(int& colIdx);
		};

		struct Results {
			MYSQL_STMT* stmt;
			std::string* lastError;
			int numResultSets = 0;		// 结果集个数
			Reader reader;

			Results(MYSQL_STMT* const& stmt, std::string* const& lastError)
				: stmt(stmt)
				, lastError(lastError)
				, reader(lastError) {
			}

			// 访问一个结果集
			template<typename ...Fields>
			inline void Fetch(std::function<bool(Reader&)>&& h) {
				static_assert(sizeof...(Fields), "must be specify fields types.");
				
				reader.numCols = sizeof...(Fields);
				// todo: fill fields? get field count? verify?
				reader.Resize(sizeof...(Fields));

				int idx = 0;
				std::initializer_list<int> n{ (reader.data[idx++].Init<Fields>(), 0)... };
				(void)n;

				while (mysql_stmt_fetch(stmt)) {
					if (!h(reader)) break;
				}
			}
		};

		inline Query& Query::Execute(std::function<void(Results& rs)>&& h) {
			if (mysql_stmt_bind_param(stmt, bind)) {
				conn.lastError = mysql_stmt_error(stmt);
				throw - 1;
			}
			if (mysql_stmt_execute(stmt)) {
				conn.lastError = mysql_stmt_error(stmt);
				throw - 1;
			}
			if (h) {
				Results rs(stmt, &conn.lastError);
				h(rs);
			}
			return *this;
		}
	}
}


int main(int argc, char* argv[]) {
	xx::MySql::Connection conn;
	//try {
		conn.Open("192.168.1.215", 3306, "root", "1", "test");
		xx::MySql::Query q(conn);
		q.SetQuery<int, char[5]>("select ?, ?");
		q.SetParameters(123, "asdf");
		q.Execute([](xx::MySql::Results& rs) {
			int c1;
			std::string c2;
			rs.Fetch<int, char[5]>([&](xx::MySql::Reader& r) {
				//r.Reads(c1, c2);
				c1 = r.ReadInt32(0);
				c2 = r.ReadString(1);
				return true;
			});
			xx::CoutN(c1,", ", c2);
		});
	//}
	//catch (int const& e) {
	//	std::cout << conn.lastError << std::endl;
	//	return e;
	//}
	return 0;
}

//		conn.Exec("select 1; select 2;"/*"show tables;"*/, [](xx::MySql::Reader& reader) {
//			xx::CoutN("reader.numFields = ", reader.numFields, ", reader.numRows = ", reader.numRows);
//			reader.Foreach([&](xx::MySql::Row const& row) {
//				for (uint32_t i = 0; i < reader.numFields; ++i) {
//					xx::CoutN(row.ReadInt32(i));
//				}
//				return true;
//				});
//			});


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