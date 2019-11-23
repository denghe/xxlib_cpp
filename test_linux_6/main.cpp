#include <iostream>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>

// 得到当前 system 时间点的 epoch (精度为 ms)
inline int64_t NowSystemEpochMS() noexcept {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int main(int argc, char* argv[]) {
	auto begin = NowSystemEpochMS();
	std::atomic<std::int64_t> n(0);
	std::vector<std::thread> ts;
	for (int j = 0; j < 4; j++) {
		ts.emplace_back([&, j = j] {
			for (std::size_t i = 0; i < 100000000; i++)
			{
				++n;
				if (i % 10000000 == 0) {
					std::cout << "threadId = " << j << ", n = " << n << std::endl;
				}
			}
		});
	}
	for (auto&& t : ts) {
		t.join();
	}
	auto end = NowSystemEpochMS() - begin;
	std::cout << "ms = " << end << ", n = " << n << std::endl;
	return 0;
}
