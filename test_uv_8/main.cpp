#include "xx_coros.h"

//#include <iostream>
//int main() {
//	int a;
//	boost_context::continuation c = boost_context::callcc(
//		[&a](boost_context::continuation && c) {
//		a = 0;
//		int b = 1;
//		for (;;) {
//			c = c.resume();
//			int next = a + b;
//			a = b;
//			b = next;
//		}
//		return std::move(c);
//	});
//	for (int j = 0; j < 10; ++j) {
//		std::cout << a << " ";
//		c = c.resume();
//	}
//	std::cout << std::endl;
//	std::cout << "main: done" << std::endl;
//	return EXIT_SUCCESS;
//}

#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
	xx::Coros cors;
	int i = 0;
	cors.Add([&](xx::Coro&& yield) {
		while (++i < 11111111) {
			yield();
		}
	});
	cors.Add([&](xx::Coro&& yield) {
		while (++i < 11111111) {
			yield();
		}
	});
	cors.Add([&](xx::Coro&& yield) {
		while (++i < 11111111) {
			yield();
		}
	});
	cors.Add([&](xx::Coro&& yield) {
		while (++i < 11111111) {
			yield();
		}
	});
	cors.Add([&](xx::Coro&& yield) {
		while (++i < 11111111) {
			yield();
		}
	});
	auto&& t = std::chrono::system_clock::now();
	while (cors.RunOnce()) {}
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t).count() << std::endl;
	return 0;
}
