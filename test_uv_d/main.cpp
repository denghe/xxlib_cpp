#include "xx_sqlite.h"
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
	//xx::SQLite db(":memory:");
	xx::SQLite db("test.db3");
	db.SetPragmaJournalMode(xx::SQLiteJournalModes::Memory);	// WAL 可能更快 但是会生成一个 .wal 文件
	db.SetPragmaLockingMode(xx::SQLiteLockingModes::Exclusive);
	db.SetPragmaCacheSize(4096);
	db.SetPragmaTempStoreType(xx::SQLiteTempStoreTypes::Memory);
	try {
		db.Execute(R"-(
DROP TABLE IF EXISTS 't1'
)-");
		db.Execute(R"-(
CREATE TABLE t1 (
	c1 int PRIMARY KEY,
	c2 text NOT NULL
)
)-");
		auto& beginTime = std::chrono::steady_clock::now();
		auto&& q = db.CreateQuery("insert into t1 ('c2') values (?)");
		for (int j = 0; j < 1000; ++j) {
			db.BeginTransaction();
			for (int i = 0; i < 1000; ++i) {
				q.SetParameters("asdf");
				q.Execute();
			}
			db.Commit();
		}
		auto& endTime = std::chrono::steady_clock::now();
		std::cout << "ms = " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() << std::endl;
		auto&& q2 = db.CreateQuery("select count(*) from t1");
		q2.Execute([](xx::SQLiteReader& sr) {
			std::cout << "count(*) = " << sr.ReadInt32(0) << std::endl;
			});
		std::cin.get();
	}
	catch (int const& errCode) {
		std::cout << "errCode = " << errCode << ", errMsg = " << db.lastErrorMessage << std::endl;
	}
	return 0;
}
