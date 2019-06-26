#pragma once
#include "sqlite3.h"
#include <assert.h>
#include <memory>
#include <string>
#include <functional>

namespace xx {
	namespace SQLite {

		// SQLITE 基本上只有 4 种数据类型： int, int64_t, double, string. 故查询模板只能传入这几种

		struct Connection;
		struct Query;
		struct Reader;

		// 保持与 SQLite 的宏一致
		enum class DataTypes : uint8_t {
			Integer = 1,
			Float = 2,
			Text = 3,
			Blob = 4,
			Null = 5
		};

		// 数据写盘模式
		enum class SynchronousTypes : uint8_t {
			Full,			// 等完全写入磁盘
			Normal,			// 阶段性等写磁盘
			Off,			// 完全不等
		};
		static const char* strSynchronousTypes[] = {
			"FULL", "NORMAL", "OFF"
		};

		// 事务数据记录模式
		enum class JournalModes : uint8_t {
			Delete,			// 终止事务时删掉 journal 文件
			Truncate,		// 不删, 字节清 0 ( 可能比删快 )
			Persist,		// 在文件头打标记( 可能比字节清 0 快 )
			Memory,			// 内存模式( 可能丢数据 )
			WAL,			// write-ahead 模式( 似乎比上面都快, 不会丢数据 )
			Off,			// 无事务支持( 最快 )
		};
		static const char* strJournalModes[] = {
			"DELETE", "TRUNCATE", "PERSIST", "MEMORY", "WAL", "OFF"
		};

		// 临时表处理模式
		enum class TempStoreTypes : uint8_t {
			Default,		// 默认( 视 C TEMP_STORE 宏而定 )
			File,			// 在文件中建临时表
			Memory,			// 在内存中建临时表
		};
		static const char* strTempStoreTypes[] = {
			"DEFAULT", "FILE", "MEMORY"
		};

		// 排它锁持有方式，仅当关闭数据库连接，或者将锁模式改回为NORMAL时，再次访问数据库文件（读或写）才会放掉
		enum class LockingModes : uint8_t {
			Normal,			// 数据库连接在每一个读或写事务终点的时候放掉文件锁
			Exclusive,		// 连接永远不会释放文件锁. 第一次执行读操作时，会获取并持有共享锁，第一次写，会获取并持有排它锁
		};
		static const char* strLockingModes[] = {
			"NORMAL", "EXCLUSIVE"
		};

		// 查询主体
		struct Query {
		protected:
			Connection& owner;
			sqlite3_stmt* stmt = nullptr;
		public:
			typedef std::function<void(Reader& sr)> ReadFunc;

			Query(Connection& owner);
			Query(Connection& owner, char const* const& sql, int const& sqlLen = 0);
			template<size_t sqlLen>
			Query(Connection& owner, char const(&sql)[sqlLen]);
			~Query();
			Query() = delete;
			Query(Query const&) = delete;
			Query& operator=(Query const&) = delete;

			// 主用于判断 stmt 是否为空( 是否传入 sql )
			operator bool() const noexcept;

			// 配合 Query(Connection& owner) 后期传入 sql 以初始化 stmt
			void SetQuery(char const* const& sql, int const& sqlLen = 0);

			// 释放 stmt
			void Clear() noexcept;

			// 下面这些函数都是靠 try 来检测错误

			void SetParameter(int parmIdx, int const& v);
			void SetParameter(int parmIdx, int64_t const& v);
			void SetParameter(int parmIdx, double const& v);
			void SetParameter(int parmIdx, char const* const& str, int strLen = 0, bool const& makeCopy = false);
			void SetParameter(int parmIdx, char const* const& buf, size_t const& len, bool const& makeCopy = false);
			void SetParameter(int parmIdx, std::string* const& str, bool const& makeCopy = false);
			void SetParameter(int parmIdx, std::string const& str, bool const& makeCopy = false);
			void SetParameter(int parmIdx, std::shared_ptr<std::string> const& str, bool const& makeCopy = false);
			//void SetParameter(int parmIdx, BBuffer* const& buf, bool const& makeCopy = false);
			//void SetParameter(int parmIdx, BBuffer const& buf, bool const& makeCopy = false);
			//void SetParameter(int parmIdx, BBuffer_p const& buf, bool const& makeCopy = false);
			template<typename EnumType>
			void SetParameter(int parmIdx, EnumType const& v);

