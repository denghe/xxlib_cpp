#include "xx_epoll.h"

namespace xx {

	// Update 进入条件. 同时也表明了所在容器. 条件互斥.
	enum class UpdateConditions {
		None,
		Frame,
		Timeout,
		Network,
		Unknown
	};


	struct StateManager;

	// 带条件执行 Update 函数的封装
	struct State {
		State(StateManager& sm) : sm(sm) {}
		State(State&&) = delete;
		State& operator=(State&&) = delete;
		State(State&) = delete;
		State& operator=(State&) = delete;


		friend struct StateManager;
	protected:
		// 指向管理器
		StateManager& sm;

		// 当前 update 进入条件
		UpdateConditions uc;

		// 指向自己位于 主 父容器的下标
		int index = -1;

		// todo: union ?

		// 存放 Update 进入后要跳转到的行号。 lineNumber = Update( .... )
		int lineNumber = 0;

		// 指向自己位于 immediately 条件父容器的下标
		int noneIndex = -1;

		// 指向自己位于 frame 条件父容器的下标
		int frameIndex = -1;

		// 指向自己位于 network 条件父容器的 key. 同时也是 fd
		int networkFD = -1;

		// 指向自己位于 timeout 条件父容器的下标
		int timeoutIndex = -1;

		// 位于相同刻度时间队列中时, prev + next 组成双向链表
		State* timeoutPrev = nullptr;

		// 位于相同刻度时间队列中时, prev + next 组成双向链表
		State* timeoutNext = nullptr;

		// 指向上层调用栈. 会导致 uc 变为 Unknown
		State* callPrev = nullptr;

		// 由 Manager 视条件调用
		virtual int Update() = 0;

		virtual ~State();

		// 设置 Update 进入条件为 无条件. 立刻执行
		void SetCond_None();

		// 设置 Update 进入条件为 每帧
		void SetCond_Frame();

		// 设置 Update 进入条件为 超时
		void SetCond_Timeout(int64_t const& timeoutInterval);

		// 设置 Update 进入条件为 网络事件 或 超时
		void SetCond_Network(int const& fd, int64_t const& timeoutInterval);


		// 模拟调用子函数. 清理当前 Cond 并将 this 存入 创建的新实例的 callPrev.
		template<typename T, typename ...Args>
		void Call(Args&&...args);

		// 专用于清除超时条件
		void ClearCond_Timeout(bool const& cleanup = false);
	};


	// "带条件执行 Update 函数的封装" 的管理器, 调度器
	struct StateManager {
		// 主容器
		std::vector<State*> states;

		// 无条件执行容器
		std::vector<State*> noneStates;

		// 条件容器: 每帧回调
		std::vector<State*> frameStates;

		// 条件容器: 网络事件. fd 为 key
		std::unordered_map<int, State*> networkStates;

		// 超时时间轮长度。需要 2^n 对齐以实现快速取余. 按照典型的 60 帧逻辑，60 秒需要 3600
		static const int timeoutWheelLen = 1 << 12;

		// 条件容器: 时间轮. 填入参与 timeout 检测的 对象指针 的下标( 链表头 )
		std::array<State*, timeoutWheelLen> timeoutWheel;

		// 时间轮游标. 指向当前链表. 每帧 +1, 按 timeoutWheelLen 长度取余, 循环使用
		int timeoutWheelCursor = 0;


		// 死亡标记. 析构时设置为 true 以便 states 快速删除
		bool disposed = false;

		inline bool Empty() {
			return states.empty();
		}

		// 创建一个条件 Update 类	// todo: pool
		template<typename T, typename ...Args>
		inline T* Create(Args&&...args) {
			auto u = new T(*this, std::forward<Args>(args)...);
			u->index = (int)states.size();
			states.push_back(u);
			u->noneIndex = (int)noneStates.size();
			noneStates.push_back(u);
			return u;
		}

		StateManager() {
			timeoutWheel.fill(0);
		}

