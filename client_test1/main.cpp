#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include "xx_mysql.h"

int main() {
	xx::MySql::Connection conn;
	try {
		conn.Open("192.168.1.230", 3306, "root", "Abc123", "test");
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
	catch (int const& e) {
		std::cout << conn.lastError << std::endl;
		return e;
	}
	return 0;
}
