#include <xx_epoll.h>
#include <xx_timeouter.h>

// todo: FD 走 shared_ptr, 以便能适配 listener, dialer, udp

enum class FDTypes {
	Unknown,
	Listener,
	TcpDialer,
	TcpSocket,
	UdpSocket
};

struct FD : std::enable_shared_from_this<FD> {
	virtual ~FD() {/* 最外层派生类析构要写 this->Dispose(-1); */ }

	FDTypes type = FDTypes::Unknown;
	int fd = -1;

	virtual bool Disposed() const noexcept {
		return type == FDTypes::Unknown;
	}

	// flag == -1: call by destructor.  0 : do not callback.  1: callback
	// return true: dispose success. false: already disposed.
	virtual bool Dispose(int const& flag = 1) noexcept {
		if (Disposed()) return false;
		type = FDTypes::Unknown;
		if (fd != -1) {
			close(fd);
			fd = -1;
		}
		return true;
	}
};

struct Listener : FD {
	~Listener() {
		this->Dispose(-1);
	}
};

struct TcpDialer : FD {
	~TcpDialer() {
		this->Dispose(-1);
	}
};

struct TcpSocket : FD {
	~TcpSocket() {
		this->Dispose(-1);
	}
};

struct UdpSocket : FD {
	~UdpSocket() {
		this->Dispose(-1);
	}
};

struct Epoll {
	int efd = -1;
	std::array<epoll_event, 4096> events;

	Epoll() {
		efd = epoll_create1(0);
		if (-1 == efd) throw - 1;
	}

	// 进入一次 epoll wait. 可传入超时时间. 
	inline int Wait(int const& timeoutMS) {
		int n = epoll_wait(efd, events.data(), events.size(), timeoutMS);
		if (n == -1) return errno;

		for (int i = 0; i < n; ++i) {
			// get fd
			auto fd = events[i].data.fd;
			auto ev = events[i].events;
			//xx::CoutN(fd, " ", ev);

			// error
			if (ev & EPOLLERR || ev & EPOLLHUP) {
				//peers[fd].Dispose();
				continue;
			}

			// accept
			//else {
			//	// 遍历所有 listen fd, 逐个判断是不是
			//	int idx = 0;
			//LabListenCheck:
			//	if (fd == listenFDs[idx]) {

			//		// todo: 多线程环境下，如果 accept 超出数量限制, 或者负载极不均匀, 是否可以挪移到别的 epoll?

			//		// 一直 accept 到没有为止
			//	LabAccept:
			//		int sockFD = Accept(fd);
			//		if (sockFD > 0) {
			//			auto&& peer = peers[sockFD];
			//			peer.Init(this, sockFD, fd);
			//			OnAccept(Peer_r(peer), (int)idx);
			//			goto LabAccept;
			//		}
			//		continue;
			//	}
			//	else if (++idx < listenFDsCount) goto LabListenCheck;
			//}

			//// action
			//if (fd == actionsPipes[0]) {
			//	assert(ev & EPOLLIN);
			//	if (HandleActions()) return -1;
			//	else continue;
			//}

			//// 已连接的 socket
			//Peer& peer = peers[fd];
			//if (peer.listenFD != -1) {
			//	// read
			//	if (ev & EPOLLIN) {
			//		if (Read(fd)) {
			//			peer.Dispose();
			//			continue;
			//		}
			//		if (OnReceive(Peer_r(peer))) {
			//			peer.Dispose();
			//			continue;
			//		}
			//		if (peer.Disposed()) continue;
			//	}
			//	// write
			//	assert(!peer.Disposed());
			//	if (ev & EPOLLOUT) {
			//		// 设置为可写状态
			//		peer.writing = false;
			//		if (Write(fd)) {
			//			peer.Dispose();
			//		}
			//	}
			//}
			//// 正在连接的 socket
			//else {
			//	// 读取错误 或者读到错误 都认为是连接失败. 当前策略是无脑 Dispose. 会产生回调. Dial 发起方需要记录 Dial 返回值, 如果这个回调的 listenFD == -1
			//	int err;
			//	socklen_t result_len = sizeof(err);
			//	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &result_len) == -1 || err) {
			//		OnAccept(Peer_r(peer), -1);
			//		peer.Dispose(false);
			//		continue;
			//	}
			//	// 拨号连接成功. 打标记. 避免再次进入该 if 分支
			//	peer.listenFD = -2;
			//	peer.SetTimeout(0);
			//	OnAccept(Peer_r(peer), -1);
			//}
		}
		return 0;
	}


	// 开始运行并尽量维持在指定帧率. 临时拖慢将补帧
	inline int Run(double const& frameRate = 60.3) {
		assert(frameRate > 0);

		// 稳定帧回调用的时间池
		double ticksPool = 0;

		// 本次要 Wait 的超时时长
		int waitMS = 0;

		// 计算帧时间间隔
		auto ticksPerFrame = 10000000.0 / frameRate;

		// 取当前时间
		auto lastTicks = xx::NowEpoch10m();

		// 开始循环
		while (true) {

			// 计算上个循环到现在经历的时长, 并累加到 pool
			auto currTicks = xx::NowEpoch10m();
			auto elapsedTicks = (double)(currTicks - lastTicks);
			ticksPool += elapsedTicks;
			lastTicks = currTicks;

			// 如果累计时长跨度大于一帧的时长 则 Update
			if (ticksPool > ticksPerFrame) {

				// 消耗累计时长
				ticksPool -= ticksPerFrame;

				// 本次 Wait 不等待.
				waitMS = 0;

				// 处理各种连接超时事件
				//HandleTimeout();

				// 帧逻辑调用一次
				//if (int r = Update()) return r;
			}
			else {
				// 计算等待时长
				waitMS = (int)((ticksPerFrame - elapsedTicks) / 10000.0);
			}

			// 调用一次 epoll wait. 
			if (int r = Wait(waitMS)) return r;
		}

		return 0;
	}
};

struct Foo : xx::TimeouterBase {
	virtual void OnTimeout() override {
		xx::Cout("x");
	}
};

int main() {
	xx::TimeouterManager tm(8);

	auto foo = xx::Make<Foo>();
	foo->timerManager = &tm;
	foo->SetTimeout(5);

	auto foo2 = xx::Make<Foo>();
	foo2->timerManager = &tm;
	foo2->SetTimeout(5);

	for (int i = 0; i < 32; ++i) {
		tm.Update();
		xx::Cout(".");
	}
	xx::CoutN("end");
	return 0;
}
