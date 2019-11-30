#pragma once
#include "xx_epoll2.h"

namespace xx {
	// 适配 sockaddr*
	template<typename T>
	struct SFuncs<T, std::enable_if_t<std::is_same_v<sockaddr*, std::decay_t<T>>>> {
		static inline void WriteTo(std::string& s, T const& in) noexcept {
			char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
			if (!getnameinfo(in, in->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN
				, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
				xx::Append(s, (char*)hbuf, ":", (char*)sbuf);
			}
		}
	};
}

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

	inline int TcpPeer::Send(xx::Buf&& data) {
		sendQueue.Push(std::move(data));
		return !writing ? Write() : 0;
	}

	inline int TcpPeer::Flush() {
		return !writing ? Write() : 0;
	}

	inline int TcpPeer::Write() {
		// 如果没有待发送数据，停止监控 EPOLLOUT 并退出
		if (!sendQueue.bytes) return ep->Ctl(fd, EPOLLIN, EPOLL_CTL_MOD);

		// 前置准备
		std::array<iovec, UIO_MAXIOV> vs;					// buf + len 数组指针
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
		// 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
		if (!recv.cap) {
			recv.Reserve(readBufLen);
		}

		// 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
		if (recv.len == recv.cap) return -1;

		// 通过 fd 从系统网络缓冲区读取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则认为失败 自杀
		auto&& len = read(fd, recv.buf + recv.len, recv.cap - recv.len);
		if (len <= 0) return -2;
		recv.len += len;

		// 调用用户数据处理函数
		return OnReceive();
	}

	inline int TcpPeer::OnEpollEvent(uint32_t const& e) {
		// read
		if (e & EPOLLIN) {
			if (int r = Read()) return r;
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
		auto&& r = write(fd, recv.buf, recv.len) == (ssize_t)recv.len ? 0 : -1;
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

		// 设置非阻塞状态
		if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;

		// 设置一些 tcp 参数( 可选 )
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
		if (OnAccept(peer)) {
			peer->Dispose(0);
		}

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
	// UdpPeer
	/***********************************************************************************************************/

	inline int UdpPeer::OnEpollEvent(uint32_t const& e) {
		// read
		if (e & EPOLLIN) {
			char buf[65536];
			sockaddr_in fromAddr;
			socklen_t addrLen = sizeof(fromAddr);
			auto len = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*) & fromAddr, &addrLen);
			if (len < 0) {
				ep->lastErrorNumber = (int)len;
				return 0;
			}

			//auto hostp = gethostbyaddr((const char*)&fromAddr.sin_addr.s_addr, sizeof(fromAddr.sin_addr.s_addr), AF_INET);
			//if (!hostp) {
			//	ep->lastErrorNumber = -1;
			//	return 0;
			//}
			//auto hostaddrp = inet_ntoa(fromAddr.sin_addr);
			//if (!hostaddrp) {
			//	ep->lastErrorNumber = -2;
			//	return 0;
			//}
			//printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);

			return OnReceive((sockaddr*)&fromAddr, buf, len);
		}
		// write:
		// udp 似乎不必关注 write 状态。无脑认为可写。写也不必判断是否成功。
		return 0;
	}