			template<typename...Parameters>
			Query& SetParameters(Parameters const& ...ps);
		protected:
			template<typename Parameter, typename...Parameters>
			void SetParametersCore(int& parmIdx, Parameter const& p, Parameters const& ...ps);
			void SetParametersCore(int& parmIdx);

		public:
			Query& Execute(ReadFunc&& rf = nullptr);
			Query& operator()();

			// todo: std::optional 检测与支持

			template<typename T>
			Query& operator()(T& outVal);
		};


		// 数据库主体
		struct Connection {
			friend Query;

			// 保存最后一次的错误码
			int lastErrorCode = 0;

			// 保存 / 指向 最后一次的错误信息
			const char* lastErrorMessage = nullptr;
		protected:

			// sqlite3 上下文
			sqlite3* ctx = nullptr;

			// 临时拿来拼 sql 串
			std::string sqlBuilder;

			// 预创建的一堆常用查询语句
			Query qBeginTransaction;
			Query qCommit;
			Query qRollback;
			Query qEndTransaction;
			Query qTableExists;
			Query qGetTableCount;
			Query qAttach;
			Query qDetach;

			// 为throw错误提供便利
			void ThrowError(int const& errCode, char const* const& errMsg = nullptr);
		public:
			// fn 可以是 :memory: 以创建内存数据库
			Connection(char const* const& fn, bool const& readOnly = false) noexcept;
			~Connection();
			Connection() = delete;
			Connection(Connection const&) = delete;
			Connection& operator=(Connection const&) = delete;

			// 主用于判断构造函数是否执行成功( db 成功打开 )
			operator bool() const noexcept;

			// 获取受影响行数
			int GetAffectedRows();

			// 下列函数均靠 try 检测是否执行出错

			// 各种 set pragma( 通常推荐设置 JournalModes::WAL 似乎能提升一些 insert 的性能 )

			// 事务数据记录模式( 设成 WAL 能提升一些性能 )
			void SetPragmaJournalMode(JournalModes jm);

			// 启用外键约束( 默认为未启用 )
			void SetPragmaForeignKeys(bool enable);

			// 数据写盘模式( Off 最快 )
			void SetPragmaSynchronousType(SynchronousTypes st);

			// 临时表处理模式
			void SetPragmaTempStoreType(TempStoreTypes tst);

			// 排它锁持有方式( 文件被 单个应用独占情况下 EXCLUSIVE 最快 )
			void SetPragmaLockingMode(LockingModes lm);

			// 内存数据库页数( 4096 似乎最快 )
			void SetPragmaCacheSize(int cacheSize);


			// 附加另外一个库
			void Attach(char const* const& alias, char const* const& fn);	// ATTACH DATABASE 'fn' AS 'alias'

			// 反附加另外一个库
			void Detach(char const* const& alias);							// DETACH DATABASE 'alias'

			// 启动事务
			void BeginTransaction();										// BEGIN TRANSACTION

			// 提交事务
			void Commit();													// COMMIT TRANSACTION

			// 回滚
			void Rollback();												// ROLLBACK TRANSACTION

			// 结束事务( 同 Commit )
			void EndTransaction();											// END TRANSACTION

			// 返回 1 表示只包含 'sqlite_sequence' 这样一个预创建表. 
			// android 下也有可能返回 2, 有张 android 字样的预创建表存在
			int GetTableCount();											// SELECT count(*) FROM sqlite_master

			// 判断表是否存在
			bool TableExists(char const* const& tn);						// SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?

			// 会清空表数据, 并且重置自增计数. 如果存在约束, 或 sqlite_sequence 不存在, 有可能清空失败.
			void TruncateTable(char const* const& tn);						// DELETE FROM ?; DELETE FROM sqlite_sequence WHERE name = ?;

			// 直接执行一个 SQL 语句
			void Call(char const* const& sql, int(*selectRowCB)(void* userData, int numCols, char** colValues, char** colNames) = nullptr, void* const& userData = nullptr);

			// 直接执行一个 SQL 语句. 如果 T 不为 void, 将返回第一行第一列的值. 
			// todo: 如果 T 是 tuple 则多值填充
			template<typename T = void>
			T Execute(char const* const& sql, int const& sqlLen);

