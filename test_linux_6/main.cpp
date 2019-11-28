#include <xx_epoll.h>
#include <xx_timeout.h>

// todo: 加一波 noexcept, const 啥的

// 内存模型为 智能指针 从创建出来开始就放入 容器, 并非弱引用, 故不需要想办法加持, 也能提升网络事件派发效率( 省掉了 lock() )

struct Epoll;

struct EpollItem : std::enable_shared_from_this<EpollItem> {
	// 指向总容器
	Epoll* ep = nullptr;

	// 销毁标志
	bool disposed = false;
	bool Disposed() const noexcept {
		return disposed;
	}

	// flag == -1: 在析构函数中调用.  0: 不产生回调  1: 产生回调
	// 下列代码为内容示例, 方便复制小改
	virtual void Dispose(int const& flag = 1) noexcept {
		// 检查 & 打标记 以避免重复执行析构逻辑
		if (disposed) return;
		disposed = true;

		// 调用派生类自己的析构部分
		Disposing(flag);

		// todo: 从容器移除以触发析构
	};

	// 子析构( 从析构函数中间接调用时无效 )
	virtual void Disposing(int const& flag) noexcept {}

	// 每层非虚派生类析构都要写 this->Dispose(-1);
	virtual ~EpollItem() { this->Dispose(-1); }
};


struct FDHandler : EpollItem {
	// linux 系统文件描述符. 同时也是 ep->fdHandlers 的下标
	int fd = -1;

	// epoll fd 事件处理. return 非 0 表示自杀
	virtual int OnEpollEvent(uint32_t const& e) = 0;

	// 关 fd, 从 epoll 移除, call Disposing, 从容器移除( 可能触发析构 )
	virtual void Dispose(int const& flag = 1) noexcept override;

	virtual ~FDHandler() { this->Dispose(-1); }
};


struct Timer : EpollItem, xx::TimeoutBase {
	// ep->timers 的下标
	int indexAtContainer = -1;

	// 时间到达时触发. 如果想实现 repeat 效果, 就在函数返回前 自己 timer->SetTimeout
	std::function<void(Timer* const& timer)> onFire;

	// 负责触发 onFire
	virtual void OnTimeout() noexcept override;

	// 从 timeoutManager 移除, call Disposing, 从容器移除( 可能触发析构 )
	virtual void Dispose(int const& flag = 1) noexcept override;

	~Timer() { this->Dispose(-1); }
};


struct TcpListener;
struct TcpPeer : FDHandler, xx::TimeoutBase {
	// ip
	std::string ip;

	// 收数据用堆积容器
	xx::List<uint8_t> recv;

	// 是否正在发送( 是：不管 sendQueue 空不空，都不能 write, 只能塞 sendQueue )
	bool writing = false;

	// 待发送队列
	std::optional<xx::BufQueue> sendQueue;

	// 每 fd 每一次可写, 写入的长度限制( 希望能实现当大量数据下发时各个 socket 公平的占用带宽 )
	std::size_t sendLenPerFrame = 65536;

	// 读缓冲区内存扩容增量
	std::size_t readBufLen = 65536;

	// writev 函数 (buf + len) 数组 参数允许的最大数组长度
	static const std::size_t maxNumIovecs = 1024;

	virtual int OnEpollEvent(uint32_t const& e) override;

	// 数据接收事件: 从 recv 拿数据
	virtual int OnReceive();

	// 用于处理超时掐线
	virtual void OnTimeout() override;

	// 断线时的处理
	inline virtual void OnDisconnect() {}

	// 触发 OnDisconnect
	virtual void Disposing(int const& flag) noexcept override;

	~TcpPeer() { this->Dispose(-1); }

	int Send(xx::Buf&& eb);
	int Flush();

protected:
	int Write();
	int Read();
};


struct TcpListener : FDHandler {
	// 覆盖并提供创建 peer 对象的实现. 返回 nullptr 表示创建失败
	virtual std::shared_ptr<TcpPeer> OnCreatePeer();

