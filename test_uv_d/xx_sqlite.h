#pragma once
#include "sqlite3.h"
#include <assert.h>
#include <memory>
#include <string>
#include <functional>

namespace xx {
	struct SQLite;
	struct SQLiteQuery;
	struct SQLiteReader;

	// 保持与 SQLite 的宏一致
	enum class SQLiteDataTypes : uint8_t {
		Integer = 1,
		Float = 2,
		Text = 3,
		Blob = 4,
		Null = 5
	};

	// 数据写盘模式
	enum class SQLiteSynchronousTypes : uint8_t {
		Full,			// 等完全写入磁盘
		Normal,			// 阶段性等写磁盘
		Off,			// 完全不等
		MAX
	};
	static const char* SQLiteSynchronousTypes_ss[] = {
		"FULL", "NORMAL", "OFF"
	};

	// 事务数据记录模式
	enum class SQLiteJournalModes : uint8_t {
		Delete,			// 终止事务时删掉 journal 文件
		Truncate,		// 不删, 字节清 0 ( 可能比删快 )
		Persist,		// 在文件头打标记( 可能比字节清 0 快 )
		Memory,			// 内存模式( 可能丢数据 )
		WAL,			// write-ahead 模式( 似乎比上面都快, 不会丢数据 )
		Off,			// 无事务支持( 最快 )
		MAX
	};
	static const char* SQLiteJournalModes_ss[] = {
		"DELETE", "TRUNCATE", "PERSIST", "MEMORY", "WAL", "OFF"
	};

	// 临时表处理模式
	enum class SQLiteTempStoreTypes : uint8_t {
		Default,		// 默认( 视 C TEMP_STORE 宏而定 )
		File,			// 在文件中建临时表
		Memory,			// 在内存中建临时表
		MAX
	};
	static const char* SQLiteTempStoreTypes_ss[] = {
		"DEFAULT", "FILE", "MEMORY"
	};

	// 排它锁持有方式，仅当关闭数据库连接，或者将锁模式改回为NORMAL时，再次访问数据库文件（读或写）才会放掉
	enum class SQLiteLockingModes : uint8_t {
		Normal,			// 数据库连接在每一个读或写事务终点的时候放掉文件锁
		Exclusive,		// 连接永远不会释放文件锁. 第一次执行读操作时，会获取并持有共享锁，第一次写，会获取并持有排它锁
		MAX
	};
	static const char* SQLiteLockingModes_ss[] = {
		"NORMAL", "EXCLUSIVE"
	};

	// 查询主体
	struct SQLiteQuery {
	protected:
		SQLite& owner;
		sqlite3_stmt* stmt = nullptr;
	public:
		typedef std::function<void(SQLiteReader& sr)> ReadFunc;

		SQLiteQuery(SQLite& owner);
		SQLiteQuery(SQLite& owner, char const* const& sql, int const& sqlLen = 0);
		~SQLiteQuery();
		SQLiteQuery() = delete;
		SQLiteQuery(SQLiteQuery const&) = delete;
		SQLiteQuery& operator=(SQLiteQuery const&) = delete;

		// 主用于判断 stmt 是否为空( 是否传入 sql )
		operator bool() const noexcept;

		// 配合 SQLiteQuery(SQLite& owner) 后期传入 sql 以初始化 stmt
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
		void SetParameters(Parameters const& ...ps);
		template<typename Parameter, typename...Parameters>
		void SetParametersCore(int& parmIdx, Parameter const& p, Parameters const& ...ps);
		void SetParametersCore(int& parmIdx);

		void Execute(ReadFunc&& rf = nullptr);
	};


	// 数据库主体
	struct SQLite {
		friend SQLiteQuery;

		// 保存最后一次的错误码
		int lastErrorCode = 0;

		// 保存 / 指向 最后一次的错误信息
		const char* lastErrorMessage = nullptr;
	protected:

		// sqlite3 上下文
		sqlite3* dbctx = nullptr;

		// 临时拿来拼 sql 串
		std::string sqlBuilder;

		// 预创建的一堆常用查询语句
		SQLiteQuery query_BeginTransaction;
		SQLiteQuery query_Commit;
		SQLiteQuery query_Rollback;
		SQLiteQuery query_EndTransaction;
		SQLiteQuery query_TableExists;
		SQLiteQuery query_GetTableCount;
		SQLiteQuery query_Attach;
		SQLiteQuery query_Detach;

