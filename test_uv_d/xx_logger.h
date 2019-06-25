#pragma once
#include "xx_sqlite.h"
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>

namespace xx {

	// 对应 sqlite log 表 中的一行
	struct LogRow {
		//int64_t id = 0;				// 主键
		int64_t time = 0;				// 发生时间
		std::string desc;				// 日志明细

		LogRow() = default;
		LogRow(LogRow const&) = delete;
		LogRow& operator=(LogRow const&) = delete;
		LogRow(LogRow&& o)
			: time(std::move(o.time))
			, desc(std::move(o.desc))
		{
		}
		inline LogRow& operator=(LogRow&& o) {
			this->time = std::move(o.time);
			this->desc = std::move(o.desc);
			return *this;
		}
	};

	// sqlite 版日志写入器, 每秒最多只能写入几十万行, 特点是空间可以循环利用, 自动删除过期数据
	struct SqliteLogWriter {
		// sqlite 连接主体
		SQLite::Connection db;

		// 要用到的查询
		SQLite::Query query_Insert;
		SQLite::Query query_DeleteRange;
		SQLite::Query query_DeleteAll;

		// 表数据行数限制. insert 的同时将执行 delete 删除最早的数据以维持总行数, 滚动利用存储空间
		int64_t rowsLimit = 0;

		// 当前自增id( 用前++ )
		int64_t autoIncId = 0;

		// 当前数据行数
		int64_t numRows = 0;

		SqliteLogWriter(char const* const& logFileName, bool cleanup, int64_t rowsLimit)
			: db(logFileName)
			, query_Insert(db)
			, query_DeleteRange(db)
			, query_DeleteAll(db)
			, rowsLimit(rowsLimit)
		{
			if (rowsLimit <= 0) {
				rowsLimit = std::numeric_limits<decltype(rowsLimit)>::max();
			}

			// 优化选项
			db.SetPragmaJournalMode(xx::SQLite::JournalModes::Memory);
			db.SetPragmaTempStoreType(xx::SQLite::TempStoreTypes::Memory);
			db.SetPragmaCacheSize(4096);

			// 建表
			if (!db.TableExists("log")) {
				db.Call(R"=-=(
CREATE TABLE `log`(
    `id` INTEGER PRIMARY KEY NOT NULL UNIQUE,
    `time` INTEGER NOT NULL, 
    `desc` TEXT NOT NULL
);
)=-=");
				db.Call(R"=-=(
CREATE VIEW `view_log` AS
SELECT `id`, `time`, datetime(`time` / 10000000, 'unixepoch') as `datetime`, `desc`
  FROM `log`;
)=-=");
				// time 创建索引??
			}
			else {
				// 初始化查询器
				query_DeleteAll.SetQuery("delete from `log`");

				if (cleanup) {
					// 清数据
					query_DeleteAll();
				}
				else {
					// 取最大, 最小 id 值
					autoIncId = db.Execute<int64_t>("select max(`id`) from `log`");
					numRows = db.Execute<int64_t>("select min(`id`) from `log`") - autoIncId + 1;
				}
			}

			// 继续初始化查询器
			query_Insert.SetQuery("insert into `log` (`id`, `time`, `desc`) values (?, ?, ?)");
			query_DeleteRange.SetQuery("delete from `log` where id <= ?");

			// 检测当前数据是否超限. 超了就删除一部分
			if (numRows > rowsLimit) {
				query_DeleteRange.SetParameters(autoIncId - rowsLimit)();
				numRows = rowsLimit;
			}
		}
		SqliteLogWriter(SqliteLogWriter const&) = delete;
		SqliteLogWriter& operator=(SqliteLogWriter const&) = delete;

