#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "xx_object.h"

namespace xx {

	/*
	T sample:
	struct Foo {
		Foo() = default;
		void operator()(std::function<void(Foo&)>& job)
		{
			try {
				job(*this);
			}
			catch (int const& e) {
				xx::CoutN("catch throw int: ", e);
			}
			catch (std::exception const& e) {
				xx::CoutN("catch throw std::exception: ", e);
			}
			catch (...) {
				xx::CoutN("catch ...");
			}
		}
	};
	*/

	template<typename T>
	class ThreadPoolEx {
		std::vector<std::thread> threads;
		std::queue<std::function<void(T&)>> jobs;
		std::mutex mtx;
		std::condition_variable cond;
		bool stop = false;

	public:
		ThreadPoolEx(int const& numThreads = 4) {
			for (int i = 0; i < numThreads; ++i) {
				threads.emplace_back([this] {
					T t ;													// require default constructor
					while (true) {
						std::function<void(T&)> job;
						{
							std::unique_lock<std::mutex> lock(this->mtx);
							this->cond.wait(lock, [this] {
								return this->stop || !this->jobs.empty();
								});
							if (this->stop && this->jobs.empty()) return;
							job = std::move(this->jobs.front());
							this->jobs.pop();
						}
						t(job);												// require void operator()(std::function<void(T&)>& job) { job(*this); }
					}
					});
			}
		}

		int Add(std::function<void(T&)>&& job) {
			{
				std::unique_lock<std::mutex> lock(mtx);
				if (stop) return -1;
				jobs.emplace(std::move(job));
			}
			cond.notify_one();
			return 0;
		}

		~ThreadPoolEx() {
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