		~StateManager() {
			disposed = true;
			for (int i = (int)states.size() - 1; i >= 0; --i) {
				delete states[i];
			}
		}

		inline void UpdateOnce() {
			UpdateNone();
			UpdateTimeout();
			UpdateNone();
			UpdateFrame();
			UpdateNone();
			// UpdateNetwork??
			// UpdateNone()
		}

		friend struct State;
	protected:
		inline int Call(State* const& u) {
			assert(u);
			u->lineNumber = u->Update();
			if (!u->lineNumber) {
				delete u;
				return 1;
			}
			return 0;
		}

		inline void ClearCond(State* const& u) {
			switch (u->uc) {
			case UpdateConditions::None:
				XX_SWAP_REMOVE(u, noneIndex, noneStates);
				u->noneIndex = -1;
				break;
			case UpdateConditions::Frame:
				XX_SWAP_REMOVE(u, frameIndex, frameStates);
				u->frameIndex = -1;
				break;
			case UpdateConditions::Timeout:
				u->ClearCond_Timeout(true);
				break;
			case UpdateConditions::Network:
				// todo
				break;
			case UpdateConditions::Unknown:
				break;
			}
		}

		// 执行无条件 states 直到队列被清空
		inline void UpdateNone() {
			while (noneStates.size()) {
				for (int i = (int)noneStates.size() - 1; i >= 0; --i) {
					if (Call(noneStates[i])) continue;
				}
			}
		}

		// 每帧调用一次。触发 timeout 处理
		inline void UpdateTimeout() {
			// 循环递增游标
			timeoutWheelCursor = (timeoutWheelCursor + 1) & (timeoutWheelLen - 1);

			// 遍历超时对象
			auto c = timeoutWheel[timeoutWheelCursor];
			while (c) {
				// 自动取消恢复条件
				c->ClearCond_Timeout(true);
				auto nc = c->timeoutNext;
				Call(c);
				c = nc;
			};
		}

		// 每帧调用一次。触发 frame 处理
		inline void UpdateFrame() {
			for (int i = (int)frameStates.size() - 1; i >= 0; --i) {
				if (Call(frameStates[i])) continue;
			}
		}

		// 网络事件发生时调用
		inline void UpdateNetwork(int const& fd) {
			// todo
		}

	};


	inline State::~State() {
		if (sm.disposed) return;
		sm.ClearCond(this);
		XX_SWAP_REMOVE(this, index, sm.states);
		index = -1;
		if (callPrev) {
			// 模拟还原堆栈. 将 prev 放到 none 队列确保接下来接着执行
			assert(callPrev->uc == UpdateConditions::Unknown);
			callPrev->uc = UpdateConditions::Frame;
			callPrev->frameIndex = (int)sm.frameStates.size();
			sm.frameStates.push_back(callPrev);
			callPrev = nullptr;
		}
	}


	inline void State::SetCond_None() {
		if (uc == UpdateConditions::None) return;
		sm.ClearCond(this);
		uc = UpdateConditions::None;
		noneIndex = (int)sm.noneStates.size();
		sm.noneStates.push_back(this);
	}

	inline void State::SetCond_Frame() {
		if (uc == UpdateConditions::Frame) return;
		sm.ClearCond(this);
		uc = UpdateConditions::Frame;
		frameIndex = (int)sm.frameStates.size();
		sm.frameStates.push_back(this);
	}

	inline void State::SetCond_Timeout(int64_t const& timeoutInterval) {
		assert(timeoutInterval > 0 && timeoutInterval < sm.timeoutWheelLen);
		sm.ClearCond(this);
		uc = UpdateConditions::Timeout;

		// 试着从 wheel 链表中移除
		ClearCond_Timeout();

		// 放入相应的链表
		// 环形定位到 wheel 元素目标链表下标
		timeoutIndex = (timeoutInterval + sm.timeoutWheelCursor) & (sm.timeoutWheelLen - 1);

		// 成为链表头
		timeoutPrev = nullptr;
		timeoutNext = sm.timeoutWheel[timeoutIndex];
		sm.timeoutWheel[timeoutIndex] = this;

		// 和之前的链表头连起来( 如果有的话 )
		if (timeoutNext) {
			timeoutNext->timeoutPrev = this;
		}
	}

