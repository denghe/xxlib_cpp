#pragma once
#include <vector>

// 如果面临多重继承，则适合复制代码内容直接整合
namespace xx {
	struct TimeoutManager;

	// 需要超时管理的基类
	struct TimeoutBase {
		~TimeoutBase();
		int timeoutIndex = -1;
		TimeoutBase* timeoutPrev = nullptr;
		TimeoutBase* timeoutNext = nullptr;

		// 返回 管理器 指针
		virtual TimeoutManager* GetTimeoutManager() = 0;

		// 设置超时时间. 时间到即触发 OnTimeout 函数调用. 传入 0 撤销. 要求 interval < wheel size
		int SetTimeout(int const& interval);

		virtual void OnTimeout() = 0;
	};

	// 基于时间轮的超时管理器
	struct TimeoutManager {
		// 只存指针引用, 不管理内存
		std::vector<TimeoutBase*> wheel;
		int cursor = 0;

		// 传入 2^n 的轮子长度
		TimeoutManager(size_t const& maxLen = 1 << 12) {
			wheel.resize(maxLen);
		}

		// 每帧调用一次
		inline void UpdateTimeoutWheel() {
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

	// 返回非 0 表示找不到 管理器 或 参数错误
	inline int TimeoutBase::SetTimeout(int const& interval) {
		auto timeoutManager = GetTimeoutManager();
		assert(timeoutManager);

		// 试着从 wheel 链表中移除
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				timeoutManager->wheel[timeoutIndex] = timeoutNext;
			}
		}

		// 检查是否传入间隔时间
		if (interval) {
			// 如果设置了新的超时时间, 则放入相应的链表
			// 安全检查
			if (interval < 0 || interval >(int)timeoutManager->wheel.size()) return -2;

			// 环形定位到 wheel 元素目标链表下标
			timeoutIndex = (interval + timeoutManager->cursor) & ((int)timeoutManager->wheel.size() - 1);

			// 成为链表头
			timeoutPrev = nullptr;
			timeoutNext = timeoutManager->wheel[timeoutIndex];
			timeoutManager->wheel[timeoutIndex] = this;

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

	inline TimeoutBase::~TimeoutBase() {
		if (timeoutIndex != -1) {
			SetTimeout(0);
		}
	}
}
