#pragma once
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <vector>
#include <queue>

namespace xx {
	struct ThreadPool {
		ThreadPool(size_t const& count = 4) {
			for (size_t i = 0; i < count; ++i) {
				ts.emplace_back([this]() {
					while (true) {
						std::function<void()> f;
						{
							std::unique_lock<std::mutex> lock(m);
							c.wait(lock, [this] { return stoped || !fs.empty(); });
							if (stoped && fs.empty()) return;
							f = std::move(fs.front());
							fs.pop();
						}
						f();
					}
					});
			}
		}

		int Add(std::function<void()>&& f) {
			{
				std::unique_lock<std::mutex> lock(m);
				if (stoped) return -1;
				fs.emplace(std::move(f));
			}
			c.notify_one();
			return 0;
		}

		~ThreadPool() {
			{
				std::unique_lock<std::mutex> lock(m);
				stoped = true;
			}
			c.notify_all();
			for (auto&& t : ts) {
				t.join();
			}
		}

	protected:
		std::vector<std::thread> ts;
		std::queue<std::function<void()>> fs;
		std::condition_variable c;
		std::mutex m;
		bool stoped = false;
	};
}

#include <chrono>
#include <atomic>
#include <iostream>

int main() {
	auto t = std::chrono::steady_clock::now();
	std::atomic<int> n(0);
	{
		xx::ThreadPool tp(1000);
		t = std::chrono::steady_clock::now();
		for (int i = 0; i < 100000; ++i) {
			tp.Add([&] {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				++n;
				});
		}
	}
	std::cout << n << std::endl;
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::steady_clock::now() - t)).count() << std::endl;
	return 0;
}
