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
	template<typename T>
	struct DefaultErrorHandler {
		void operator ()(const T& t ,const std::exception& e)
		{
			//todo
		}
		void operator ()(const T& t, const int& code)
		{
			//todo
		}
	};

	template<typename T,typename ErrorHandler = DefaultErrorHandler<T>>
	class ThreadPoolEx {
		std::vector<std::thread> threads;
		std::queue<std::function<void(T&)>> jobs;
		std::mutex mtx;
		std::condition_variable cond;
		bool stop = false;

		ErrorHandler eh;
	public:
		ThreadPoolEx(int const& numThreads = 4, ErrorHandler&& eh = ErrorHandler())
		:eh (std::move(eh))
		{
			for (int i = 0; i < numThreads; ++i) {
				threads.emplace_back([this] {
					T t ;
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
						try
						{
							job(t);
						}
						catch (const std::exception& e)
						{
							this->eh(t,e);
						}
						catch (const int& code)
						{
							this->eh(t, code);
						}
						catch (...)
						{
							//this->eh(t);
						}
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
