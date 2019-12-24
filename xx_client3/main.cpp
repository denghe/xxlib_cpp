#include "xx_object.h"
#include "xx_mysql.h"
#include "xx_threadpool_ex.h"
#include <atomic>

struct MySqlConn : xx::MySql::Connection {
	MySqlConn() {
		Open("192.168.1.109", 3306, "root", "1", "test1");
	}
	void operator()(std::function<void(MySqlConn&)>& job) {
		job(*this);
	}
};

int main() {

	auto ms = xx::NowSteadyEpochMS();
	{
		xx::ThreadPoolEx<MySqlConn> tp(1);

		std::atomic<std::size_t> n = 0;

		for (int i = 0; i < 10000; ++i) {
			tp.Add([&](MySqlConn& conn) {
				conn.Execute(
					"BEGIN;"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"INSERT INTO `xx` (`Column 1`, `Column 2`, `Column 3`, `Column 4`, `Column 5`) VALUES (1,2,3,4,5);"
					"COMMIT;"
				);
				conn.DropResult();
				auto tmp = ++n;
				if (tmp % 1000 == 0) {
					xx::CoutN(", n = ", tmp);
				}
				});
		}
	}
	xx::CoutN("ms = ", xx::NowSteadyEpochMS() - ms);

	return 0;
}