	// 覆盖并提供为 peer 绑定事件的实现. 返回非 0 表示终止 accept
	virtual int OnAccept(std::shared_ptr<TcpPeer>& peer);

	// 调用 accept
	virtual int OnEpollEvent(uint32_t const& e) override;

	~TcpListener() { this->Dispose(-1); }

protected:
	// return fd. <0: error. 0: empty (EAGAIN / EWOULDBLOCK), > 0: fd
	int Accept(int const& listenFD);
};


struct TcpConn : TcpPeer {
	// 是否连接成功
	bool connected = false;

	// 成功, 超时或连接错误 都将触发该函数. 进一步判断 connected 可知状态
	virtual void OnConnect() = 0;

	// 用于处理连接超时
	virtual void OnTimeout();

	// 通过 connected 来路由两套事件逻辑
	virtual int OnEpollEvent(uint32_t const& e) override;

	~TcpConn() { this->Dispose(-1); }
};


struct UdpPeer : FDHandler {
	// todo: 提供 udp 基础收发功能
	~UdpPeer() { this->Dispose(-1); }
};


struct UdpListener : UdpPeer {
	// todo: 自己处理收发模拟握手 模拟 accept( 拿已创建的 KcpPeer 来分配 )
	// 循环使用一组 UdpPeer, 创建逻辑 kcp 连接. 多个 UdpPeer 用于加深 epoll 事件队列深度 避免瓶颈 )
};


struct UdpDialer : UdpPeer {
};


struct KcpPeer : UdpPeer {
};


struct Epoll {
	// fd 处理类 之 唯一持有容器. 别处引用尽量用 weak_ptr
	std::vector<std::shared_ptr<FDHandler>> fdHandlers;

	// timer 唯一持有容器. 别处引用尽量用 weak_ptr
	std::vector<std::shared_ptr<Timer>> timers;

	// epoll_wait 事件存储
	std::array<epoll_event, 4096> events;

	// 存储的 epoll fd
	int efd = -1;

	// 对于一些返回值非 int 的函数, 具体错误码将存放于此
	int lastErrorNumber = 0;

	// 超时管理器
	xx::TimeoutManager timeoutManager;

	// 可初始化 fd 总数
	Epoll(int const& maxNumFD = 65536) {
		// 创建 epoll fd
		efd = epoll_create1(0);
		if (-1 == efd) throw - 1;

		// 初始化 fd 处理器存储空间
		fdHandlers.resize(maxNumFD);
	}

	~Epoll() {
		if (efd != -1) {
			close(efd);
			efd = -1;
		}
		for (auto&& o : fdHandlers) {
			if (o && !o->Disposed()) {
				o->Dispose(0);
			}
		}
		for (auto&& o : timers) {
			if (o && !o->Disposed()) {
				o->Dispose(0);
			}
		}
	}

	// return fd. <0: error
	inline static int MakeListenFD(int const& port) {
		char portStr[20];
		snprintf(portStr, sizeof(portStr), "%d", port);

		addrinfo hints;														// todo: ipv6 support
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_UNSPEC;										// ipv4 / 6
		hints.ai_socktype = SOCK_STREAM;									// SOCK_STREAM / SOCK_DGRAM
		hints.ai_flags = AI_PASSIVE;										// all interfaces

		addrinfo* ai_, * ai;
		if (getaddrinfo(nullptr, portStr, &hints, &ai_)) return -1;

		int fd;
		for (ai = ai_; ai != nullptr; ai = ai->ai_next) {
			fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
			if (fd == -1) continue;

			int enable = 1;
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
				close(fd);
				continue;
			}
			if (!bind(fd, ai->ai_addr, ai->ai_addrlen)) break;				// success

			close(fd);
		}
		freeaddrinfo(ai_);