	inline void State::ClearCond_Timeout(bool const& cleanup) {
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				sm.timeoutWheel[timeoutIndex] = timeoutNext;
			}
		}
		if (cleanup) {
			timeoutPrev = nullptr;
			timeoutNext = nullptr;
			timeoutIndex = -1;
		}
	}

	inline void State::SetCond_Network(int const& fd, int64_t const& interval) {
		//if (networkFD != fd) {
		//	if (networkFD != -1) {
		//		sm.csNet.erase(networkFD);
		//	}
		//	networkFD = fd;
		//}
		//if (fd != -1) {
		//	sm.csNet.emplace(fd, this);
		//}
		//SetCond_Timeout(interval);
	}

	template<typename T, typename ...Args>
	inline void State::Call(Args&&...args) {
		sm.ClearCond(this);
		uc = UpdateConditions::Unknown;
		auto u = sm.Create<T>(std::forward<Args>(args)...);
		u->callPrev = this;
	}
}



struct Bar : xx::State {
	using BaseType = xx::State;
	using BaseType::BaseType;
	int i = 0;

	int Update() override {
		COR_BEGIN;

		for (i = 0; i < 5; i++)
		{
			xx::CoutN("bar ", i);

			SetCond_Timeout(2);
			COR_YIELD;
		}

		xx::CoutN("bar end");

		COR_END;
	}
};

struct Foo : xx::State {
	using BaseType = xx::State;
	using BaseType::BaseType;
	int i = 0;
	int Update() override {
		COR_BEGIN;

		for (i = 0; i < 3; i++)
		{
			xx::CoutN("foo ", i);

			SetCond_Timeout(2);
			COR_YIELD;
		}

		Call<Bar>();
		COR_YIELD;

		for (i = 0; i < 3; i++)
		{
			xx::CoutN("foo ", i);

			SetCond_Timeout(2);
			COR_YIELD;
		}

		xx::CoutN("foo end");
		COR_END;
	}
};

int main() {
	xx::StateManager rm;
	rm.Create<Foo>();
	while (!rm.Empty()) {
		rm.UpdateOnce();
		Sleep(1000);
	}
	return 0;
}





































/*

协程如果只有 yield() 可用，是比较粗糙的。会在每帧无脑被唤醒，进而执行自己的状态检查，虽然也能完成需求，但是会空耗 cpu, 且处理及时度降低很多.

考虑参考 golang 的 select 部分, yield 变为条件式.

协程事件类型分类：
1. 网络: 连，断，收 ( 理论上讲还可以有个 发送队列已清空 )
2. 帧步进

// todo: Dispatch

// todo:  udp, 异步域名解析

udp 单个 端口 大负载 面临的问题:
	从系统态 到 内存态 的瓶颈, pps 提升不上去
	很难并行处理( 包乱序问题 )

解决方案:
	创建多 fd 多 port, 绕开单个 fd 的流量
	一个 port 做 listener, 收到 client 握手信息之后返回 id + 其他 port 值, 之后由该 port & fd 负责处理该 client 的数据收发

需要做一个 port/fd 连接管理器模拟握手
理论上讲收到的数据可以丢线程池. 按 id 来限定处理线程 确保单个逻辑连接在单个线程中处理
udp 没有连接 / 断开 的说法，都要靠自己模拟, fd 也不容易失效.

*/

//#include "xx_epoll.h"

