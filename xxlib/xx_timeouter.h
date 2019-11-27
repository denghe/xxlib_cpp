#pragma once
#include <vector>

namespace xx {
	struct TimeouterManager;

	// ��Ҫ��ʱ����Ļ���
	struct TimeouterBase {
		virtual ~TimeouterBase() {};

		TimeouterManager* timerManager = nullptr;
		int timeoutIndex = -1;
		TimeouterBase* timeoutPrev = nullptr;
		TimeouterBase* timeoutNext = nullptr;

		// ���ó�ʱʱ��. ʱ�䵽������ OnTimeout ��������. ���� 0 ����. Ҫ�� interval < wheel size
		int SetTimeout(int const& interval);

		virtual void OnTimeout() = 0;
	};

	// ����ʱ���ֵĳ�ʱ������
	struct TimeouterManager {
		// ֻ��ָ������, �������ڴ�
		std::vector<TimeouterBase*> wheel;
		int cursor = 0;

		// ���� 2^n �����ӳ���
		TimeouterManager(std::size_t const& maxLen = 1 << 12) {
			wheel.resize(maxLen);
		}

		// ÿ֡����һ��
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
		// δ��ʱ��������
		if (!timerManager) return -1;

		// ���Ŵ� wheel �������Ƴ�
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

		// ����Ƿ�����ʱ��
		if (interval) {
			// ����������µĳ�ʱʱ��, �������Ӧ������
			// ��ȫ���
			if (interval < 0 || interval >(int)timerManager->wheel.size()) return -1;

			// ���ζ�λ�� wheel Ԫ��Ŀ�������±�
			timeoutIndex = (interval + timerManager->cursor) & ((int)timerManager->wheel.size() - 1);

			// ��Ϊ����ͷ
			timeoutPrev = nullptr;
			timeoutNext = timerManager->wheel[timeoutIndex];
			timerManager->wheel[timeoutIndex] = this;

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
