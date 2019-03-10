#pragma once
#include "xx_uv.h"
#include "xx_stackless.h"

namespace xx {

	struct UvLoopStackless : UvLoop, Stackless {
		std::chrono::time_point<std::chrono::system_clock> corsLastTime;
		std::chrono::nanoseconds corsDurationPool;
		std::shared_ptr<UvTimer> corsTimer;
		UvLoopStackless(double const& framesPerSecond) : UvLoop() {
			if (funcs.size()) throw - 1;
			corsTimer = CreateTimer<>(0, 1);
			if (!corsTimer) throw - 2;
			corsLastTime = std::chrono::system_clock::now();
			corsDurationPool = std::chrono::nanoseconds(0);
			corsTimer->OnFire = [this, nanosPerFrame = std::chrono::nanoseconds(int64_t(1.0 / framesPerSecond * 1000000000))]{
				auto currTime = std::chrono::system_clock::now();
				corsDurationPool += currTime - corsLastTime;
				corsLastTime = currTime;
				while (corsDurationPool > nanosPerFrame) {
					if (!RunOnce()) {
						corsTimer.reset();
						return;
					};
					corsDurationPool -= nanosPerFrame;
				}
			};
		}
	};

}
