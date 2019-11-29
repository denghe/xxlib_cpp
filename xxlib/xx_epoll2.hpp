#pragma once
#include "xx_epoll2.h"

namespace xx::Epoll {
	/***********************************************************************************************************/
	// FDHandler
	/***********************************************************************************************************/

	inline void FDHandler::Dispose(int const& flag) {
		// 检查 & 打标记 以避免重复执行析构逻辑
		if (disposed) return;
		disposed = true;

		// 调用派生类自己的析构部分
		Disposing(flag);

		// 从 ep 移除监视 并关闭
		if (ep) {
			ep->CloseDel(fd);

			// 从容器移除以触发析构
			if (flag != -1) {
				ep->fdHandlers[fd].reset();
			}
		}
	}

	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	inline void Timer::OnTimeout() {
		assert(!disposed);

		if (onFire) {
			onFire(this);
		}
		// 非 repeat 模式( 未再次 SetTimeout )直接自杀
		if (timeoutIndex == -1) {
			Dispose(0);
		}
	}

	inline void Timer::Dispose(int const& flag) {
		// 检查 & 打标记 以避免重复执行析构逻辑
		if (disposed) return;
		disposed = true;

		// 调用派生类自己的析构部分
		Disposing(flag);

		// 从 timeoutManager 移除
		if (timeoutIndex != -1) {
			SetTimeout(0);
		}

		// 从容器移除以触发析构
		if (flag != -1) {
			// 清除可能存在的 lambda 持有
			onFire = nullptr;

			if (indexAtContainer != -1) {
				XX_SWAP_REMOVE(this, indexAtContainer, ep->timers);
			}
		}
		else {
			assert(indexAtContainer == -1);
		}
	}


	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	inline void TcpPeer::Disposing(int const& flag) {
		if (timeoutIndex != -1) {
			SetTimeout(0);
		}
		if (flag == 1) {
			OnDisconnect();
		}
	}

	inline int TcpPeer::Send(xx::Buf&& eb) {
		sendQueue.Push(std::move(eb));
		return !writing ? Write() : 0;
	}

	inline int TcpPeer::Flush() {
		return !writing ? Write() : 0;
	}

	inline int TcpPeer::Write() {
		// 如果没有待发送数据，停止监控 EPOLLOUT 并退出
		if (!sendQueue.bytes) return ep->Ctl(fd, EPOLLIN, EPOLL_CTL_MOD);

		// 前置准备
		std::array<iovec, maxNumIovecs> vs;					// buf + len 数组指针
		int vsLen = 0;										// 数组长度
		auto bufLen = sendLenPerFrame;						// 计划发送字节数

		// 填充 vs, vsLen, bufLen 并返回预期 offset. 每次只发送 bufLen 长度
		auto&& offset = sendQueue.Fill(vs, vsLen, bufLen);

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
			sendQueue.Pop(vsLen, offset, bufLen);

			// 这次就写这么多了. 直接返回. 下次继续 Write
			return 0;
		}

		// 发送了一部分
		else {
			// 弹出已发送数据
			sendQueue.Pop(sentLen);
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
		// 默认实现为 echo. 仅供测试. 随意使用 write 可能导致待发队列中的数据被分割
		auto&& r = write(fd, recv.buf, recv.len) == recv.len ? 0 : -1;
		recv.Clear();
		return r;
	}

	inline void TcpPeer::OnTimeout() {
		Dispose(1);
	}


	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	inline std::shared_ptr<TcpPeer> TcpListener::OnCreatePeer() {
		return xx::TryMake<TcpPeer>();
	}

	inline int TcpListener::OnEpollEvent(uint32_t const& e) {
		// accept 到 没有 或 出错 为止
		while (Accept(fd) > 0) {};
		return 0;
	}

	inline int TcpListener::Accept(int const& listenFD) {
		// 开始创建 fd
		// todo: ipv6 support
		sockaddr addr;
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

		// 如果 fd 超出最大存储限制就退出。返回 fd 是为了外部能继续执行 accept
		if (fd >= (int)ep->fdHandlers.size()) return fd;
		assert(!ep->fdHandlers[fd]);

		// 继续初始化该 fd. 如果有异常则退出
		if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
		int on = 1;
		if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) return -3;

