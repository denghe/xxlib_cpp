#pragma once
#include "xx_uv.h"
#include "xx_coroutine.h"

namespace xx {

	struct UvLoopCoroutine : UvLoop, Coroutines {
		std::chrono::time_point<std::chrono::system_clock> corsLastTime;
		std::chrono::nanoseconds corsDurationPool;
		UvLoopCoroutine(double const& framesPerSecond) : UvLoop() {
			if (cors.size()) throw - 1;
			auto timer = CreateTimer<>(0, 1);
			if (!timer) throw - 2;
			corsLastTime = std::chrono::system_clock::now();
			corsDurationPool = std::chrono::nanoseconds(0);
			timer->OnFire = [this, timer, nanosPerFrame = std::chrono::nanoseconds(int64_t(1.0 / framesPerSecond * 1000000000))]{
				auto currTime = std::chrono::system_clock::now();
				corsDurationPool += currTime - corsLastTime;
				corsLastTime = currTime;
				while (corsDurationPool > nanosPerFrame) {
					if (!RunOnce()) {
						timer->OnFire = nullptr;
						return;
					};
					corsDurationPool -= nanosPerFrame;
				}
			};
		}
	};

}
