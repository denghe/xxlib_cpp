#pragma once

#include "xx_bbuffer.h"
#include <boost/context/all.hpp>

namespace xx {

	struct Coros;
	struct Coro {
		boost::context::continuation c1;
		boost::context::continuation c2;
		std::size_t idx = -1;
		Coros& coros;

		void operator()() {
			c2 = c2.resume();
		}

		Coro(Coros& coros, boost::context::continuation&& c);
		~Coro();
		Coro(Coro&&) = delete;
		Coro& operator=(Coro&&) = delete;
		Coro(Coro&) = delete;
		Coro& operator=(Coro&) = delete;
	};

	struct Coros {
		List<Coro*> resumers;

		void Add(std::function<void(Coro& yield)>&& job, bool runImmediately = false) {

			auto&& c1 = boost::context::callcc([this, job = std::move(job)](boost::context::continuation&& c2) {
				Coro co(*const_cast<Coros*>(this), std::move(c2));
				co.c2 = co.c2.resume();
				job(co);
				return std::move(co.c2);
			});

			auto&& c = resumers[resumers.len - 1]->c1;
			c = std::move(c1);

			if (runImmediately) {
				c = c.resume();
			}
		}

		void RunOnce() {
			for (int i = (int)resumers.len - 1; i >= 0; --i) {
				auto&& co = resumers[i];
				co->c1 = co->c1.resume();
			}
		}

		void Run() {
			while (resumers.len) {
				RunOnce();
			}
		}
	};

	inline Coro::Coro(Coros& coros, boost::context::continuation&& c)
		: c2(std::move(c))
		, coros(coros)
	{
		idx = coros.resumers.len;
		coros.resumers.Add(this);
	}

	inline Coro::~Coro() {
		coros.resumers[coros.resumers.len - 1]->idx = idx;
		coros.resumers.SwapRemoveAt(idx);
	}

}