		return ai ? fd : -2;
	}

	// return !0: error
	inline int Ctl(int const& fd, uint32_t const& flags, int const& op = EPOLL_CTL_ADD) {
		epoll_event event;
		event.data.fd = fd;
		event.events = flags;
		return epoll_ctl(efd, op, fd, &event);
	};

	// 添加监听器
	template<typename H = TcpListener, typename ...Args>
	inline std::shared_ptr<H> TcpListen(int const& port, Args&&... args) {
		static_assert(std::is_base_of_v<TcpListener, H>);
		auto&& fd = MakeListenFD(port);
		if (fd < 0) {
			lastErrorNumber = -1;
			return nullptr;
		}

		xx::ScopeGuard sg([&] { close(fd); });
		if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) {
			lastErrorNumber = -2;
			return nullptr;
		}
		if (-1 == listen(fd, SOMAXCONN)) {
			lastErrorNumber = -3;
			return nullptr;
		}

		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -4;
			return nullptr;
		}

		auto h = xx::Make<H>(std::forward<Args>(args)...);
		h->ep = this;
		h->fd = fd;
		fdHandlers[fd] = h;

		sg.Cancel();
		return h;
	}

	// 创建 连出去的 peer
	template<typename H = TcpConn, typename ...Args>
	std::shared_ptr<H> TcpDial(char const* const& ip, int const& port, int const& timeoutInterval, Args&&... args) {
		static_assert(std::is_base_of_v<TcpConn, H>);
		sockaddr_in dest;							// todo: ipv6 support
		memset(&dest, 0, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons((uint16_t)port);
		if (!inet_pton(AF_INET, ip, &dest.sin_addr.s_addr)) {
			lastErrorNumber = errno;
			return nullptr;
		}

		auto&& fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (fd == -1) {
			lastErrorNumber = errno;
			return nullptr;
		}
		xx::ScopeGuard sg([&] { close(fd); });

		int on = 1;
		if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) {
			lastErrorNumber = -3;
			return nullptr;
		}

		if (connect(fd, (sockaddr*)&dest, sizeof(dest)) == -1) {
			if (errno != EINPROGRESS) {
				lastErrorNumber = errno;
				return nullptr;
			}
		}
		// else : 立刻连接上了

		// 纳入 ep 管理
		if (int r = Ctl(fd, EPOLLIN | EPOLLOUT)) {
			lastErrorNumber = r;
			return nullptr;
		}
		auto conn = xx::Make<H>(std::forward<Args>(args)...);
		conn->ep = this;
		conn->timeoutManager = &timeoutManager;
		conn->fd = fd;
		fdHandlers[fd] = conn;

		// 设置拨号超时
		if (int r = conn->SetTimeout(timeoutInterval)) {
			lastErrorNumber = r;
			return nullptr;
		}

		sg.Cancel();
		return conn;
	}

	// 创建 timer
	template<typename T = Timer, typename ...Args>
	std::shared_ptr<T> Delay(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args) {
		static_assert(std::is_base_of_v<Timer, T>);
		if (!cb) return nullptr;
		auto timer = xx::Make<T>(std::forward<Args>(args)...);
		timer->timeoutManager = &timeoutManager;
		if (timer->SetTimeout(interval)) return nullptr;
		timer->ep = this;
		timer->onFire = std::move(cb);
		timer->indexAtContainer = (int)timers.size();
		timers.push_back(timer);
		return timer;
	}

	// 进入一次 epoll wait. 可传入超时时间. 
	inline int Wait(int const& timeoutMS) {
		int n = epoll_wait(efd, events.data(), (int)events.size(), timeoutMS);
		if (n == -1) return errno;
		for (int i = 0; i < n; ++i) {
			auto&& h = fdHandlers[events[i].data.fd];
			assert(h);
			auto e = events[i].events;
			if (e & EPOLLERR || e & EPOLLHUP) {
				h->Dispose();
			}
			else {
				if (h->OnEpollEvent(e)) {
					h->Dispose();
				}
			}
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

				// 驱动 timers
				timeoutManager.Update();

				// 帧逻辑调用一次
				if (int r = Update()) return r;
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

	// 帧逻辑可以放在这. 返回非 0 将令 Run 退出
	inline virtual int Update() {
		return 0;
	}
};


inline void FDHandler::Dispose(int const& flag) noexcept {
	// 检查 & 打标记 以避免重复执行析构逻辑
	if (disposed) return;
	disposed = true;

	// 调用派生类自己的析构部分
	Disposing(flag);

	// 从 ep 移除监视 并关闭
	assert(fd != -1);
	epoll_ctl(ep->efd, EPOLL_CTL_DEL, fd, nullptr);
	close(fd);

	// 从容器移除以触发析构
	if (flag != -1) {
		ep->fdHandlers[fd].reset();
	}
}


inline void Timer::OnTimeout() noexcept {
	if (onFire) {
		onFire(this);
	}
	if (timeoutIndex == -1) {
		Dispose();
	}
}

inline void Timer::Dispose(int const& flag) noexcept {
	// 检查 & 打标记 以避免重复执行析构逻辑
	if (disposed) return;
	disposed = true;

	// 调用派生类自己的析构部分
	Disposing(flag);

	// 从 timeoutManager 移除
	if (timeoutIndex != -1) {
		SetTimeout(0);
	}

	// 清除可能存在的循环引用
	onFire = nullptr;

	// 从容器移除以触发析构
	if (flag != -1) {
		if (indexAtContainer != -1) {
			XX_SWAP_REMOVE(this, indexAtContainer, ep->timers);
		}
	}
}


inline std::shared_ptr<TcpPeer> TcpListener::OnCreatePeer() {
	return xx::Make<TcpPeer>();
}

inline int TcpListener::OnEpollEvent(uint32_t const& e) {
	// accept 到 没有 或 出错 为止
	while (Accept(fd) > 0) {};
	return 0;
}

inline int TcpListener::Accept(int const& listenFD) {
	sockaddr addr;									// todo: ipv6 support
	socklen_t len = sizeof(addr);

	// 接收并得到目标 fd
	int fd = accept(listenFD, &addr, &len);
	if (-1 == fd) {
		ep->lastErrorNumber = errno;
		if (ep->lastErrorNumber == EAGAIN || ep->lastErrorNumber == EWOULDBLOCK) return 0;
		else return -1;
	}

	// 确保退出时自动关闭 fd
	xx::ScopeGuard sg([&] { close(fd); });

	// 如果创建 socket 容器类失败则直接退出
	auto peer = OnCreatePeer();
	if (!peer) return fd;
	peer->ep = ep;
	peer->fd = fd;
	peer->timeoutManager = &ep->timeoutManager;

	// 继续初始化该 fd. 如果有异常则退出
	if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
	int on = 1;
	if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) return -3;

	// 填充 ip
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	if (!getnameinfo(&addr, len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
		xx::Append(peer->ip, hbuf, ":", sbuf);
	}

	// 纳入 ep 管理
	if (-1 == ep->Ctl(fd, EPOLLIN)) return -6;
	ep->fdHandlers[fd] = peer;

	// 调用用户自定义后续绑定
	OnAccept(peer);

	// 取消自动 close fd 行为
	sg.Cancel();
	return fd;
}

inline int TcpListener::OnAccept(std::shared_ptr<TcpPeer>& peer) {
	return 0;
}


inline void TcpPeer::Disposing(int const& flag) noexcept {
	if (timeoutIndex != -1) {
		SetTimeout(0);
	}
	if (flag == 1) {
		OnDisconnect();
	}
}

inline int TcpPeer::Send(xx::Buf&& eb) {
	sendQueue.value().Push(std::move(eb));
	return !writing ? Write() : 0;
}

inline int TcpPeer::Flush() {
	return !writing ? Write() : 0;
}

inline int TcpPeer::Write() {
	auto&& q = sendQueue.value();

	// 如果没有待发送数据，停止监控 EPOLLOUT 并退出
	if (!q.bytes) return ep->Ctl(fd, EPOLLIN, EPOLL_CTL_MOD);

	// 前置准备
	std::array<iovec, maxNumIovecs> vs;					// buf + len 数组指针
	int vsLen = 0;										// 数组长度
	auto bufLen = sendLenPerFrame;						// 计划发送字节数

	// 填充 vs, vsLen, bufLen 并返回预期 offset. 每次只发送 bufLen 长度
	auto&& offset = q.Fill(vs, vsLen, bufLen);

	// 返回值为 实际发出的字节数
	auto&& sentLen = writev(fd, vs.data(), vsLen);

	// 已断开
	if (sentLen == 0) return -2;

	// 繁忙 或 出错
	else if (sentLen == -1) {
		if (errno == EAGAIN) goto LabEnd;
		else return -3;
	}

	// 完整发送
	else if ((std::size_t)sentLen == bufLen) {
		// 快速弹出已发送数据
		q.Pop(vsLen, offset, bufLen);

		// 这次就写这么多了. 直接返回. 下次继续 Write
		return 0;
	}

	// 发送了一部分
	else {
		// 弹出已发送数据
		q.Pop(sentLen);
	}

LabEnd:
	// 标记为不可写
	writing = true;

	// 开启对可写状态的监控, 直到队列变空再关闭监控
	return ep->Ctl(fd, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
}

inline int TcpPeer::Read() {
	if (!recv.cap) {
		recv.Reserve(readBufLen);
	}
	if (recv.len == recv.cap) return -1;
	auto&& len = read(fd, recv.buf + recv.len, recv.cap - recv.len);
	if (len <= 0) return -2;
	recv.len += len;
	return 0;
}

inline int TcpPeer::OnEpollEvent(uint32_t const& e) {
	// read
	if (e & EPOLLIN) {
		if (int r = Read()) return r;
		if (int r = OnReceive()) return r;
	}
	// write
	if (e & EPOLLOUT) {
		// 设置为可写状态
		writing = false;
		if (int r = Write()) return r;
	}
	return 0;
}

inline int TcpPeer::OnReceive() {
	// 默认实现为 echo
	auto&& r = write(fd, recv.buf, recv.len) > 0 ? 0 : -1;
	recv.Clear();
	return r;
}

inline void TcpPeer::OnTimeout() {
	Dispose();
}



inline void TcpConn::OnTimeout() {
	OnConnect();
	Dispose();
}

inline int TcpConn::OnEpollEvent(uint32_t const& e) {
	if (connected) {
		return this->TcpPeer::OnEpollEvent(e);
	}
	// 读取错误 或者读到错误 都认为是连接失败. 
	int err;
	socklen_t result_len = sizeof(err);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &result_len) == -1 || err) {
		OnConnect();
		return -1;
	}
	// 拨号连接成功. 打标记. 避免再次进入该 if 分支
	SetTimeout(0);
	connected = true;
	OnConnect();
	return 0;
}





struct L : TcpListener {
	inline virtual int OnAccept(std::shared_ptr<TcpPeer>& peer) override {
		// 设置 3 秒后自动断开
		return peer->SetTimeout(30);
	}
};

struct C : TcpConn {
	inline virtual void OnConnect() override {
		xx::CoutN("connected = ", connected);
	}
};

int main() {
	xx::IgnoreSignal();
	Epoll ep;
	ep.Delay(10, [&](Timer* const& timer) {
		xx::CoutN("111");
		ep.TcpDial<C>("127.0.0.1", 12345, 20);
		xx::CoutN("222");
		});
	auto listener = ep.TcpListen<L>(12345);
	return ep.Run(10);
}