		// 为throw错误提供便利
		void ThrowError(int const& errCode, char const* const& errMsg = nullptr);
	public:
		// fn 可以是 :memory: 以创建内存数据库
		SQLite(char const* const& fn, bool const& readOnly = false) noexcept;
		~SQLite();
		SQLite() = delete;
		SQLite(SQLite const&) = delete;
		SQLite& operator=(SQLite const&) = delete;

		// 主用于判断构造函数是否执行成功( db 成功打开 )
		operator bool() const noexcept;

		// 获取受影响行数
		int GetAffectedRows();

		// 下列函数均靠 try 检测是否执行出错

		// 根据 SQL 语句, 创建一个查询对象( 有错误发生将返回空 )
		SQLiteQuery CreateQuery(char const* const& sql, int const& sqlLen = 0);

		// 各种 set pragma( 通常推荐设置 SQLiteJournalModes::WAL 似乎能提升一些 insert 的性能 )

		// 事务数据记录模式( 设成 WAL 能提升一些性能 )
		void SetPragmaJournalMode(SQLiteJournalModes jm);

		// 启用外键约束( 默认为未启用 )
		void SetPragmaForeignKeys(bool enable);

		// 数据写盘模式( Off 最快 )
		void SetPragmaSynchronousType(SQLiteSynchronousTypes st);

		// 临时表处理模式
		void SetPragmaTempStoreType(SQLiteTempStoreTypes tst);

		// 排它锁持有方式( 文件被 单个应用独占情况下 EXCLUSIVE 最快 )
		void SetPragmaLockingMode(SQLiteLockingModes lm);

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

		// 会清空表数据, 并且重置自增计数. 如果存在约束, 有可能清空失败.
		void TruncateTable(char const* const& tn);						// DELETE FROM ?; DELETE FROM sqlite_sequence WHERE name = ?;

		// 直接执行一个 SQL 语句( 相当于 create query + execute + destroy 全套 )
		void Execute(char const* const& sql, int(*selectRowCB)(void* userData, int numCols, char** colValues, char** colNames) = nullptr, void* const& userData = nullptr);
	};






	struct SQLiteReader {
	protected:
		sqlite3_stmt* stmt = nullptr;
	public:
		int numCols = 0;

		SQLiteReader(sqlite3_stmt* const& stmt);
		SQLiteReader() = delete;
		SQLiteReader(SQLiteReader const&) = delete;
		SQLiteReader& operator=(SQLiteReader const&) = delete;

		SQLiteDataTypes GetColumnDataType(int const& colIdx);
		char const* GetColumnName(int const& colIdx);
		bool IsDBNull(int const& colIdx);
		int ReadInt32(int const& colIdx);
		int64_t ReadInt64(int const& colIdx);
		double ReadDouble(int const& colIdx);
		char const* ReadString(int const& colIdx);
		std::pair<char const*, int> ReadText(int const& colIdx);
		std::pair<char const*, int> ReadBlob(int const& colIdx);
	};





	/******************************************************************************************************/
	// impls
	/******************************************************************************************************/

	/***************************************************************/
	// SQLite

	inline SQLite::SQLite(char const* const& fn, bool const& readOnly) noexcept
		: query_BeginTransaction(*this)
		, query_Commit(*this)
		, query_Rollback(*this)
		, query_EndTransaction(*this)
		, query_TableExists(*this)
		, query_GetTableCount(*this)
		, query_Attach(*this)
		, query_Detach(*this)
	{
		int r = sqlite3_open_v2(fn, &dbctx, readOnly ? SQLITE_OPEN_READONLY : (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), nullptr);
		if (r != SQLITE_OK) {
			dbctx = nullptr;
			return;
		}

		query_Attach.SetQuery("ATTACH DATABASE ? AS ?");
		query_Detach.SetQuery("DETACH DATABASE ?");
		query_BeginTransaction.SetQuery("BEGIN TRANSACTION");
		query_Commit.SetQuery("COMMIT TRANSACTION");
		query_Rollback.SetQuery("ROLLBACK TRANSACTION");
		query_EndTransaction.SetQuery("END TRANSACTION");
		query_TableExists.SetQuery("SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?");
		query_GetTableCount.SetQuery("SELECT count(*) FROM sqlite_master WHERE type = 'table'");
	}

	inline SQLite::~SQLite() {
		if (dbctx) {
			sqlite3_close(dbctx);
			dbctx = nullptr;
		}
	}

