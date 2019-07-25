#include <iostream>
#include <chrono>
#include "xx_logger.h"
#include <exception>
//#ifndef NDEBUG
//_ACRTIMP void __cdecl _wassert(
//	_In_z_ wchar_t const* _Message,
//	_In_z_ wchar_t const* _File,
//	_In_   unsigned       _Line
//);
//#endif
//#define ENABLE_XXASSERT 0
//#if ENABLE_XXASSERT
//#    define xxAssert(expression) (void)(                                                       \
//            (!!(expression)) ||                                                              \
//            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
//        )
//#else 
//#    define xxAssert(expression) assert(expression)
//#endif
//
//
//#ifdef NDEBUG
//
//#define xxassert(expression) ((void)0)
//
//#else
//
//_ACRTIMP void __cdecl _wassert(
//	_In_z_ wchar_t const* _Message,
//	_In_z_ wchar_t const* _File,
//	_In_   unsigned       _Line
//);
//
//#define xxassert(expression) (void)(                                                       \
//            (!!(expression)) ||                                                              \
//            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
//        )
//
//#endif


#ifdef NDEBUG
#define ENABLE_XXASSERT 1
#if ENABLE_XXASSERT
#    define xxAssert(expression) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (wprintf_s(L"%s %s %d\n", _CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)))|| \
            ((throw -1),0) \
        )
#else 
#    define xxAssert(expression) ((void)0)
#endif
#endif
int main(int argc, char* argv[]) {
	//xxAssert(false);
	auto a = 0.0f;
	float  o = -0.0f / a;
	std::cout << o;
	using namespace std::chrono;
	std::string logFN = argv[0];
	logFN += ".log.db3";
	{
		xx::Logger log(logFN.c_str(), true);

		auto&& t2 = steady_clock::now();
		for (int j = 0; j < 10; ++j) {
			auto&& t = steady_clock::now();
			for (int i = 0; i < 1000000; ++i) {
				log.Write("asdfqeradflkasdfljlsjdflkjasdkfljalkdfjlsajdfl");
			}
			std::cout << duration_cast<milliseconds>(steady_clock::now() - t).count() << std::endl;
			std::this_thread::sleep_for(50ms);
			while (log.writing) {
				std::this_thread::sleep_for(50ms);
			}
		}
		std::cout << "total: " << duration_cast<milliseconds>(steady_clock::now() - t2).count() << std::endl;
	}

//	//xx::SQLite::Connection db(":memory:");
//	xx::SQLite::Connection db("test.db3");
//	db.SetPragmaJournalMode(xx::SQLite::JournalModes::Memory);	// WAL 可能更快 但是会生成一个 .wal 文件
//	db.SetPragmaLockingMode(xx::SQLite::LockingModes::Exclusive);
//	db.SetPragmaCacheSize(4096);
//	db.SetPragmaTempStoreType(xx::SQLite::TempStoreTypes::Memory);
//	try {
//		db.Call(R"-(
//DROP TABLE IF EXISTS 't1'
//)-");
//		db.Call(R"-(
//CREATE TABLE t1 (
//	c1 int PRIMARY KEY,
//	c2 text NOT NULL
//)
//)-");
//		auto& beginTime = std::chrono::steady_clock::now();
//		xx::SQLite::Query q(db, "insert into t1 ('c2') values (?)");
//		for (int j = 0; j < 1000; ++j) {
//			db.BeginTransaction();
//			for (int i = 0; i < 1000; ++i) {
//				q.SetParameters("asdf")();
//			}
//			db.Commit();
//		}
//		auto& endTime = std::chrono::steady_clock::now();
//		std::cout << "ms = " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() << std::endl;
//
//		std::cout << "count(*) = " << db.Execute<int>("select count(*) from t1") << std::endl;
//		std::cin.get();
//	}
//	catch (int const& errCode) {
//		std::cout << "errCode = " << errCode << ", errMsg = " << db.lastErrorMessage << std::endl;
//	}
//




	return 0;
}