			template<typename T = void, size_t sqlLen>
			T Execute(char const(&sql)[sqlLen]);
		};






		struct Reader {
		protected:
			sqlite3_stmt* stmt = nullptr;
		public:
			int numCols = 0;

			Reader(sqlite3_stmt* const& stmt);
			Reader() = delete;
			Reader(Reader const&) = delete;
			Reader& operator=(Reader const&) = delete;

			DataTypes GetColumnDataType(int const& colIdx);
			char const* GetColumnName(int const& colIdx);

			bool IsDBNull(int const& colIdx);

			int ReadInt32(int const& colIdx);
			int64_t ReadInt64(int const& colIdx);
			double ReadDouble(int const& colIdx);
			char const* ReadString(int const& colIdx);
			std::pair<char const*, int> ReadText(int const& colIdx);
			std::pair<char const*, int> ReadBlob(int const& colIdx);

			void Read(int const& colIdx, int& outVal);
			void Read(int const& colIdx, int64_t& outVal);
			void Read(int const& colIdx, double& outVal);
			void Read(int const& colIdx, std::string& outVal);
			template<typename T>
			void Read(int const& colIdx, T& outVal);
		};





		/******************************************************************************************************/
		// impls
		/******************************************************************************************************/

		/***************************************************************/
		// Connection

		inline Connection::Connection(char const* const& fn, bool const& readOnly) noexcept
			: qBeginTransaction(*this)
			, qCommit(*this)
			, qRollback(*this)
			, qEndTransaction(*this)
			, qTableExists(*this)
			, qGetTableCount(*this)
			, qAttach(*this)
			, qDetach(*this)
		{
			int r = sqlite3_open_v2(fn, &ctx, readOnly ? SQLITE_OPEN_READONLY : (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), nullptr);
			if (r != SQLITE_OK) {
				ctx = nullptr;
				return;
			}

			qAttach.SetQuery("ATTACH DATABASE ? AS ?");
			qDetach.SetQuery("DETACH DATABASE ?");
			qBeginTransaction.SetQuery("BEGIN TRANSACTION");
			qCommit.SetQuery("COMMIT TRANSACTION");
			qRollback.SetQuery("ROLLBACK TRANSACTION");
			qEndTransaction.SetQuery("END TRANSACTION");
			qTableExists.SetQuery("SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?");
			qGetTableCount.SetQuery("SELECT count(*) FROM sqlite_master WHERE type = 'table'");
		}

		inline Connection::~Connection() {
			if (ctx) {
				sqlite3_close(ctx);
				ctx = nullptr;
			}
		}

		inline Connection::operator bool() const noexcept {
			return ctx != nullptr;
		}

		inline void Connection::ThrowError(int const& errCode, char const* const& errMsg) {
			lastErrorCode = errCode;
			lastErrorMessage = errMsg ? errMsg : sqlite3_errmsg(ctx);
			throw errCode;
		}

		inline int Connection::GetAffectedRows() {
			return sqlite3_changes(ctx);
		}

		inline void Connection::SetPragmaSynchronousType(SynchronousTypes st) {
			if ((int)st < 0 || (int)st >(int)SynchronousTypes::Off) ThrowError(-1, "bad SynchronousTypes");
			sqlBuilder.clear();
			sqlBuilder.append("PRAGMA synchronous = ");
			sqlBuilder.append(strSynchronousTypes[(int)st]);
			Call(sqlBuilder.c_str());
		}

		inline void Connection::SetPragmaJournalMode(JournalModes jm) {
			if ((int)jm < 0 || (int)jm >(int)JournalModes::Off) ThrowError(-1, "bad JournalModes");
			sqlBuilder.clear();
			sqlBuilder.append("PRAGMA journal_mode = ");
			sqlBuilder.append(strJournalModes[(int)jm]);
			Call(sqlBuilder.c_str());
		}

		inline void Connection::SetPragmaTempStoreType(TempStoreTypes tst) {
			if ((int)tst < 0 || (int)tst >(int)TempStoreTypes::Memory) ThrowError(-1, "bad TempStoreTypes");
			sqlBuilder.clear();
			sqlBuilder.append("PRAGMA temp_store = ");
			sqlBuilder.append(strTempStoreTypes[(int)tst]);
			Call(sqlBuilder.c_str());
		}