	inline SQLite::operator bool() const noexcept {
		return dbctx != nullptr;
	}

	inline void SQLite::ThrowError(int const& errCode, char const* const& errMsg) {
		lastErrorCode = errCode;
		lastErrorMessage = errMsg ? errMsg : sqlite3_errmsg(dbctx);
		throw errCode;
	}

	inline int SQLite::GetAffectedRows() {
		return sqlite3_changes(dbctx);
	}

	inline SQLiteQuery SQLite::CreateQuery(char const* const& sql, int const& sqlLen) {
		return SQLiteQuery(*this, sql, sqlLen ? sqlLen : (int)strlen(sql));
	}

	inline void SQLite::SetPragmaSynchronousType(SQLiteSynchronousTypes st) {
		if ((int)st < 0 || (int)st >(int)SQLiteSynchronousTypes::MAX) ThrowError(-1, "bad SQLiteSynchronousTypes");
		sqlBuilder.clear();
		sqlBuilder.append("PRAGMA synchronous = ");
		sqlBuilder.append(SQLiteSynchronousTypes_ss[(int)st]);
		Execute(sqlBuilder.c_str());
	}

	inline void SQLite::SetPragmaJournalMode(SQLiteJournalModes jm) {
		if ((int)jm < 0 || (int)jm >(int)SQLiteJournalModes::MAX) ThrowError(-1, "bad SQLiteJournalModes");
		sqlBuilder.clear();
		sqlBuilder.append("PRAGMA journal_mode = ");
		sqlBuilder.append(SQLiteJournalModes_ss[(int)jm]);
		Execute(sqlBuilder.c_str());
	}

	inline void SQLite::SetPragmaTempStoreType(SQLiteTempStoreTypes tst) {
		if ((int)tst < 0 || (int)tst >(int)SQLiteTempStoreTypes::MAX) ThrowError(-1, "bad SQLiteTempStoreTypes");
		sqlBuilder.clear();
		sqlBuilder.append("PRAGMA temp_store = ");
		sqlBuilder.append(SQLiteTempStoreTypes_ss[(int)tst]);
		Execute(sqlBuilder.c_str());
	}

	inline void SQLite::SetPragmaLockingMode(SQLiteLockingModes lm) {
		if ((int)lm < 0 || (int)lm >(int)SQLiteLockingModes::MAX) ThrowError(-1, "bad SQLiteLockingModes");
		sqlBuilder.clear();
		sqlBuilder.append("PRAGMA locking_mode = ");
		sqlBuilder.append(SQLiteLockingModes_ss[(int)lm]);
		Execute(sqlBuilder.c_str());
	}

	inline void SQLite::SetPragmaCacheSize(int cacheSize) {
		if (cacheSize < 1) ThrowError(-1, "bad cacheSize( default is 2000 )");
		sqlBuilder.clear();
		sqlBuilder.append("PRAGMA cache_size = ");
		sqlBuilder.append(std::to_string(cacheSize));
		Execute(sqlBuilder.c_str());
	}

	inline void SQLite::SetPragmaForeignKeys(bool enable) {
		sqlBuilder.clear();
		sqlBuilder.append("PRAGMA foreign_keys = ");
		sqlBuilder.append(enable ? "true" : "false");
		Execute(sqlBuilder.c_str());
	}

	inline void SQLite::Execute(char const* const& sql, int(*selectRowCB)(void* userData, int numCols, char** colValues, char** colNames), void* const& userData) {
		lastErrorCode = sqlite3_exec(dbctx, sql, selectRowCB, userData, (char**)&lastErrorMessage);
		if (lastErrorCode != SQLITE_OK) throw lastErrorCode;
	}


	inline void SQLite::Attach(char const* const& alias, char const* const& fn) {
		query_Attach.SetParameters(fn, alias);
		query_Attach.Execute();
	}

	inline void SQLite::Detach(char const* const& alias) {
		query_Detach.SetParameters(alias);
		query_Detach.Execute();
	}

	inline void SQLite::BeginTransaction() {
		query_BeginTransaction.Execute();
	}

	inline void SQLite::Commit() {
		query_Commit.Execute();
	}

	inline void SQLite::Rollback() {
		query_Rollback.Execute();
	}

	inline void SQLite::EndTransaction() {
		query_EndTransaction.Execute();
	}

