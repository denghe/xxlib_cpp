#pragma once
#include <boost/coroutine2/all.hpp>
#include <chrono>

namespace xx {

	using Yield = boost::coroutines2::coroutine<void>::pull_type;
	using Coroutine = boost::coroutines2::coroutine<void>::push_type;

	struct Coroutines {
		std::vector<Coroutine> cors;
		inline void Add(Coroutine&& cf) {
			if (cf) {
				cors.push_back(std::move(cf));
			}
		}
		inline void RunAdd(Coroutine&& cf) {
			cf();
			Add(std::move(cf));
		}
		size_t RunOnce() {
			if (cors.size()) {
				for (decltype(auto) i = cors.size() - 1; i != (size_t)-1; --i) {
					cors[i]();
					if (!cors[i]) {
						if (i + 1 < cors.size()) {
							cors[i] = std::move(cors[cors.size() - 1]);
						}
						cors.pop_back();
					}
				}
			}
			return cors.size();
		}
	};

	void Sleep(Yield& yield, int const& ticks) {
		for (size_t i = 0; i < ticks; i++) {
			yield();
		}
	}
	void Sleep(Yield& yield, std::chrono::system_clock::time_point const& t) {
		while (t > std::chrono::system_clock::now()) {
			yield();
		}
	}
	void Sleep(Yield& yield, std::chrono::seconds const& duration) {
		Sleep(yield, std::chrono::system_clock::now() + duration);
	}

}

/*
Coroutines cors;
cors.Add(Coroutine([&](Yield& yield) {
	yield();
}));
....
while (cors.RunOnce()) {}
*/
