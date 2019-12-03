#pragma once
#include <vector>

namespace xx {
	struct TimeoutManager;

	// ��Ҫ��ʱ����Ļ���
	struct TimeoutBase {
		int timeoutIndex = -1;
		TimeoutBase* timeoutPrev = nullptr;
		TimeoutBase* timeoutNext = nullptr;

		// ���� ������ ָ��
		virtual TimeoutManager* GetTimeoutManager() = 0;

		// ���ó�ʱʱ��. ʱ�䵽������ OnTimeout ��������. ���� 0 ����. Ҫ�� interval < wheel size
		int SetTimeout(int const& interval);

		virtual void OnTimeout() = 0;
	};

	// ����ʱ���ֵĳ�ʱ������
	struct TimeoutManager {
		// ֻ��ָ������, �������ڴ�
		std::vector<TimeoutBase*> wheel;
		int cursor = 0;

		// ���� 2^n �����ӳ���
		TimeoutManager(std::size_t const& maxLen = 1 << 12) {
			wheel.resize(maxLen);
		}

		// ÿ֡����һ��
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

	// ���ط� 0 ��ʾ�Ҳ��� ������ �� ��������
	inline int TimeoutBase::SetTimeout(int const& interval) {
		auto timeoutManager = GetTimeoutManager();
		assert(timeoutManager);

		// ���Ŵ� wheel �������Ƴ�
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

		// ����Ƿ�����ʱ��
		if (interval) {
			// ����������µĳ�ʱʱ��, �������Ӧ������
			// ��ȫ���
			if (interval < 0 || interval >(int)timeoutManager->wheel.size()) return -2;

			// ���ζ�λ�� wheel Ԫ��Ŀ�������±�
			timeoutIndex = (interval + timeoutManager->cursor) & ((int)timeoutManager->wheel.size() - 1);

			// ��Ϊ����ͷ
			timeoutPrev = nullptr;
			timeoutNext = timeoutManager->wheel[timeoutIndex];
			timeoutManager->wheel[timeoutIndex] = this;

			// ��֮ǰ������ͷ������( ����еĻ� )
			if (timeoutNext) {
				timeoutNext->timeoutPrev = this;
			}
		}
		else {
			// ���õ���ʼ״̬
			timeoutIndex = -1;
			timeoutPrev = nullptr;
			timeoutNext = nullptr;
		}

		return 0;
	}
}