	inline int UdpPeer::OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len) {
		// echo. 忽略返回值
		(void)SendTo(fromAddr, buf, len);
		return 0;
	}

	inline int UdpPeer::SendTo(sockaddr* toAddr, char const* const& buf, std::size_t const& len) {
		return (int)sendto(fd, buf, len, 0, toAddr, toAddr->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);
	}



	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	inline std::shared_ptr<KcpPeer> UdpListener::OnCreatePeer() {
		return xx::TryMake<KcpPeer>();
	}

	inline int UdpListener::OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len) {
		// sockaddr* 转为 ip:port
		std::string ipAndPort;
		xx::Append(ipAndPort, fromAddr);

		// 当前握手方案为 UdpDialer 每秒 N 次不停发送 4 字节数据( serial )过来, 
		// 收到后根据其 ip:port 做 key, 生成 convId
		// 每次收到都向其发送 convId
		if (len == 4) {
			auto&& idx = ep->shakes.Find(ipAndPort);
			if (idx == -1) {
				idx = ep->shakes.Add(ipAndPort, std::make_pair(++convId, NowSteadyEpochMS() + handShakeTimeoutMS)).index;
			}
			char tmp[8];	// serial + convId
			memcpy(tmp, buf, 4);
			memcpy(tmp + 4, &ep->shakes.ValueAt(idx).first, 4);
			SendTo(fromAddr, tmp, 8);
			return 0;
		}

		// 忽略长度小于 kcp 头的数据包 ( IKCP_OVERHEAD at ikcp.c )
		if (len < 24) return 0;

		// read conv header
		uint32_t conv;
		memcpy(&conv, buf, sizeof(conv));
		std::shared_ptr<KcpPeer> peer;

		// 根据 conv 试定位到 peer
		auto&& peerIter = ep->kcps.Find(conv);

		// 如果不存在 就在 shakes 中按 ip:port 找
		if (peerIter == -1) {
			auto&& idx = ep->shakes.Find(ipAndPort);

			// 未找到或 conv 对不上: 忽略
			if (idx == -1 || ep->shakes.ValueAt(idx).first != conv) return 0;

			// 从握手信息移除
			ep->shakes.RemoveAt(idx);

			// 始创建 peer
			peer = OnCreatePeer();
			if (!peer) return 0;

			// 继续初始化
			peer->ep = ep;
			peer->owner = As<UdpListener>(shared_from_this());
			peer->conv = conv;
			peer->createMS = NowSteadyEpochMS();

			// 如果初始化 kcp 失败就忽略
			if (peer->InitKcp()) return 0;

			// 更新地址信息
			memcpy(&peer->addr, fromAddr, fromAddr->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);

			// 放入容器
			ep->kcps[conv] = peer;

			// 触发事件回调
			if (OnAccept(peer)) {
				peer->Dispose(0);
				return 0;
			}
		}
		else {
			// 定位到目标 peer
			peer = ep->kcps.ValueAt(peerIter);
			assert(peer && !peer->Disposed());

			// 更新地址信息
			memcpy(&peer->addr, fromAddr, fromAddr->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);
		}

		// 将数据灌入 kcp. 进而可能触发 peer->OnReceive
		if (peer->Input((uint8_t*)buf, len)) {
			peer->Dispose(1);
		}
		return 0;
	}




	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	inline void KcpPeer::Dispose(int const& flag) {
		// 检查 & 打标记 以避免重复执行析构逻辑
		if (disposed) return;
		disposed = true;

		// 调用派生类自己的析构部分
		Disposing(flag);

		// 这里放置 从容器移除的代码 以触发析构
		ep->kcps.Remove(conv);
	};

	inline int KcpPeer::OnReceive() {
		int r = Send(recv.buf, recv.len);
		recv.Clear();
		return r;
	}

	inline int KcpPeer::InitKcp() {
		assert(!kcp);
		// 创建并设置 kcp 的一些参数
		kcp = ikcp_create(conv, this);
		if (!kcp) return -1;
		int r = ikcp_wndsize(kcp, 1024, 1024);
		r = ikcp_nodelay(kcp, 1, 10, 2, 1);
		kcp->rx_minrto = 10;
		kcp->stream = 1;

		// 给 kcp 绑定 output 功能函数
		ikcp_setoutput(kcp, [](const char* inBuf, int len, ikcpcb* kcp, void* user)->int {
			auto self = (KcpPeer*)user;
			if (!self->owner) return -1;
			return self->owner->SendTo((sockaddr*)&self->addr, inBuf, len);
			});
		return 0;
	}

	inline int KcpPeer::UpdateKcpLogic(int64_t const& nowMS) {
		assert(!disposed);
		assert(kcp);
		// 计算出当前 ms
		// 已知问题: 受 ikcp uint32 限制, 连接最多保持 50 多天
		auto&& currentMS = uint32_t(nowMS - createMS);

		// 如果 update 时间没到 就退出
		if (nextUpdateMS > currentMS) return 0;

		// 来一发
		ikcp_update(kcp, currentMS);

		// 更新下次 update 时间
		nextUpdateMS = ikcp_check(kcp, currentMS);

		// 开始处理收到的数据
		do {
			// 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
			if (!recv.cap) {
				recv.Reserve(readBufLen);
			}
			// 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
			if (recv.len == recv.cap) return -1;

			// 从 kcp 提取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则下次再说
			auto&& len = ikcp_recv(kcp, (char*)recv.buf + recv.len, recv.cap - recv.len);
			if (len <= 0) break;

			// 调用用户数据处理函数
			if (int r = OnReceive()) {
				Dispose(1);
				return r;
			}
		} while (true);
		return 0;
	}

	inline int KcpPeer::Send(uint8_t const* const& buf, ssize_t const& dataLen) {
		return ikcp_send(kcp, (char*)buf, (int)dataLen);
	}

	inline void KcpPeer::Flush() {
		ikcp_flush(kcp);
	}

	inline int KcpPeer::Input(uint8_t* const& recvBuf, uint32_t const& recvLen) {
		return ikcp_input(kcp, (char*)recvBuf, recvLen);
	}

	inline void KcpPeer::OnTimeout() {
		Dispose(1);
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

	inline int Context::MakeSocketFD(int const& port, int const& sockType) {
		char portStr[20];
		snprintf(portStr, sizeof(portStr), "%d", port);

		addrinfo hints;														// todo: ipv6 support
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_UNSPEC;										// ipv4 / 6
		hints.ai_socktype = sockType;										// SOCK_STREAM / SOCK_DGRAM
		hints.ai_flags = AI_PASSIVE;										// all interfaces

		addrinfo* ai_, * ai;
		if (getaddrinfo(nullptr, portStr, &hints, &ai_)) return -1;

		int fd;
		for (ai = ai_; ai != nullptr; ai = ai->ai_next) {
			fd = socket(ai->ai_family, ai->ai_socktype | SOCK_NONBLOCK, ai->ai_protocol);
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

		if (!ai) return -2;

		// 检测 fd 存储上限
		if (fd >= (int)fdHandlers.size()) {
			close(fd);
			return -3;
		}
		assert(!fdHandlers[fd]);

		return fd;
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

	inline void Context::UpdateKcps() {
		auto nowMS = xx::NowSteadyEpochMS();
		for (auto&& data : kcps) {
			if (auto r = data.value->UpdateKcpLogic(nowMS)) {
				data.value->Dispose(1);
			}
		}

		for (auto&& iter = shakes.begin(); iter != shakes.end(); ++iter) {
			if (iter->value.second < nowMS) {
				iter.Remove();
			}
		}
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

				// 驱动 kcps
				UpdateKcps();

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

	// 创建 tcp 监听器
	template<typename L, typename ...Args>
	inline std::shared_ptr<L> Context::TcpListen(int const& port, Args&&... args) {
		static_assert(std::is_base_of_v<TcpListener, L>);

		// 创建监听用 socket fd
		auto&& fd = MakeSocketFD(port);
		if (fd < 0) {
			lastErrorNumber = -1;
			return nullptr;
		}
		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// 开始监听
		if (-1 == listen(fd, SOMAXCONN)) {
			lastErrorNumber = -3;
			return nullptr;
		}

		// fd 纳入 epoll 管理
		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -4;
			return nullptr;
		}

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { CloseDel(fd); });

		// 试创建目标类实例
		auto o = xx::TryMake<L>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -5;
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
		// todo: ipv6 support, 域名解析?
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



	// 创建 UdpPeer. port 传 0 则自适应( 仅用于发数据 )
	template<typename U, typename ...Args>
	inline std::shared_ptr<U> Context::UdpBind(int const& port, Args&&... args) {
		static_assert(std::is_base_of_v<UdpPeer, U>);

		// 创建 udp socket fd
		auto&& fd = MakeSocketFD(port, SOCK_DGRAM);
		if (fd < 0) {
			lastErrorNumber = fd;
			return nullptr;
		}
		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// fd 纳入 epoll 管理
		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -4;
			return nullptr;
		}

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { CloseDel(fd); });

		// 试创建目标类实例
		auto o = xx::TryMake<U>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -5;
			return nullptr;
		}

		// 继续初始化并放入容器
		o->ep = this;
		o->fd = fd;
		o->port = port;
		fdHandlers[fd] = o;

		// 调用用户自定义后续初始化
		o->Init();

		// 撤销自动关闭行为并返回结果
		sg.Cancel();
		return o;
	}

}