	inline bool SQLite::TableExists(char const* const& tn) {
		query_TableExists.SetParameters(tn);
		bool exists = false;
		query_TableExists.Execute([&](SQLiteReader& dr) {
			exists = dr.ReadInt32(0) > 0;
		});
		return exists;
	}

	inline int SQLite::GetTableCount() {
		int count = 0;
		query_GetTableCount.Execute([&](SQLiteReader& dr) {
			count = dr.ReadInt32(0);
		});
		return count;
	}

	inline void SQLite::TruncateTable(char const* const& tn) {
		// todo: 对 tn 转义
		sqlBuilder.clear();
		sqlBuilder += std::string("BEGIN; DELETE FROM [") + tn + "]; DELETE FROM [sqlite_sequence] WHERE [name] = '" + tn + "'; COMMIT;";
		Execute(sqlBuilder.c_str());
	}




	/***************************************************************/
	// SQLiteQuery

	inline SQLiteQuery::SQLiteQuery(SQLite& owner)
		: owner(owner)
	{
	}

	inline SQLiteQuery::SQLiteQuery(SQLite& owner, char const* const& sql, int const& sqlLen)
		: owner(owner)
	{
		SetQuery(sql, sqlLen);
	}

	inline SQLiteQuery::~SQLiteQuery() {
		Clear();
	}

	inline SQLiteQuery::operator bool() const noexcept {
		return stmt != nullptr;
	}