		inline void Connection::SetPragmaLockingMode(LockingModes lm) {
			if ((int)lm < 0 || (int)lm >(int)LockingModes::Exclusive) ThrowError(-1, "bad LockingModes");
			sqlBuilder.clear();
			sqlBuilder.append("PRAGMA locking_mode = ");
			sqlBuilder.append(strLockingModes[(int)lm]);
			Call(sqlBuilder.c_str());
		}

		inline void Connection::SetPragmaCacheSize(int cacheSize) {
			if (cacheSize < 1) ThrowError(-1, "bad cacheSize( default is 2000 )");
			sqlBuilder.clear();
			sqlBuilder.append("PRAGMA cache_size = ");
			sqlBuilder.append(std::to_string(cacheSize));
			Call(sqlBuilder.c_str());
		}

		inline void Connection::SetPragmaForeignKeys(bool enable) {
			sqlBuilder.clear();
			sqlBuilder.append("PRAGMA foreign_keys = ");
			sqlBuilder.append(enable ? "true" : "false");
			Call(sqlBuilder.c_str());
		}

		inline void Connection::Call(char const* const& sql, int(*selectRowCB)(void* userData, int numCols, char** colValues, char** colNames), void* const& userData) {
			lastErrorCode = sqlite3_exec(ctx, sql, selectRowCB, userData, (char**)& lastErrorMessage);
			if (lastErrorCode != SQLITE_OK) throw lastErrorCode;
		}

		template<typename T>
		inline T Connection::Execute(char const* const& sql, int const& sqlLen) {
			Query q(*this, sql, sqlLen);
			if constexpr( std::is_void_v<T> ) {
				q();
			}
			else {
				T rtv{};
				q(rtv);
				return rtv;
			}
		}

		template<typename T, size_t sqlLen>
		T Connection::Execute(char const(&sql)[sqlLen]) {
			return Execute<T>(sql, (int)sqlLen);
		}

		inline void Connection::Attach(char const* const& alias, char const* const& fn) {
			qAttach.SetParameters(fn, alias)();
		}

		inline void Connection::Detach(char const* const& alias) {
			qDetach.SetParameters(alias)();
		}

		inline void Connection::BeginTransaction() {
			qBeginTransaction();
		}

		inline void Connection::Commit() {
			qCommit();
		}

		inline void Connection::Rollback() {
			qRollback();
		}

		inline void Connection::EndTransaction() {
			qEndTransaction();
		}

		inline bool Connection::TableExists(char const* const& tn) {
			int exists = 0;
			qTableExists.SetParameters(tn)(exists);
			return exists != 0;
		}

		inline int Connection::GetTableCount() {
			int count = 0;
			qGetTableCount(count);
			return count;
		}

		inline void Connection::TruncateTable(char const* const& tn) {
			// todo: 对 tn 转义
			sqlBuilder.clear();
			sqlBuilder += std::string("BEGIN; DELETE FROM [") + tn + "]; DELETE FROM [sqlite_sequence] WHERE [name] = '" + tn + "'; COMMIT;";
			Call(sqlBuilder.c_str());
		}




		/***************************************************************/
		// Query

		inline Query::Query(Connection& owner)
			: owner(owner) {
		}

		inline Query::Query(Connection& owner, char const* const& sql, int const& sqlLen)
			: owner(owner) {
			SetQuery(sql, sqlLen);
		}

		template<size_t sqlLen>
		inline Query::Query(Connection& owner, char const(&sql)[sqlLen])
			: owner(owner) {
			SetQuery(sql, sqlLen - 1);
		}

		inline Query::~Query() {
			Clear();
		}

		inline Query::operator bool() const noexcept {
			return stmt != nullptr;
		}