//
//struct Client : xx::Epoll::Instance {
//	using BaseType = Instance;
//
//
//	Client() {
//		for (int i = 0; i < 100; ++i) {
//			coros.Add([this, i](xx::Coro2& yield) { this->Logic(yield, i); });
//		}
//	}
//
//	inline void Logic(xx::Coro2& yield, int const& i) {
//		// 准备拨号
//	LabDial:
//
//		// 防御性 yield 一次避免 goto 造成的死循环
//		yield();
//
//		// 拨号到服务器
//		auto&& pr = Dial("192.168.1.160", 12345, 5);
//
//		// 如果拨号立刻出错, 重拨
//		if (!pr) goto LabDial;
//
//		// 等待 连接结果: 失败|超时( pr 失效 ), 成功( Connected() 返回 true )
//	LabWait:
//		// 连接成功: 继续后面的流程
//		if (pr->Connected()) goto LabSend;
//
//		// 等待一帧
//		yield();
//
//		// pr 如果还健在就继续循环
//		if (pr) goto LabWait;
//
//		// 失败|超时: 重拨
//		goto LabDial;
//
//	LabSend:
//		xx::Cout(pr->id, " ");
//
//		// 直接用底层函数发包。失败: 重拨
//		if (write(pr->sockFD, "a", 1) <= 0) {
//			pr->Dispose();
//			goto LabDial;
//		}
//
//		// 等待断线
//		while (pr) {
//			yield();
//		}
//		goto LabDial;
//	}
//
//	inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
//		assert(listenIndex < 0);
//		xx::CoutN(threadId, " OnAccept: id = ", pr->id, ", fd = ", pr->sockFD);
//	}
//
//	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
//		xx::CoutN(threadId, " OnDisconnect: id = ", pr->id);
//	}
//
//	uint64_t count = 0;
//	uint64_t last = xx::NowSteadyEpochMS();
//	virtual int OnReceive(xx::Epoll::Peer_r pr) override {
//		++count;
//		if ((count & 0x1FFFF) == 0x1FFFF) {
//			auto now = xx::NowSteadyEpochMS();
//			xx::CoutN(double(0x20000 * 1000) / double(now - last));
//			last = now;
//		}
//		return this->BaseType::OnReceive(pr);
//	}
//};
//
//int main(int argc, char* argv[]) {
//	xx::IgnoreSignal();
//	return std::make_unique<Client>()->Run(1);
//}





















//#include "xx_epoll.h"
//
//struct Client : xx::Epoll::Instance {
//	using BaseType = Instance;
//
//
//	Client() {
//		for (int i = 0; i < 100; ++i) {
//			coros.Add([this, i](xx::Coro2& yield) { this->Logic(yield, i); });
//		}
//	}
//
//	inline void Logic(xx::Coro2& yield, int const& i) {
//		// 准备拨号
//	LabDial:
//
//		// 防御性 yield 一次避免 goto 造成的死循环
//		yield();
//
//		// 拨号到服务器
//		auto&& pr = Dial("192.168.1.160", 12345, 5);
//
//		// 如果拨号立刻出错, 重拨
//		if (!pr) goto LabDial;
//
//		// 等待 连接结果: 失败|超时( pr 失效 ), 成功( Connected() 返回 true )
//	LabWait:
//		// 连接成功: 继续后面的流程
//		if (pr->Connected()) goto LabSend;
//
//		// 等待一帧
//		yield();
//
//		// pr 如果还健在就继续循环
//		if (pr) goto LabWait;
//
//		// 失败|超时: 重拨
//		goto LabDial;
//
//	LabSend:
//		xx::Cout(pr->id, " ");
//
//		// 直接用底层函数发包。失败: 重拨
//		if (write(pr->sockFD, "a", 1) <= 0) {
//			pr->Dispose();
//			goto LabDial;
//		}
//
//		// 等待断线
//		while (pr) {
//			yield();
//		}
//		goto LabDial;
//	}
//
//	inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
//		assert(listenIndex < 0);
//		xx::CoutN(threadId, " OnAccept: id = ", pr->id, ", fd = ", pr->sockFD);
//	}
//
//	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
//		xx::CoutN(threadId, " OnDisconnect: id = ", pr->id);
//	}
//
//	uint64_t count = 0;
//	uint64_t last = xx::NowSteadyEpochMS();
//	virtual int OnReceive(xx::Epoll::Peer_r pr) override {
//		++count;
//		if ((count & 0x1FFFF) == 0x1FFFF) {
//			auto now = xx::NowSteadyEpochMS();
//			xx::CoutN(double(0x20000 * 1000) / double(now - last));
//			last = now;
//		}
//		return this->BaseType::OnReceive(pr);
//	}
//};
//
//int main(int argc, char* argv[]) {
//	xx::IgnoreSignal();
//	return std::make_unique<Client>()->Run(1);
//}













