#pragma once
#include <vector>

namespace xx {
	struct TimeouterManager;

	// 需要超时管理的基类
	struct TimeouterBase {
		virtual ~TimeouterBase() {};

		TimeouterManager* timerManager = nullptr;
		int timeoutIndex = -1;
		TimeouterBase* timeoutPrev = nullptr;
		TimeouterBase* timeoutNext = nullptr;

		// 设置超时时间. 时间到即触发 OnTimeout 函数调用. 传入 0 撤销. 要求 interval < wheel size
		int SetTimeout(int const& interval);

		virtual void OnTimeout() = 0;
	};

	// 基于时间轮的超时管理器
	struct TimeouterManager {
		// 只存指针引用, 不管理内存
		std::vector<TimeouterBase*> wheel;
		int cursor = 0;

		// 传入 2^n 的轮子长度
		TimeouterManager(std::size_t const& maxLen = 1 << 12) {
			wheel.resize(maxLen);
		}

		// 每帧调用一次
		inline void Update() {
			cursor = (cursor + 1) & ((int)wheel.size() - 1);
			auto p = wheel[cursor];
			wheel[cursor] = nullptr;
			while (p) {
				auto np = p->timeoutNext;

				p->timeoutIndex = -1;
				p->timeoutPrev = nullptr;
				p->timeoutNext = nullptr;

				p->OnTimeout();
				p = np;
			};
		}
	};

	inline int TimeouterBase::SetTimeout(int const& interval) {
		// 未绑定时间轮容器
		if (!timerManager) return -1;

		// 试着从 wheel 链表中移除
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				timerManager->wheel[timeoutIndex] = timeoutNext;
			}
		}

		// 检查是否传入间隔时间
		if (interval) {
			// 如果设置了新的超时时间, 则放入相应的链表
			// 安全检查
			if (interval < 0 || interval >(int)timerManager->wheel.size()) return -1;

			// 环形定位到 wheel 元素目标链表下标
			timeoutIndex = (interval + timerManager->cursor) & ((int)timerManager->wheel.size() - 1);

			// 成为链表头
			timeoutPrev = nullptr;
			timeoutNext = timerManager->wheel[timeoutIndex];
			timerManager->wheel[timeoutIndex] = this;

			// 和之前的链表头连起来( 如果有的话 )
			if (timeoutNext) {
				timeoutNext->timeoutPrev = this;
			}
		}
		else {
			// 重置到初始状态
			timeoutIndex = -1;
			timeoutPrev = nullptr;
			timeoutNext = nullptr;
		}

		return 0;
	}
}
