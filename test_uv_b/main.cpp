#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#ifdef _WIN32
#include "mysql.h"
#else
#include <mariadb/mysql.h>
#endif
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

		// 引用到已有字串 方便拼接时自动转义.
		struct Escaped {
			MYSQL* ctx;
			char const* buf = nullptr;
			size_t len = 0;

			Escaped(MYSQL* ctx) : ctx(ctx) {}
			Escaped() = delete;
			Escaped(Escaped const&) = delete;
			Escaped& operator=(Escaped const&) = delete;

			Escaped& operator=(std::string const& s) {
				buf = (char*)s.data();
				len = s.size();
				return *this;
			}
		
			// todo: 得测试下看看 s 会不会空
			template<size_t len>
			Escaped& operator=(char const(&s)[len]) {
				buf = s;
				this->len = len - 1;
				return *this;
			}
		};
	}

	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_base_of_v<xx::MySql::Escaped, T>>> {
		static inline void WriteTo(std::string& s, T const& in) noexcept {
			auto&& len = s.size();
			s.resize(len + in.len * 2);
			auto&& n = mysql_real_escape_string(in.ctx, (char*)s.data() + len, in.buf, (unsigned long)in.len);
			s.resize(len + n);
		}
	};

	namespace MySql {

		struct Info {
			unsigned int numFields = 0;
			my_ulonglong numRows = 0;
			MYSQL_FIELD* fields = nullptr;
			std::string& lastError;

			Info(std::string& lastError)
				: lastError(lastError) {
			}

			// todo: get field info helpers?
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

			// 填充( 不做 数据类型校验. char* 能成功转为目标类型就算成功 )
			template<typename T>
			void Read(unsigned int const& colIdx, T& outVal) const {
				if (colIdx >= info.numFields) {
					info.lastError.clear();
					Append(info.lastError, "colIdx: ", colIdx, " out of range. numFields = ", info.numFields);
					throw - 1;
				}
				if constexpr (std::is_same_v<T, int>) {
					outVal = IsDBNull(colIdx) ? 0 : ReadInt32(colIdx);
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					outVal = IsDBNull(colIdx) ? 0 : ReadInt64(colIdx);
				}
				else if constexpr (std::is_same_v<T, double>) {
					outVal = IsDBNull(colIdx) ? 0 : ReadDouble(colIdx);
				}
				// more

				else if constexpr (std::is_same_v<T, std::optional<int>>) {
					if (IsDBNull(colIdx)) {
						outVal.reset();
					}
					else {
						outVal = ReadInt32(colIdx);
					}
				}
				else if constexpr (std::is_same_v<T, std::optional<int64_t>>) {
					if (IsDBNull(colIdx)) {
						outVal.reset();
					}
					else {
						outVal = ReadInt64(colIdx);
					}
				}
				else if constexpr (std::is_same_v<T, std::optional<double>>) {
					if (IsDBNull(colIdx)) {
						outVal.reset();
					}
					else {
						outVal = ReadDouble(colIdx);
					}
				}
				// more

				else if constexpr (std::is_same_v<T, std::string>) {
					outVal.assign(data[colIdx], lengths[colIdx]);
				}
				else if constexpr (std::is_same_v<T, std::optional<std::string>>) {
					if (IsDBNull(colIdx)) {
						outVal.reset();
					}
					else {
						outVal = std::string(data[colIdx], lengths[colIdx]);
					}
				}
				else {
					info.lastError.clear();
					Append(info.lastError, "unhandled value type: ", typeid(T).name());
					throw - 1;
				}
			}

			template<typename T>
			void Read(char const* const& colName, T& outVal) {
				for (unsigned int  i = 0; i < info.numFields; ++i) {
					if (strcmp(info.fields[i].name, colName) == 0) {
						Read(i, outVal);
						return;
					}
				}
				info.lastError.clear();
				Append(info.lastError, "could not find field name = ", colName);
				throw - 1;
			}
			template<typename T>
			void Read(std::string const& colName, T& outVal) {
				Read(colName.c_str(), outVal);
			}

			// 一次填充多个
			template<typename...Args>
			inline void Reads(Args& ...args) {
				int colIdx = 0;
				ReadsCore(colIdx, args...);
			}

		protected:
			template<typename Arg, typename...Args>
			inline void ReadsCore(int& colIdx, Arg& arg, Args& ...args) {
				Read(colIdx, arg);
				ReadsCore(++colIdx, args...);
			}

			inline void ReadsCore(int& colIdx) {
				(void)colIdx;
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

			Escaped MakeEscaped() {
				return Escaped(ctx);
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

				Info info(lastError);

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
















		// todo: 重构下面代码. 将 MYSQL_BIND_value 里面的函数移到 Binds. MYSQL_BIND_value 不在存储 MYSQL_BIND*

		// 只适合 new[], delete[]. 不可移动, 复制. 避免指针指向无效内存地址.
		struct MYSQL_BIND_value {
			union {
				int64_t i64 = 0;
				int i32;
				double d;
				unsigned long len;	// buf's data length
			};
			my_bool isNull = false;
			MYSQL_BIND* bind = nullptr;			// 需要 new 后填充
			std::string* lastError = nullptr;	// 需要 new 后填充

			MYSQL_BIND_value() = default;
			MYSQL_BIND_value(MYSQL_BIND_value const&) = delete;
			MYSQL_BIND_value& operator=(MYSQL_BIND_value const&) = delete;

			~MYSQL_BIND_value() {
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

					len = (unsigned long)v.size();
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

						len = xx::ArrayInfo_v<T> -1;			// 去掉末尾 \0
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

			//template<typename T>
			//inline void GetValue(T& v) {
			//	// todo
			//}
		};

		struct Binds {
			MYSQL_BIND* bind = nullptr;
			MYSQL_BIND_value* data = nullptr;
			size_t len = 0;
			std::string* lastError = nullptr;

			Binds(std::string* const& lastError)
				: lastError(lastError)
			{
			}
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

					data = new MYSQL_BIND_value[len]();

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
					lastError->clear();;
					Append(*lastError, "parmIdx out of range. len = ", len, ", parmIdx = ", parmIdx);
					throw - 1;
				}
				data[parmIdx].SetParameter<T>(v);
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

			//template<typename T>
			//inline void Read(int const& colIdx, T& v) {
			//	if (!data || colIdx >= len) {
			//		lastError->clear();;
			//		Append(*lastError, "colIdx out of range. len = ", len, ", colIdx = ", colIdx);
			//		throw - 1;
			//	}
			//	data[parmIdx].GetValue<T>(v);
			//}

		};

		struct QueryReader;
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

				if (mysql_stmt_bind_param(stmt, bind)) {
					conn.lastError = mysql_stmt_error(stmt);
					throw - 1;
				}
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

			Query& Execute() {
				if (mysql_stmt_execute(stmt)) {
					conn.lastError = mysql_stmt_error(stmt);
					throw - 1;
				}
			}

			template<typename ...Fields>
			bool Fetch(std::function<bool(QueryReader&)>&& h);

			Query& operator()() {
				// todo
				return *this;
			}
		};

		struct QueryReader : Binds {
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
			template<typename T>
			void Read(int const& colIdx, T& outVal) {
				if constexpr (std::is_same_v<T, int>) {
					if (bind[colIdx].buffer_type != MYSQL_TYPE_LONG) goto LabError;
					outVal = data[colIdx].i32;
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					if (bind[colIdx].buffer_type != MYSQL_TYPE_LONGLONG) goto LabError;
					outVal = data[colIdx].i64;
				}
				else if constexpr (std::is_same_v<T, double>) {
					if (bind[colIdx].buffer_type != MYSQL_TYPE_DOUBLE) goto LabError;
					outVal = data[colIdx].d;
				}
				// more

				else if constexpr (std::is_same_v<T, std::optional<int>>) {
					if (bind[colIdx].buffer_type != MYSQL_TYPE_LONG) goto LabError;
					if (data[colIdx].isNull) {
						outVal.reset();
					}
					else {
						outVal = data[colIdx].i32;
					}
				}
				else if constexpr (std::is_same_v<T, std::optional<int64_t>>) {
					if (bind[colIdx].buffer_type != MYSQL_TYPE_LONGLONG) goto LabError;
					if (data[colIdx].isNull) {
						outVal.reset();
					}
					else {
						outVal = data[colIdx].i64;
					}
				}
				else if constexpr (std::is_same_v<T, std::optional<double>>) {
					if (bind[colIdx].buffer_type != MYSQL_TYPE_DOUBLE) goto LabError;
					if (data[colIdx].isNull) {
						outVal.reset();
					}
					else {
						outVal = data[colIdx].d;
					}
				}
				// more

				else if constexpr (std::is_same_v<T, std::string>) {
					if (bind[colIdx].buffer_type != MYSQL_TYPE_STRING) goto LabError;
					outVal.assign((char*)bind[colIdx].buffer, data[colIdx].len);
				}
				else if constexpr (std::is_same_v<T, std::optional<std::string>>) {
					if (data[colIdx].isNull) {
						outVal.reset();
					}
					else {
						outVal = std::string((char*)bind[colIdx].buffer, data[colIdx].len);
					}
				}
				else goto LabError;
				return;

			LabError:
				lastError->clear();
				Append(*lastError, "Read failed. Init buffer_type is ", bind[colIdx].buffer_type);
				throw - 1;
			}

			//template<typename T>
			//void Read(char const* const& colName, T& outVal);
			//template<typename T>
			//void Read(std::string const& colName, T& outVal);

			// 一次填充多个
			template<typename...Args>
			inline void Reads(Args& ...args) {
				int colIdx = 0;
				ReadsCore(colIdx, args...);
			}

		protected:
			template<typename Arg, typename...Args>
			inline void ReadsCore(int& colIdx, Arg& arg, Args& ...args) {
				Read(colIdx, arg);
				ReadsCore(++colIdx, args...);
			}

			inline void ReadsCore(int& colIdx) {
				(void)colIdx;
			}
		};

		// 访问一个结果集
		template<typename ...Fields>
		inline bool Query::Fetch(std::function<bool(QueryReader&)>&& h) {
			static_assert(sizeof...(Fields), "must be specify fields types.");

			QueryReader reader(lastError);

			reader.numCols = sizeof...(Fields);
			// todo: fill fields? get field count? verify?
			reader.Resize(sizeof...(Fields));

			int idx = 0;
			std::initializer_list<int> n{ (reader.data[idx++].Init<Fields>(), 0)... };
			(void)n;

			if (mysql_stmt_bind_result(stmt, reader.bind)) {
				(*lastError) = mysql_stmt_error(stmt);
				throw - 1;
			}

			int status = 0;
			while (true) {
				status = mysql_stmt_fetch(stmt);
				if (status == 1 || status == MYSQL_NO_DATA) break;
				if (!h(reader)) break;
			}

			// todo: check if has next result
			return false;
		}
	}
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	// 结论：stmt query, 只比 real_query 快一点. 当前示例为 6.25% 左右. 
	// 如果 stmt query 不复用, 每次都新建, 会比 real_query 慢 1 倍

	xx::MySql::Connection conn;
	try {
		conn.Open("192.168.1.230", 3306, "root", "Abc123", "test");

		for (int j = 0; j < 5; ++j) {
			{
				auto&& v1 = conn.MakeEscaped();
				std::string sql;
				v1 = "'\0a\0s\"\"d\"\"f\0'";
				xx::Append(sql, "select '", v1, "'");

				auto&& t = xx::NowSystemEpochMS();
				std::string c1;
				for (int i = 0; i < 1000; ++i) {
					conn.Execute(sql);
					conn.Fetch(nullptr, [&](xx::MySql::Reader& r) {
						r.Reads(c1);
						return true;
						});
				}
				xx::CoutN("real_query ms = ", xx::NowSystemEpochMS() - t);
				xx::CoutN(sql);
				xx::CoutN(c1);
			}

			//{
			//	auto&& t = xx::NowSystemEpochMS();
			//	int s;
			//	xx::MySql::Query q(conn);
			//	q.SetQuery<>("select 123");
			//	for (int i = 0; i < 10000; ++i) {
			//		//q.SetQuery<>("select 123");
			//		q.Execute();
			//		q.Fetch<int>([&](xx::MySql::QueryReader& r) {
			//			r.Reads(s);
			//			return true;
			//			});
			//	}
			//	xx::CoutN(s);
			//	xx::CoutN("stmt ms = ", xx::NowSystemEpochMS() - t);
			//}
		}
	}
	catch (int const& e) {
		std::cout << conn.lastError << std::endl;
		return e;
	}

	return 0;
}



//#include <xx_sqlite.h>
//int main(int argc, char* argv[]) {
//
//	xx::SQLite::Connection conn(":memory:");
//	try {
//		xx::SQLite::Query q(conn);
//		q.SetQuery("select ?, ?");
//		std::string s;
//		s.resize(2000000);
//		auto&& t = std::chrono::steady_clock::now();
//		for (int i = 0; i < 1000; ++i) {
//			q.SetParameters(123, s);
//			q.Execute([](xx::SQLite::Reader& r) {
//				int c1;
//				std::string c2;
//				r.Reads(c1, c2);
//				if (c1 != 123) {
//					throw - 1;
//				}
//				});
//		}
//		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t).count() << std::endl;
//	}
//	catch (int const& e) {
//		std::cout << e << std::endl;
//	}
//}



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