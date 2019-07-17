#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace xx {
	class ThreadPool {
		std::vector<std::thread> threads;
		std::queue<std::function<void()>> jobs;
		std::mutex mtx;
		std::condition_variable cond;
		bool stop = false;

	public:
		ThreadPool(int const& numThreads = 4) {
			for (int i = 0; i < numThreads; ++i) {
				threads.emplace_back([this] {
					while (true) {
						std::function<void()> job;
						{
							std::unique_lock<std::mutex> lock(this->mtx);
							this->cond.wait(lock, [this] {
								return this->stop || !this->jobs.empty();
								});
							if (this->stop && this->jobs.empty()) return;
							job = std::move(this->jobs.front());
							this->jobs.pop();
						}
						job();
					}
					});
			}
		}

		int Add(std::function<void()>&& job) {
			{
				std::unique_lock<std::mutex> lock(mtx);
				if (stop) return -1;
				jobs.emplace(std::move(job));
			}
			cond.notify_one();
			return 0;
		}

		~ThreadPool() {
			{
				std::unique_lock<std::mutex> lock(mtx);
				stop = true;
			}
			cond.notify_all();
			for (std::thread& worker : threads) {
				worker.join();
			}
		}
	};
}