		inline void Query::SetQuery(char const* const& sql, int const& sqlLen) {
			Clear();
			auto r = sqlite3_prepare_v2(owner.ctx, sql, sqlLen ? sqlLen : (int)strlen(sql), &stmt, nullptr);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::Clear() noexcept {
			if (stmt) {
				sqlite3_finalize(stmt);
				stmt = nullptr;
			}
		}

		inline void Query::SetParameter(int parmIdx, int const& v) {
			assert(stmt);
			auto r = sqlite3_bind_int(stmt, parmIdx, v);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::SetParameter(int parmIdx, int64_t const& v) {
			assert(stmt);
			auto r = sqlite3_bind_int64(stmt, parmIdx, v);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::SetParameter(int parmIdx, double const& v) {
			assert(stmt);
			auto r = sqlite3_bind_double(stmt, parmIdx, v);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::SetParameter(int parmIdx, char const* const& str, int strLen, bool const& makeCopy) {
			assert(stmt);
			int r = SQLITE_OK;
			if (!str) r = sqlite3_bind_null(stmt, parmIdx);
			if (r != SQLITE_OK) owner.ThrowError(r);
			r = sqlite3_bind_text(stmt, parmIdx, str, strLen ? strLen : (int)strlen(str), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::SetParameter(int parmIdx, char const* const& buf, size_t const& len, bool const& makeCopy) {
			assert(stmt);
			int r = SQLITE_OK;
			if (!buf) r = sqlite3_bind_null(stmt, parmIdx);
			if (r != SQLITE_OK) owner.ThrowError(r);
			r = sqlite3_bind_blob(stmt, parmIdx, buf, (int)len, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::SetParameter(int parmIdx, std::string* const& str, bool const& makeCopy) {
			assert(stmt);
			int r = SQLITE_OK;
			if (!str) r = sqlite3_bind_null(stmt, parmIdx);
			if (r != SQLITE_OK) owner.ThrowError(r);
			r = sqlite3_bind_text(stmt, parmIdx, (char*)str->data(), (int)str->size(), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::SetParameter(int parmIdx, std::string const& str, bool const& makeCopy) {
			assert(stmt);
			auto r = sqlite3_bind_text(stmt, parmIdx, (char*)str.data(), (int)str.size(), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		inline void Query::SetParameter(int parmIdx, std::shared_ptr<std::string> const& str, bool const& makeCopy) {
			assert(stmt);
			int r = SQLITE_OK;
			if (!str) r = sqlite3_bind_null(stmt, parmIdx);
			if (r != SQLITE_OK) owner.ThrowError(r);
			r = sqlite3_bind_text(stmt, parmIdx, (char*)str->data(), (int)str->size(), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
			if (r != SQLITE_OK) owner.ThrowError(r);
		}

		//inline void Query::SetParameter(int parmIdx, BBuffer* const& buf, bool const& makeCopy) {
		//assert(stmt);
		//	int r = SQLITE_OK;
		//	if (!buf) r = sqlite3_bind_null(stmt, parmIdx);
		//	if (r != SQLITE_OK) owner.ThrowError(r);
		//	r = sqlite3_bind_blob(stmt, parmIdx, buf->buf, (int)buf->dataLen, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		//	if (r != SQLITE_OK) owner.ThrowError(r);
		//}

		//inline void Query::SetParameter(int parmIdx, BBuffer const& buf, bool const& makeCopy) {
		//assert(stmt);
		//	auto r = sqlite3_bind_blob(stmt, parmIdx, buf.buf, (int)buf.dataLen, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		//	if (r != SQLITE_OK) owner.ThrowError(r);
		//}

		//inline void Query::SetParameter(int parmIdx, BBuffer_p const& buf, bool const& makeCopy) {
		//assert(stmt);
		//	int r = SQLITE_OK;
		//	if (!buf) r = sqlite3_bind_null(stmt, parmIdx);
		//	if (r != SQLITE_OK) owner.ThrowError(r);
		//	r = sqlite3_bind_blob(stmt, parmIdx, buf->buf, (int)buf->dataLen, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		//	if (r != SQLITE_OK) owner.ThrowError(r);
		//}

		template<typename EnumType>
		void Query::SetParameter(int parmIdx, EnumType const& v) {
			assert(stmt);
			static_assert(std::is_enum<EnumType>::value, "parameter only support sqlite base types and enum types.");
			if constexpr (sizeof(EnumType) <= 4) SetParameter(parmIdx, (int)(typename std::underlying_type<EnumType>::type)v);
			else SetParameter(parmIdx, (int64_t)(typename std::underlying_type<EnumType>::type)v);
		}

		template<typename...Parameters>
		Query& Query::SetParameters(Parameters const& ...ps) {
			assert(stmt);
			int parmIdx = 1;
			SetParametersCore(parmIdx, ps...);
			return *this;
		}

		template<typename Parameter, typename...Parameters>
		void Query::SetParametersCore(int& parmIdx, Parameter const& p, Parameters const& ...ps) {
			assert(stmt);
			SetParameter(parmIdx, p);
			SetParametersCore(++parmIdx, ps...);
		}

		inline void Query::SetParametersCore(int& parmIdx) {}

		inline Query& Query::Execute(ReadFunc&& rf) {
			assert(stmt);
			Reader dr(stmt);

			int r = sqlite3_step(stmt);
			if (r == SQLITE_DONE || (r == SQLITE_ROW && !rf)) goto LabEnd;
			else if (r != SQLITE_ROW) goto LabErr;

			dr.numCols = sqlite3_column_count(stmt);
			do
			{
				rf(dr);
				r = sqlite3_step(stmt);
			} while (r == SQLITE_ROW);
			assert(r == SQLITE_DONE);

		LabEnd:
			r = sqlite3_reset(stmt);
			if (r == SQLITE_OK) return *this;

		LabErr:
			auto ec = r;
			auto em = sqlite3_errmsg(owner.ctx);
			sqlite3_reset(stmt);
			owner.ThrowError(ec, em);
			return *this;
		}

		inline Query& Query::operator()() {
			return Execute(nullptr);
		}

		template<typename T>
		Query& Query::operator()(T& outVal) {
			return Execute([&](Reader& r) {
				if (r.numCols && !r.IsDBNull(0)) {
					r.Read(0, outVal);
				}
			});
		}


		/***************************************************************/
		// Reader

		inline Reader::Reader(sqlite3_stmt* const& stmt) : stmt(stmt) {}

		inline DataTypes Reader::GetColumnDataType(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols);
			return (DataTypes)sqlite3_column_type(stmt, colIdx);
		}

		inline char const* Reader::GetColumnName(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols);
			return sqlite3_column_name(stmt, colIdx);
		}

		inline bool Reader::IsDBNull(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols);
			return GetColumnDataType(colIdx) == DataTypes::Null;
		}

		inline char const* Reader::ReadString(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols);
			if (IsDBNull(colIdx)) return nullptr;
			return (char const*)sqlite3_column_text(stmt, colIdx);
		}

		inline int Reader::ReadInt32(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols && !IsDBNull(colIdx));
			return sqlite3_column_int(stmt, colIdx);
		}

		inline int64_t Reader::ReadInt64(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols && !IsDBNull(colIdx));
			return sqlite3_column_int64(stmt, colIdx);
		}

		inline double Reader::ReadDouble(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols && !IsDBNull(colIdx));
			return sqlite3_column_double(stmt, colIdx);
		}

		inline std::pair<char const*, int> Reader::ReadText(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols);
			auto ptr = (char const*)sqlite3_column_text(stmt, colIdx);
			auto len = sqlite3_column_bytes(stmt, colIdx);
			return std::make_pair(ptr, len);
		}

		inline std::pair<char const*, int> Reader::ReadBlob(int const& colIdx) {
			assert(colIdx >= 0 && colIdx < numCols);
			auto ptr = (char const*)sqlite3_column_blob(stmt, colIdx);
			auto len = sqlite3_column_bytes(stmt, colIdx);
			return std::make_pair(ptr, len);
		}

		inline void Reader::Read(int const& colIdx, int& outVal) {
			outVal = ReadInt32(colIdx);
		}
		inline void Reader::Read(int const& colIdx, int64_t& outVal) {
			outVal = ReadInt64(colIdx);

		}
		inline void Reader::Read(int const& colIdx, double& outVal) {
			outVal = ReadDouble(colIdx);
		}
		inline void Reader::Read(int const& colIdx, std::string& outVal) {
			auto&& r = ReadText(colIdx);
			outVal.assign(r.first, r.second);
		}

		template<typename T>
		inline void Reader::Read(int const& colIdx, T& outVal) {
			assert(false);
			// unsupport
		}
	}
}
