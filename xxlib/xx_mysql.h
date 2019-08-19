#pragma once
#ifdef _WIN32
#include <mysql.h>
#else
#include <mariadb/mysql.h>
#endif
#include "xx_object.h"

namespace xx
{
	namespace MySql {

		// 方便 Append 时 使用 mysql_escape_string 转义. xx::Append( sql, "....", xx::MySql::Escaped( "..\\''.." ), "...." )
		struct Escaped {
			std::string s;
			template<typename T>
			Escaped(T&& s) {
				this->s = std::forward<T>(s);
			}
			Escaped(Escaped const&) = delete;
			Escaped& operator=(Escaped const&) = delete;
		};
	}

	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_base_of_v<xx::MySql::Escaped, T>>> {
		static inline void WriteTo(std::string& s, T const& in) noexcept {
			auto&& len = s.size();
			s.resize(len + in.s.size() * 2);
			auto&& n = mysql_escape_string((char*)s.data() + len, (char*)in.s.data(), (unsigned long)in.s.size());
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

			// todo: ato? 替换为更快的实现?

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

			// todo: BBuffer? more types support?

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
				for (unsigned int i = 0; i < info.numFields; ++i) {
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
			void Reads(Args& ...args) {
				int colIdx = 0;
				ReadsCore(colIdx, args...);
			}

		protected:
			template<typename Arg, typename...Args>
			void ReadsCore(int& colIdx, Arg& arg, Args& ...args) {
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
			unsigned int lastErrorNumber = 0;			// try 到的异常代码仅用于表达出错的位置. 这个是具体的错误码

			inline operator bool() {
				return ctx;
			}

			Connection() = default;
			Connection(Connection const&) = delete;
			Connection(Connection&& o) {
				operator=(std::move(o));
			}
			Connection& operator=(Connection const&) = delete;
			Connection& operator=(Connection && o) {
				std::swap(ctx, o.ctx);
				std::swap(lastError, o.lastError);
				std::swap(lastErrorNumber, o.lastErrorNumber);
				return *this;
			}
			~Connection() {
				Close();
			}

			inline int TryOpen(char const* const& host, int const& port, char const* const& username, char const* const& password, char const* const& db) noexcept {
				Close();
				ctx = mysql_init(nullptr);
				if (!ctx) {
					lastError = "mysql_init failed.";
					lastErrorNumber = -1;
					return -1;
				}
				if (!mysql_real_connect(ctx, host, username, password, db, port, nullptr, CLIENT_MULTI_STATEMENTS)) {	// todo: 关 SSL 的参数, 限制连接超时时长
					lastError = mysql_error(ctx);
					lastErrorNumber = mysql_errno(ctx);
					Close();
					return -2;
				}
				my_bool reconnect = 1;
				return mysql_options(ctx, MYSQL_OPT_RECONNECT, &reconnect);
			}

			inline void Open(char const* const& host, int const& port, char const* const& username, char const* const& password, char const* const& db) {
				if (int r = TryOpen(host, port, username, password, db)) throw r;
			}

			inline void Close() {
				if (ctx) {
					mysql_close(ctx);
					ctx = nullptr;
				}
			}

			inline int Ping() {
				if (!ctx) return 0;
				return mysql_ping(ctx);
			}



			// 执行一段 SQL 脚本. 后续使用 Fetch 检索返回结果( 如果有的话 ). 返回 0 表示成功. 非 0 为错误码.
			inline int TryExecute(char const* const& sql, unsigned long const& len) noexcept {
				if (!ctx) {
					lastError = "connection is closed.";
					lastErrorNumber = 0;
					return -1;
				}
				if (mysql_real_query(ctx, sql, len)) {
					lastError = mysql_error(ctx);
					lastErrorNumber = mysql_errno(ctx);
					return -2;
				}
				return 0;
			}

			template<size_t len>
			int TryExecute(char const(&sql)[len]) {
				return TryExecute(sql, (unsigned long)(len - 1));
			}

			inline int TryExecute(std::string const& sql) {
				return TryExecute((char*)sql.data(), (unsigned long)sql.size());
			}


			// 执行一段 SQL 脚本. 后续使用 Fetch 检索返回结果( 如果有的话 ). 出错抛异常
			inline void Execute(char const* const& sql, unsigned long const& len) {
				if (int r = TryExecute(sql, len)) throw r;
			}

			template<size_t len>
			void Execute(char const(&sql)[len]) {
				Execute(sql, (unsigned long)(len - 1));
			}

			inline void Execute(std::string const& sql) {
				Execute((char*)sql.data(), (unsigned long)sql.size());
			}


			// 填充一个结果集, 并产生相应回调. 出错返回负数. 0 表示没有. 1 表示存在下一个结果集
			// infoHandler 返回 true 将继续对每行数据发起 rowHandler 调用. 返回 false, 将终止调用
			inline int TryFetch(std::function<bool(Info&)>&& infoHandler, std::function<bool(Reader&)>&& rowHandler) {
				if (!ctx) {
					lastError = "connection is closed.";
					lastErrorNumber = 0;
					return -1;
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
								lastErrorNumber = mysql_errno(ctx);
								return -2;
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
					lastErrorNumber = mysql_errno(ctx);
					return -3;
				}

				// n = 0: 有更多结果集.  -1: 没有    >0: 出错
				auto&& n = mysql_next_result(ctx);
				if (!n) return 1;
				else if (n == -1) return 0;
				else {
					lastError = mysql_error(ctx);
					lastErrorNumber = mysql_errno(ctx);
					return -4;
				}
				return 0;
			};

			// 填充一个结果集, 并产生相应回调. 返回 是否存在下一个结果集. 出错抛异常
			// infoHandler 返回 true 将继续对每行数据发起 rowHandler 调用. 返回 false, 将终止调用
			inline bool Fetch(std::function<bool(Info&)>&& infoHandler, std::function<bool(Reader&)>&& rowHandler) {
				int r = TryFetch(std::move(infoHandler), std::move(rowHandler));
				if (r < 0) throw r;
				return (bool)r;
			}
		};
	}
}