		// 纳入 ep 管理
		if (-1 == ep->Ctl(fd, EPOLLIN)) return -4;

		// 确保退出时关闭 fd 并从 epoll 监视移除
		sg.Set([&] { ep->CloseDel(fd); });



		// 创建类容器
		auto peer = OnCreatePeer();

		// 允许创建失败( 比如内存不足，或者刻意控制数量 ). 失败不触发 OnAccept
		if (!peer) return fd;

		// 继续初始化( 下列步骤几乎不可能失败 )
		peer->ep = ep;
		peer->timeoutManager = &ep->timeoutManager;
		peer->fd = fd;
		peer->recv.Reserve(peer->readBufLen);
		ep->fdHandlers[fd] = peer;

		// 填充 ip
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
		if (!getnameinfo(&addr, len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
			xx::Append(peer->ip, hbuf, ":", sbuf);
		}

		// 调用用户自定义后续初始化
		peer->Init();

		// 调用用户自定义后续绑定
		OnAccept(peer);

		sg.Cancel();
		return fd;
	}



	/***********************************************************************************************************/
	// TcpConn
	/***********************************************************************************************************/

	inline int TcpConn::OnEpollEvent(uint32_t const& e) {
		// 已连接就直接执行 TcpPeer 旧逻辑
		if (connected) {
			return this->TcpPeer::OnEpollEvent(e);
		}

		// 读取错误 或者读到错误 都认为是连接失败. 返回非 0 触发 Dispose(1) 间接触发 OnConnect
		int err;
		socklen_t result_len = sizeof(err);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &result_len) == -1 || err) return -1;

		// 连接成功. 清理连接超时检测. 打成功标记. 触发回调
		SetTimeout(0);
		connected = true;
		OnConnect();
		return 0;
	}

	inline void TcpConn::Disposing(int const& flag) {
		// 已连接就直接执行 TcpPeer 旧逻辑
		if (connected) {
			this->TcpPeer::Disposing(flag);
			return;
		}

		// 回调并析构其他
		if (flag == 1) {
			OnConnect();
		}
		this->TcpPeer::Disposing(0);
	}



	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	inline Context::Context(int const& maxNumFD) {
		assert(maxNumFD >= 1024);

		// 创建 epoll fd
		efd = epoll_create1(0);
		if (-1 == efd) throw - 1;

		// 初始化 fd 处理器存储空间
		fdHandlers.resize(maxNumFD);
	}

	inline Context::~Context() {
		for (auto&& o : timers) {
			if (o && !o->Disposed()) {
				o->Dispose(0);
			}
		}
		for (auto&& o : fdHandlers) {
			if (o && !o->Disposed()) {
				o->Dispose(0);
			}
		}
		if (efd != -1) {
			close(efd);
			efd = -1;
		}
	}

	inline int Context::MakeListenFD(int const& port) {
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

	inline int Context::Ctl(int const& fd, uint32_t const& flags, int const& op) {
		epoll_event event;
		event.data.fd = fd;
		event.events = flags;
		return epoll_ctl(efd, op, fd, &event);
	};

	inline int Context::CloseDel(int const& fd) {
		assert(fd != -1);
		close(fd);
		return epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
	}

	inline int Context::Wait(int const& timeoutMS) {
		int n = epoll_wait(efd, events.data(), (int)events.size(), timeoutMS);
		if (n == -1) return errno;
		for (int i = 0; i < n; ++i) {
			auto&& h = fdHandlers[events[i].data.fd];
			assert(h);
			auto e = events[i].events;
			if (e & EPOLLERR || e & EPOLLHUP) {
				h->Dispose(1);
			}
			else {
				if (h->OnEpollEvent(e)) {
					h->Dispose(1);
				}
			}
		}
		return 0;
	}

	inline int Context::Run(double const& frameRate) {
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

	// 创建 监听器
	template<typename L, typename ...Args>
	inline std::shared_ptr<L> Context::TcpListen(int const& port, Args&&... args) {
		static_assert(std::is_base_of_v<TcpListener, L>);

		// 创建监听用 socket fd
		auto&& fd = MakeListenFD(port);
		if (fd < 0) {
			lastErrorNumber = -1;
			return nullptr;
		}
		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// 检测 fd 存储上限
		if (fd >= (int)fdHandlers.size()) {
			lastErrorNumber = -2;
			return nullptr;
		}
		assert(!fdHandlers[fd]);

		// 设置为非阻塞
		if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) {
			lastErrorNumber = -3;
			return nullptr;
		}

		// 开始监听
		if (-1 == listen(fd, SOMAXCONN)) {
			lastErrorNumber = -4;
			return nullptr;
		}

		// fd 纳入 epoll 管理
		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -5;
			return nullptr;
		}

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { CloseDel(fd); });

		// 试创建目标类实例
		auto o = xx::TryMake<L>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -6;
			return nullptr;
		}

		// 继续初始化并放入容器
		o->ep = this;
		o->fd = fd;
		fdHandlers[fd] = o;

		// 调用用户自定义后续初始化
		o->Init();

		// 撤销自动关闭行为并返回结果
		sg.Cancel();
		return o;
	}


	// 创建 连接 peer
	template<typename C, typename ...Args>
	inline std::shared_ptr<C> Context::TcpDial(char const* const& ip, int const& port, int const& timeoutInterval, Args&&... args) {
		static_assert(std::is_base_of_v<TcpConn, C>);

		// 转换地址字符串为 addr 格式
		// todo: ipv6 support
		sockaddr_in dest;
		memset(&dest, 0, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons((uint16_t)port);
		if (!inet_pton(AF_INET, ip, &dest.sin_addr.s_addr)) {
			lastErrorNumber = -1;
			return nullptr;
		}

		// 创建 tcp 非阻塞 socket fd
		auto&& fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (fd == -1) {
			lastErrorNumber = -2;
			return nullptr;
		}
		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// 检测 fd 存储上限
		if (fd >= (int)fdHandlers.size()) {
			lastErrorNumber = -8;
			return nullptr;
		}
		assert(!fdHandlers[fd]);

		// 设置一些 tcp 参数( 可选 )
		int on = 1;
		if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) {
			lastErrorNumber = -3;
			return nullptr;
		}

		// 开始连接
		if (connect(fd, (sockaddr*)&dest, sizeof(dest)) == -1) {
			if (errno != EINPROGRESS) {
				lastErrorNumber = -4;
				return nullptr;
			}
		}
		// else : 立刻连接上了

		// 纳入 epoll 管理
		if (int r = Ctl(fd, EPOLLIN | EPOLLOUT)) {
			lastErrorNumber = -5;
			return nullptr;
		}
		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { CloseDel(fd); });

		// 试创建目标类实例
		auto o = xx::TryMake<C>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -6;
			return nullptr;
		}

		// 设置超时时长
		o->timeoutManager = &timeoutManager;
		if (int r = o->SetTimeout(timeoutInterval)) {
			lastErrorNumber = -7;
			return nullptr;
		}

		// 继续初始化并放入容器
		o->ep = this;
		o->fd = fd;
		o->recv.Reserve(o->readBufLen);
		fdHandlers[fd] = o;

		// 调用用户自定义后续初始化
		o->Init();

		// 撤销自动关闭行为并返回结果
		sg.Cancel();
		return o;
	}



	// 创建 timer
	template<typename T, typename ...Args>
	inline std::shared_ptr<T> Context::Delay(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args) {
		static_assert(std::is_base_of_v<Timer, T>);

		// 试创建目标类实例
		auto o = xx::TryMake<T>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -1;
			return nullptr;
		}

		// 设置超时时长
		o->timeoutManager = &timeoutManager;
		if (o->SetTimeout(interval)) {
			lastErrorNumber = -2;
			return nullptr;
		}

		// 继续初始化并放入容器
		o->ep = this;
		o->onFire = std::move(cb);
		o->indexAtContainer = (int)timers.size();
		timers.push_back(o);

		// 调用用户自定义后续初始化
		o->Init();

		return o;
	}
}
