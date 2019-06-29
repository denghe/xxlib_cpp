#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include "xx_logger.h"

int main() {
	xx::SQLite::Connection db(":memory:");
	if (!db) return -1;
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

		struct Foo {
			int key;
			int64_t i;
			std::optional<int64_t> oi;
			double d;
			std::optional<double> od;
			std::string s;
			std::optional<std::string> os;
			xx::BBuffer b;
			std::shared_ptr<xx::BBuffer> ob;
		};

		xx::SQLite::Query qInsert(db, "insert into `t1` ('key', 'i', 'oi', 'd', 'od', 's', 'os', 'b', 'ob') values (?, ?, ?, ?, ?, ?, ?, ?, ?)");

		{
			Foo f;
			f.key = 1;
			f.i = 1234567890123;
			f.oi.reset();
			f.d = 1.234;
			f.od = 1.234;
			f.s = "asdf";
			f.os = "asdf";
			f.b.Write(1u, 2u, 3u);
			f.ob = std::make_shared<xx::BBuffer>();
			f.ob->Write(1u, 2u, 3u);
			qInsert.SetParameters(f.key, f.i, f.oi, f.d, f.od, f.s, f.os, f.b, f.ob)();
		}
		{
			Foo f;
			f.key = 2;
			f.i = 0;
			f.oi = 123;
			f.d = 1.234;
			f.od.reset();
			f.s = "asdf";
			f.os.reset();
			f.b.Write(1u, 2u, 3u);
			f.ob.reset();
			qInsert.SetParameters(f.key, f.i, f.oi, f.d, f.od, f.s, f.os, f.b, f.ob)();
		}

		xx::CoutN("count(*) = ", db.Execute<int64_t>("select count(*) from `t1`"));

		xx::SQLite::Query qSelect(db, "select * from `t1`");

		std::vector<Foo> fs;
		qSelect.Execute([&](xx::SQLite::Reader& r) {
			fs.emplace_back();
			auto&& f = fs.back();
			r.Reads(f.key, f.i, f.oi, f.d, f.od, f.s, f.os, f.b, f.ob);
		});

		xx::CoutN("fs.size() = ", fs.size());

		for (auto&& f : fs) {
			xx::CoutN("key = ", f.key
				, "\n, i = ", f.i
				, "\n, oi = ", f.oi
				, "\n, d = ", f.d
				, "\n, od = ", f.od
				, "\n, s = ", f.s
				, "\n, os = ", f.os
				, "\n, b = ", f.b
				, "\n, ob = ", f.ob
			);
		}
	}
	catch (int const& errCode) {
		std::cout << "errCode = " << errCode << ", errMsg = " << db.lastErrorMessage << std::endl;
	}
	return 0;
}



//db.SetPragmaJournalMode(xx::SQLite::JournalModes::Memory);
//db.SetPragmaLockingMode(xx::SQLite::LockingModes::Exclusive);
//db.SetPragmaCacheSize(4096);
//db.SetPragmaTempStoreType(xx::SQLite::TempStoreTypes::Memory);