		// 将队列数据写盘. 成功返回 0. 失败 -1
		inline int Save(std::vector<LogRow> const& rows) {
			int64_t idx = 0, len = (int64_t)rows.size();
			assert(len);
			try {
				// 如果本次写入必然导致数据库行数超限制, 就直接清库，并调整 idx 指向 rows 后方符合 rowsLimit 的起始处
				if (len >= rowsLimit) {
					// 如果库有数据就清
					if (numRows) {
						query_DeleteAll();
					}
					// 调整指向, 确保写入数据在 rowsLimit 范围内
					idx = len - rowsLimit;

					// 提前计算出写入完成之后的记录数
					numRows = rowsLimit;
				}
				else {
					// 计算 db 中要留多少行, 
					auto newRowsLimit = rowsLimit - len;
					if (numRows > newRowsLimit) {
						query_DeleteRange.SetParameters(autoIncId - newRowsLimit)();
					}

					// 提前计算出写入完成之后的记录数
					numRows = newRowsLimit + len;
				}
				if (idx == len) return 0;

				db.BeginTransaction();
				for (; idx < len; ++idx) {
					auto&& o = rows[idx];
					query_Insert.SetParameters(++autoIncId, o.time, o.desc)();
				}
				db.EndTransaction();
				return 0;
			}
			catch (...)
			{
				std::cout << "logdb insert error! errNO = " << db.lastErrorCode << " errMsg = " << db.lastErrorMessage << std::endl;
				return -1;
			}
		}
	};

	// todo: 文本版日志写入器, 写入速度受限于磁盘, 特点是可以根据 行数 啥的条件拆分文件. 自动在文件名后添加后缀存为多个文件

	// 日志记录器
	template<typename LogWriter = SqliteLogWriter>
	class Logger {
	protected:
		// 内存队列限长. 超限将放弃本次写入，返回 -1: 写入失败
		uint64_t queueLimit = 0;

		// 切换锁定依赖
		std::mutex mtx;

		// 切换使用的双队列
		std::vector<LogRow> rows;
		std::vector<LogRow> bgRows;

		// 通知后台线程退出的标志位
		int disposing = 0;

		// 日志写入器
		LogWriter writer;

	public:
		// 用于外界查询当前后台线程工作状态
		bool writing = false;

	protected:

		// 从构造函数中剥离以便于构造函数路由不同的 LogWriter
		void Init() {
			// 预分配内存
			rows.reserve(queueLimit);
			bgRows.reserve(queueLimit);

			// 起一个后台线程用于日志写库
			std::thread t([this] {
				while (true) {
					// 切换前后台队列( 如果有数据. 没有就 sleep 一下继续扫 )
					{
						std::lock_guard<std::mutex> lg(mtx);
						if (!rows.size()) goto LabEnd;
						std::swap(rows, bgRows);
					}

					writing = true;
					if (writer.Save(bgRows)) {
						writing = false;
						disposing = -1;
						return;
					}
					writing = false;
					bgRows.clear();
					continue;

				LabEnd:
					if (disposing == 1) {
						++disposing;
						continue;
					}
					else if (disposing == 2) {
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				disposing = 0;
				});
			t.detach();
		}

	public:

		template<typename ENABLED = std::enable_if_t<std::is_base_of_v<SqliteLogWriter, LogWriter>>>
		Logger(char const* const& logFileName, bool cleanup = false, int64_t rowsLimit = 1000000, uint64_t queueLimit = 1000000)
			: queueLimit(queueLimit)
			, writer(logFileName, cleanup, rowsLimit) {
			Init();
		}

		~Logger() {
			if (disposing) return;
			disposing = 1;
			while (disposing) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		Logger(Logger const&) = delete;
		Logger& operator=(Logger const&) = delete;

		// 往内存队列插入一条日志. 返回 非0 表示失败( 可能超过了数量限制 或者是实例正在析构 或已析构 )
		template<typename DescType>
		int Write(DescType&& desc) noexcept {
			std::lock_guard<std::mutex> lg(mtx);
			if (disposing || (queueLimit && rows.size() > queueLimit)) return -1;

			typedef std::chrono::duration<long long, std::ratio<1LL, 10000000LL>> MicroX10;	// 10 倍 micro 精度. 当前 x86 pc 常见精度
			rows.emplace_back();
			auto&& o = rows.back();
			o.time = std::chrono::duration_cast<MicroX10>(std::chrono::system_clock::now().time_since_epoch()).count();
			o.desc = std::forward<DescType>(desc);
			return 0;
		}
	};
}
