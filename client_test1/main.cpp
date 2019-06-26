#pragma execution_character_set("utf-8")
#include <xx_logger.h>

int main() {
	xx::SQLite::Connection db(":memory:");
	db.SetPragmaJournalMode(xx::SQLite::JournalModes::Memory);
	db.SetPragmaLockingMode(xx::SQLite::LockingModes::Exclusive);
	db.SetPragmaCacheSize(4096);
	db.SetPragmaTempStoreType(xx::SQLite::TempStoreTypes::Memory);

	try {
		db.Call(R"-(
CREATE TABLE `t1` (
	`key` int primary key,
	`i` int not null,
	`oi` int,
	`d` double not null,
	`od` double,
	`s` text not null,
	`os` text,
	`b` blob not null,
	`ob` blob
)
)-");

		struct Row {
			int key;
			int64_t i;
			std::optional<int64_t> oi;
			double d;
			std::optional<double> od;
			std::string s;
			std::optional<std::string> os;
			xx::BBuffer b;
			std::optional<xx::BBuffer> ob;
		};

		xx::SQLite::Query qInsert(db, "insert into `t1` ('key', 'i', 'oi', 'd', 'od', 's', 'os', 'b', 'ob') values (?, ?, ?, ?, ?, ?, ?, ?, ?)");

		Row r;
		r.key = 1;
		r.i = 1234567890123;
		r.oi.reset();
		r.d = 1.234;
		r.od = 1.234;
		r.s = "asdf";
		r.os = "asdf";
		r.b.Write(1u, 2u, 3u);
		r.ob = xx::BBuffer{};
		r.ob->Write(1u, 2u, 3u);
		qInsert.SetParameters(r.key, r.i, r.oi, r.d, r.od, r.s, r.os, r.b, r.ob)();

		r.key = 2;
		r.i = 0;
		r.oi = 123;
		r.d = 1.234;
		r.od.reset();
		r.s = "asdf";
		r.os.reset();
		r.b.Write(1u, 2u, 3u);
		r.ob.reset();
		qInsert.SetParameters(r.key, r.i, r.oi, r.d, r.od, r.s, r.os, r.b, r.ob)();

		xx::CoutN("count(*) = ", db.Execute<int64_t>("select count(*) from `t1`"));

		xx::SQLite::Query qSelect(db, "select * from `t1`");
		qSelect.Execute([](xx::SQLite::Reader& r) {
			xx::CoutN("key = ", r.ReadInt32(0));
		});
	}
	catch (int const& errCode) {
		std::cout << "errCode = " << errCode << ", errMsg = " << db.lastErrorMessage << std::endl;
	}
	return 0;
}
