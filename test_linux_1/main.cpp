#include "xx_epoll.h"

namespace xx {

	// 协程恢复执行的条件. 可位与操作多条件触发
	enum class ResumeConditions : int {
		Immediately = 0,
		Frame = 1,
		Timeout = 2,
		Network = 4
	};

	struct ResumeManager;
	struct Resumer {
		friend struct ResumeManager;
	protected:
		// 指向管理器
		ResumeManager& mgr;

		// 指向自己位于 cs 父容器的下标
		std::size_t csIndex = -1;

		// 指向自己位于 csFrame 条件父容器的下标
		std::size_t csFrameIndex = -1;

		// 指向自己位于 csTimeout 条件父容器的下标
		int timeoutIndex = -1;				// 位于 timeoutWheel 的下标. 如果该 peer 为 head 则该值非 -1 则可借助该值定位到 wheel 中的位置
		Resumer* timeoutPrev = nullptr;		// 位于相同刻度时间队列中时, prev + next 组成双向链表
		Resumer* timeoutNext = nullptr;		// 位于相同刻度时间队列中时, prev + next 组成双向链表

		// 备份 fd 条件参数
		int networkFD = -1;

		virtual int Update(ResumeConditions const& resumeReason) = 0;
		virtual ~Resumer();
	public:

		// 存放 Update 进入后要跳转到的行号。 lineNumber = Update( .... )
		int lineNumber = 0;

		// 传入 false 取消
		void ResumeByFrame(bool const& enable = true);

		// 返回 0 成功. 失败可能: 参数越界. 传入 0 取消
		int ResumeByTimeout(int64_t const& interval);

		// 传入 -1, 0 取消
		void ResumeByNetworkTimeout(int const& fd, int64_t const& interval);

		Resumer(ResumeManager& mgr) : mgr(mgr) {}
		Resumer(Resumer&&) = delete;
		Resumer& operator=(Resumer&&) = delete;
		Resumer(Resumer&) = delete;
		Resumer& operator=(Resumer&) = delete;
	};

	struct ResumeManager {
		// 主容器
		std::vector<Resumer*> cs;

		// 条件容器: 每帧回调
		std::vector<Resumer*> csFrame;

		// 超时时间轮长度。需要 2^n 对齐以实现快速取余. 按照典型的 60 帧逻辑，60 秒需要 3600
		static const int timeoutWheelLen = 1 << 12;

		// 条件容器: 时间轮. 填入参与 timeout 检测的 对象指针 的下标( 链表头 )
		std::array<Resumer*, timeoutWheelLen> timeoutWheel;

		// 时间轮游标. 指向当前链表. 每帧 +1, 按 timeoutWheelLen 长度取余, 循环使用
		int timeoutWheelCursor = 0;

		// 条件容器: 网络事件. fd 为 key
		std::unordered_map<int, Resumer*> csNet;

		// 添加一个 job
		inline void Add(Resumer* const& job, bool const& runImmediately = false) {
			assert(job);

			// 放入主容器
			job->csIndex = cs.size();
			cs.push_back(job);

			// 如果需要的话, 立刻执行一次. 如果已退出，则不必放入容器
			if (runImmediately) {
				Run(job, ResumeConditions::Immediately);
			}
		}

		inline int Run(Resumer* const& job, ResumeConditions const& reason) {
			assert(job);
			job->lineNumber = job->Update(reason);
			if (!job->lineNumber) {
				delete job;
				return 1;
			}
			return 0;
		}

		// 每帧调用一次。触发 frame & timeout 处理
		inline void HandleEvent_Frame() {
			// 循环递增游标
			timeoutWheelCursor = (timeoutWheelCursor + 1) & (timeoutWheelLen - 1);

			// 遍历超时对象
			auto c = timeoutWheel[timeoutWheelCursor];			// clone
			while (c) {
				// 自动取消恢复条件
				c->ResumeByNetworkTimeout(-1, 0);
				auto nc = c->timeoutNext;						// clone
				Run(c, ResumeConditions::Timeout);
				c = nc;
			};

			// 遍历每帧触发的对象
			for (int i = (int)csFrame.size() - 1; i >= 0; --i) {
				if (Run(csFrame[i], ResumeConditions::Timeout)) continue;
			}
		}

		inline void HandleEvent_Network(int const& fd) {
			// todo
		}

		~ResumeManager() {
			for (int i = (int)cs.size() - 1; i >= 0; --i) {
				delete cs[i];
			}
		}
	};

	inline Resumer::~Resumer() {
		ResumeByFrame(false);
		ResumeByNetworkTimeout(-1, 0);
		XX_SWAP_REMOVE(this, csIndex, mgr.cs);
	}

	inline void Resumer::ResumeByFrame(bool const& enable) {
		if (enable) {
			if (csFrameIndex == -1) {
				csFrameIndex = mgr.csFrame.size();
				mgr.csFrame.emplace_back(this);
			}
		}
		else if (csFrameIndex != -1) {
			XX_SWAP_REMOVE(this, csFrameIndex, mgr.csFrame);
		}
	}

	inline int Resumer::ResumeByTimeout(int64_t const& interval) {
		// 试着从 wheel 链表中移除
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				mgr.timeoutWheel[timeoutIndex] = timeoutNext;
			}
		}

		// 检查是否传入间隔时间
		if (interval) {
			// 如果设置了新的超时时间, 则放入相应的链表
			// 安全检查
			if (interval < 0 || interval > mgr.timeoutWheelLen) return -1;

			// 环形定位到 wheel 元素目标链表下标
			timeoutIndex = (interval + mgr.timeoutWheelCursor) & (mgr.timeoutWheelLen - 1);

			// 成为链表头
			timeoutPrev = nullptr;
			timeoutNext = mgr.timeoutWheel[timeoutIndex];
			mgr.timeoutWheel[timeoutIndex] = this;

			// 和之前的链表头连起来( 如果有的话 )
			if (timeoutNext) {
				timeoutNext->timeoutPrev = this;
			}
		}
		else {
			// 重置到初始状态
			timeoutPrev = nullptr;
			timeoutNext = nullptr;
			timeoutIndex = -1;
		}
		return 0;
	}

	inline void Resumer::ResumeByNetworkTimeout(int const& fd, int64_t const& interval) {
		if (networkFD != fd) {
			if (networkFD != -1) {
				mgr.csNet.erase(networkFD);
			}
			networkFD = fd;
		}
		if (fd != -1) {
			mgr.csNet.emplace(fd, this);
		}
		ResumeByTimeout(interval);
	}
}

int main() {
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
//			c = c.resume();
//		}
//		return std::move(c);
//		});
//	while (c) {
//		xx::CoutN("."); c = c.resume();
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