//// 模拟业务逻辑
////Sleep((double)rand() / RAND_MAX * 50);


//struct sigaction sa;
//sa.sa_handler = SIG_IGN;//设定接受到指定信号后的动作为忽略
//sa.sa_flags = 0;
//if (sigemptyset(&sa.sa_mask) == -1 || //初始化信号集为空
//	sigaction(SIGPIPE, &sa, 0) == -1) { //屏蔽SIGPIPE信号
//	perror("failed to ignore SIGPIPE; sigaction");
//	exit(EXIT_FAILURE);
//}

//signal(SIGPIPE, SIG_IGN);

//sigset_t signal_mask;
//sigemptyset(&signal_mask);
//sigaddset(&signal_mask, SIGPIPE);
//int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
//if (rc != 0) {
//	printf("block sigpipe error\n");
//}

//rc = sigprocmask(SIG_BLOCK, &signal_mask, NULL);
//if (rc != 0) {
//	printf("block sigpipe error\n");
//}



//uint64_t counter = 0;
//if (r) {
//	xx::Cout(r);
//}
//xx::Cout(".");
//++counter;
//if (counter % 100 == 0) {
//	xx::CoutN();
//}

//std::vector<EpollMessage> msgs;
					//auto&& m = msgs.emplace_back();
				//m.fd = events[i].data.fd;
				//m.type = EpollMessageTypes::Accept;
					//auto&& m = msgs.emplace_back();
				//m.fd = events[i].data.fd;
				//m.type = EpollMessageTypes::Disconnect;


//enum class EpollMessageTypes {
//	Unknown,
//	Accept,
//	Disconnect,
//	Read
//};

//struct EpollMessage {
//	EpollMessageTypes type = EpollMessageTypes::Unknown;
//	int fd = 0;
//	EpollBuf buf;

//	EpollMessage() = default;
//	EpollMessage(EpollMessage const&) = delete;
//	EpollMessage& operator=(EpollMessage const&) = delete;
//	EpollMessage(EpollMessage&&) = default;
//	EpollMessage& operator=(EpollMessage&&) = default;
//};

//int main() {
//	xx::Coro2s cs;
//	cs.Add([&](xx::Coro2& yield) {
//		for (size_t i = 0; i < 10; i++) {
//			xx::CoutN(i);
//			yield();
//		}
//		});
//	cs.Run();
//	xx::CoutN("end");
//	std::cin.get();
//	return 0;
//}

//
//int main() {
//	auto&& c = boost::context::callcc([](boost::context::continuation&& c) {
//	    for (size_t i = 0; i < 10; i++) {
//			xx::CoutN(i);
//			c = c.update();
//		}
//		return std::move(c);
//		});
//	while (c) {
//		xx::CoutN("."); c = c.update();
//	}
//	std::cin.get();
//	return 0;
//}




//#include "xx_epoll_context.h"
//
//int main() {
//
//	// echo server sample
//	xx::EpollListen(12345, xx::SockTypes::TCP, 4, [](int fd, auto read, auto write) {
//			printf("peer accepted. fd = %i\n", fd);
//			char buf[1024];
//			while (size_t received = read(buf, sizeof(buf))) {
//				if (write(buf, received)) break;
//			}
//			printf("peer disconnected: fd = %i\n", fd);
//		}
//	);
//	return 0;
//}