	inline void SQLiteQuery::SetQuery(char const* const& sql, int const& sqlLen) {
		Clear();
		auto r = sqlite3_prepare_v2(owner.dbctx, sql, sqlLen ? sqlLen : (int)strlen(sql), &stmt, nullptr);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::Clear() noexcept {
		if (stmt) {
			sqlite3_finalize(stmt);
			stmt = nullptr;
		}
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, int const& v) {
		assert(stmt);
		auto r = sqlite3_bind_int(stmt, parmIdx, v);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, int64_t const& v) {
		assert(stmt);
		auto r = sqlite3_bind_int64(stmt, parmIdx, v);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, double const& v) {
		assert(stmt);
		auto r = sqlite3_bind_double(stmt, parmIdx, v);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, char const* const& str, int strLen, bool const& makeCopy) {
		assert(stmt);
		int r = SQLITE_OK;
		if (!str) r = sqlite3_bind_null(stmt, parmIdx);
		if (r != SQLITE_OK) owner.ThrowError(r);
		r = sqlite3_bind_text(stmt, parmIdx, str, strLen ? strLen : (int)strlen(str), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, char const* const& buf, size_t const& len, bool const& makeCopy) {
		assert(stmt);
		int r = SQLITE_OK;
		if (!buf) r = sqlite3_bind_null(stmt, parmIdx);
		if (r != SQLITE_OK) owner.ThrowError(r);
		r = sqlite3_bind_blob(stmt, parmIdx, buf, (int)len, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, std::string* const& str, bool const& makeCopy) {
		assert(stmt);
		int r = SQLITE_OK;
		if (!str) r = sqlite3_bind_null(stmt, parmIdx);
		if (r != SQLITE_OK) owner.ThrowError(r);
		r = sqlite3_bind_text(stmt, parmIdx, (char*)str->data(), (int)str->size(), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, std::string const& str, bool const& makeCopy) {
		assert(stmt);
		auto r = sqlite3_bind_text(stmt, parmIdx, (char*)str.data(), (int)str.size(), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	inline void SQLiteQuery::SetParameter(int parmIdx, std::shared_ptr<std::string> const& str, bool const& makeCopy) {
		assert(stmt);
		int r = SQLITE_OK;
		if (!str) r = sqlite3_bind_null(stmt, parmIdx);
		if (r != SQLITE_OK) owner.ThrowError(r);
		r = sqlite3_bind_text(stmt, parmIdx, (char*)str->data(), (int)str->size(), makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
		if (r != SQLITE_OK) owner.ThrowError(r);
	}

	//inline void SQLiteQuery::SetParameter(int parmIdx, BBuffer* const& buf, bool const& makeCopy) {
	//assert(stmt);
	//	int r = SQLITE_OK;
	//	if (!buf) r = sqlite3_bind_null(stmt, parmIdx);
	//	if (r != SQLITE_OK) owner.ThrowError(r);
	//	r = sqlite3_bind_blob(stmt, parmIdx, buf->buf, (int)buf->dataLen, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
	//	if (r != SQLITE_OK) owner.ThrowError(r);
	//}

	//inline void SQLiteQuery::SetParameter(int parmIdx, BBuffer const& buf, bool const& makeCopy) {
	//assert(stmt);
	//	auto r = sqlite3_bind_blob(stmt, parmIdx, buf.buf, (int)buf.dataLen, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
	//	if (r != SQLITE_OK) owner.ThrowError(r);
	//}

	//inline void SQLiteQuery::SetParameter(int parmIdx, BBuffer_p const& buf, bool const& makeCopy) {
	//assert(stmt);
	//	int r = SQLITE_OK;
	//	if (!buf) r = sqlite3_bind_null(stmt, parmIdx);
	//	if (r != SQLITE_OK) owner.ThrowError(r);
	//	r = sqlite3_bind_blob(stmt, parmIdx, buf->buf, (int)buf->dataLen, makeCopy ? SQLITE_TRANSIENT : SQLITE_STATIC);
	//	if (r != SQLITE_OK) owner.ThrowError(r);
	//}

	template<typename EnumType>
	void SQLiteQuery::SetParameter(int parmIdx, EnumType const& v) {
		assert(stmt);
		static_assert(std::is_enum<EnumType>::value, "parameter only support sqlite base types and enum types.");
		if constexpr (sizeof(EnumType) <= 4) SetParameter(parmIdx, (int)(typename std::underlying_type<EnumType>::type)v);
		else SetParameter(parmIdx, (int64_t)(typename std::underlying_type<EnumType>::type)v);
	}

	template<typename...Parameters>
	void SQLiteQuery::SetParameters(Parameters const&...ps) {
		assert(stmt);
		int parmIdx = 1;
		SetParametersCore(parmIdx, ps...);
	}

	template<typename Parameter, typename...Parameters>
	void SQLiteQuery::SetParametersCore(int& parmIdx, Parameter const& p, Parameters const&...ps) {
		assert(stmt);
		SetParameter(parmIdx, p);
		SetParametersCore(++parmIdx, ps...);
	}

	inline void SQLiteQuery::SetParametersCore(int& parmIdx) {}

	inline void SQLiteQuery::Execute(ReadFunc && rf) {
		assert(stmt);
		SQLiteReader dr(stmt);

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
		if (r == SQLITE_OK) return;

	LabErr:
		auto ec = r;
		auto em = sqlite3_errmsg(owner.dbctx);
		sqlite3_reset(stmt);
		owner.ThrowError(ec, em);
	}



	/***************************************************************/
	// SQLiteReader

	inline SQLiteReader::SQLiteReader(sqlite3_stmt* const& stmt) : stmt(stmt) {}

	inline SQLiteDataTypes SQLiteReader::GetColumnDataType(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols);
		return (SQLiteDataTypes)sqlite3_column_type(stmt, colIdx);
	}

	inline char const* SQLiteReader::GetColumnName(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols);
		return sqlite3_column_name(stmt, colIdx);
	}

	inline bool SQLiteReader::IsDBNull(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols);
		return GetColumnDataType(colIdx) == SQLiteDataTypes::Null;
	}

	inline char const* SQLiteReader::ReadString(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols);
		if (IsDBNull(colIdx)) return nullptr;
		return (char const*)sqlite3_column_text(stmt, colIdx);
	}

	inline int SQLiteReader::ReadInt32(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols && !IsDBNull(colIdx));
		return sqlite3_column_int(stmt, colIdx);
	}

	inline int64_t SQLiteReader::ReadInt64(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols && !IsDBNull(colIdx));
		return sqlite3_column_int64(stmt, colIdx);
	}

	inline double SQLiteReader::ReadDouble(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols && !IsDBNull(colIdx));
		return sqlite3_column_double(stmt, colIdx);
	}

	inline std::pair<char const*, int> SQLiteReader::ReadText(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols);
		auto ptr = (char const*)sqlite3_column_text(stmt, colIdx);
		auto len = sqlite3_column_bytes(stmt, colIdx);
		return std::make_pair(ptr, len);
	}

	inline std::pair<char const*, int> SQLiteReader::ReadBlob(int const& colIdx) {
		assert(colIdx >= 0 && colIdx < numCols);
		auto ptr = (char const*)sqlite3_column_blob(stmt, colIdx);
		auto len = sqlite3_column_bytes(stmt, colIdx);
		return std::make_pair(ptr, len);
	}

}
