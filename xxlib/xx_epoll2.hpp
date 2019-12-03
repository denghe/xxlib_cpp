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
	// Item
	/***********************************************************************************************************/

	inline void Item::Dispose() {
		if (ep && indexAtContainer != -1) {
			ep->items.RemoveAt(indexAtContainer);
		}
	}

	/***********************************************************************************************************/
	// Item_r
	/***********************************************************************************************************/

	template<typename T>
	inline Item_r<T>::Item_r(T* const& ptr) {
		assert(ptr);
		assert(ptr->ep);
		assert(ptr->indexAtContainer != -1);
		if constexpr (!std::is_base_of_v<FDItem, T>) {
			items = &ptr->ep->items;
		}
		index = ptr->indexAtContainer;
		version = ptr->ep->items.VersionAt(index);
	}

	template<typename T>
	inline Item_r<T>::Item_r(std::unique_ptr<T> const& ptr) : Item_r(ptr.get()) {}

	template<typename T>
	inline Item_r<T>::operator bool() const {
		if constexpr (std::is_base_of_v<FDItem, T>) {
			return !version && Context::fdHandlers[index].second == version;
		}
		else {
			return !version && items->VersionAt(index) == version;
		}
	}

	template<typename T>
	inline T* Item_r<T>::operator->() const {
		if (!operator bool()) throw - 1;		// 空指针
		if constexpr (std::is_base_of_v<FDItem, T>) {
			return (T*)Context::fdHandlers[index].first.get();
		}
		else {
			return (T*)items->ValueAt(index).get();
		}
	}

	template<typename T>
	inline T* Item_r<T>::Lock() const {
		if constexpr (std::is_base_of_v<FDItem, T>) {
			return operator bool() ? (T*)Context::fdHandlers[index].first.get() : nullptr;
		}
		else {
			return operator bool() ? (T*)items->ValueAt(index).get() : nullptr;
		}
	}


	template<typename T>
	template<typename U>
	inline Item_r<U> Item_r<T>::As() const {
		auto p = Lock();
		if (!dynamic_cast<U*>(p)) return Item_r<U>();
		Item_r<U> rtv;
		if constexpr (!std::is_base_of_v<FDItem, T>) {
			rtv.items = items;
		}
		rtv.index = index;
		rtv.version = version;
		return rtv;
	}


	/***********************************************************************************************************/
	// FDItem
	/***********************************************************************************************************/

	inline void FDItem::Dispose() {
		if (ep && indexAtContainer != -1) {
			ep->fdHandlers[indexAtContainer].first.reset();
		}
	}

	inline FDItem::~FDItem() {
		if (ep && indexAtContainer != -1) {
			ep->CloseDel(indexAtContainer);
			indexAtContainer = -1;
		}
	}


	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	inline TimeoutManager* Timer::GetTimeoutManager() {
		return ep;
	}

	inline void Timer::OnTimeout() {
		if (onFire) {
			onFire(this);
		}
		// 非 repeat 模式( 未再次 SetTimeout )直接自杀
		if (timeoutIndex == -1) {
			Dispose();
		}
	}

	inline Timer::~Timer() {
		SetTimeout(0);
	}

	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	inline TimeoutManager* TcpPeer::GetTimeoutManager() {
		return ep;
	}

	inline TcpPeer::~TcpPeer() {
		SetTimeout(0);
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
		if (!sendQueue.bytes) return ep->Ctl(indexAtContainer, EPOLLIN, EPOLL_CTL_MOD);

		// 前置准备
		std::array<iovec, UIO_MAXIOV> vs;					// buf + len 数组指针
		int vsLen = 0;										// 数组长度
		auto bufLen = sendLenPerFrame;						// 计划发送字节数

		// 填充 vs, vsLen, bufLen 并返回预期 offset. 每次只发送 bufLen 长度
		auto&& offset = sendQueue.Fill(vs, vsLen, bufLen);

		// 返回值为 实际发出的字节数
		auto&& sentLen = writev(indexAtContainer, vs.data(), vsLen);

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
		return ep->Ctl(indexAtContainer, EPOLLIN | EPOLLOUT, EPOLL_CTL_MOD);
	}

	inline void TcpPeer::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			OnDisconnect(-1);
			Dispose();
			return;
		}
		// read
		if (e & EPOLLIN) {
			// 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
			if (!recv.cap) {
				recv.Reserve(readBufLen);
			}

			// 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
			if (recv.len == recv.cap) {
				OnDisconnect(-2);
				Dispose();
				return;
			}

			// 通过 fd 从系统网络缓冲区读取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则认为失败 自杀
			auto&& len = read(indexAtContainer, recv.buf + recv.len, recv.cap - recv.len);
			if (len <= 0) {
				OnDisconnect(-3);
				Dispose();
				return;
			}
			recv.len += len;

			// 用弱引用检测 OnReceive 中是否发生自杀行为
			Item_r<TcpPeer> alive(this);
			OnReceive();
			if (!alive) return;	// 已自杀
		}
		// write
		if (e & EPOLLOUT) {
			// 设置为可写状态
			writing = false;
			if (int r = Write()) {
				ep->lastErrorNumber = r;
				OnDisconnect(-4);
				Dispose();
				return;
			}
		}
	}

	inline void TcpPeer::OnReceive() {
		// 默认实现为 echo. 仅供测试. 随意使用 write 可能导致待发队列中的数据被分割
		if (write(indexAtContainer, recv.buf, recv.len) != (ssize_t)recv.len) {
			Dispose();
		}
	}

	inline void TcpPeer::OnTimeout() {
		Dispose();
	}


	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	inline std::unique_ptr<TcpPeer> TcpListener::OnCreatePeer() {
		return xx::TryMakeU<TcpPeer>();
	}

	inline void TcpListener::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}
		// accept 到 没有 或 出错 为止
		while (Accept(indexAtContainer) > 0) {};
	}

	inline int TcpListener::Accept(int const& listenFD) {
		// 开始创建 fd
		sockaddr addr;						// todo: ipv6 support. 根据 listener fd 的协议栈来路由
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
		assert(!ep->fdHandlers[fd].first);

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
		auto peer_p = peer.get();

		// 预填充并放入容器
		peer->ep = ep;
		peer->indexAtContainer = fd;
		ep->fdHandlers[fd].first = std::move(peer);		// 移动之后下面要继续访问用 peer_p
		ep->fdHandlers[fd].second = ++ep->autoIncVersion;

		// 填充 ip
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
		if (!getnameinfo(&addr, len, hbuf, sizeof hbuf, sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV)) {
			xx::Append(peer_p->ip, hbuf, ":", sbuf);
		}

		// 调用用户自定义后续绑定
		OnAccept(Item_r<TcpPeer>(peer_p));

		sg.Cancel();
		return fd;
	}



	/***********************************************************************************************************/
	// TcpConn
	/***********************************************************************************************************/

	inline void TcpConn::OnEpollEvent(uint32_t const& e) {
		// 如果 dialer 无法定位到 或 error 事件则自杀
		auto dialer = this->dialer.Lock();
		if (!dialer || e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}
		// 读取错误 或者读到错误 都认为是连接失败. 返回非 0 触发 Dispose
		int err;
		socklen_t result_len = sizeof(err);
		if (getsockopt(indexAtContainer, SOL_SOCKET, SO_ERROR, &err, &result_len) == -1 || err) {
			// error
			if (e & EPOLLERR || e & EPOLLHUP) {
				Dispose();
				return;
			}
		}

		// 连接成功
		auto fd = this->indexAtContainer;		// 备份
		this->indexAtContainer = -1;			// 设为 -1 以免 close
		dialer->Finish(fd);
	}


	/***********************************************************************************************************/
	// TcpDialer
	/***********************************************************************************************************/

	inline TimeoutManager* TcpDialer::GetTimeoutManager() {
		return ep;
	}

	inline int TcpDialer::AddAddress(std::string const& ip, int const& port) {
		auto&& addr = addrs.emplace_back();
		if (int r = FillAddress(ip, port, addr)) {
			addrs.pop_back();
			return r;
		}
		return 0;
	}

	inline int TcpDialer::Dial(int const& timeoutFrames) {
		Stop();
		SetTimeout(timeoutFrames);
		for (auto&& addr : addrs) {
			// 创建 tcp 非阻塞 socket fd
			auto&& fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (fd == -1) return -1;

			// 确保 return 时自动 close
			xx::ScopeGuard sg([&] { close(fd); });

			// 检测 fd 存储上限
			if (fd >= (int)ep->fdHandlers.size()) return -2;
			assert(!ep->fdHandlers[fd].first);

			// 设置一些 tcp 参数( 可选 )
			int on = 1;
			if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) return -3;

			// 开始连接
			if (connect(fd, (sockaddr*)&addr, addr.sin6_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN) == -1) {
				if (errno != EINPROGRESS) return -4;
			}
			// else : 立刻连接上了

			// 纳入 epoll 管理
			if (int r = ep->Ctl(fd, EPOLLIN | EPOLLOUT)) return r;

			// 确保 return 时自动 close 并脱离 epoll 管理
			sg.Set([&] { ep->CloseDel(fd); });

			// 试创建目标类实例
			auto o = std::make_unique<TcpConn>();
			auto op = o.get();

			// 继续初始化并放入容器
			o->ep = ep;
			o->indexAtContainer = fd;
			o->dialer = this;
			ep->fdHandlers[fd].first = std::move(o);		// 移动之后下面要访问要用 op
			ep->fdHandlers[fd].second = ++ep->autoIncVersion;

			conns.push_back(op);
			sg.Cancel();
		}
		return 0;
	}

	inline bool TcpDialer::Busy() {
		// 用超时检测判断是否正在拨号
		return timeoutIndex != -1;
	}

	inline void TcpDialer::Stop() {
		// 清理原先的残留
		for (auto&& conn : conns) {
			if (auto&& c = conn.Lock()) {
				c->Dispose();
			}
		}
		conns.clear();

		// 清除超时检测
		SetTimeout(0);
	}

	inline void TcpDialer::OnTimeout() {
		Stop();
		OnConnect(emptyPeer);
	}

	inline TcpDialer::~TcpDialer() {
		Stop();
	}

	inline void TcpDialer::Finish(int fd) {
		Stop();
		auto peer = OnCreatePeer();
		if (peer) {
			auto peer_p = peer.get();
			peer->ep = ep;
			peer->indexAtContainer = fd;
			ep->fdHandlers[fd].first = std::move(peer);
			ep->fdHandlers[fd].second = ++ep->autoIncVersion;

			// todo: fill peer->ip by tcp socket?
			OnConnect(peer_p);
		}
		else {
			ep->CloseDel(fd);
			OnConnect(emptyPeer);
		}
	}

	inline std::unique_ptr<TcpPeer> TcpDialer::OnCreatePeer() {
		return std::make_unique<TcpPeer>();
	}




	/***********************************************************************************************************/
	// UdpPeer
	/***********************************************************************************************************/

	inline void UdpPeer::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}
		// read
		if (e & EPOLLIN) {
			char buf[65536];
			sockaddr_in fromAddr;
			socklen_t addrLen = sizeof(fromAddr);
			auto len = recvfrom(indexAtContainer, buf, sizeof(buf), 0, (struct sockaddr*) & fromAddr, &addrLen);
			if (len < 0) {
				ep->lastErrorNumber = (int)len;
				Dispose();
				return;
			}
			OnReceive((sockaddr*)&fromAddr, buf, len);
		}
		// write:
		// udp 似乎不必关注 write 状态。无脑认为可写。写也不必判断是否成功。
	}

	inline void UdpPeer::OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len) {
		// echo. 忽略返回值
		(void)SendTo(fromAddr, buf, len);
	}

	inline int UdpPeer::SendTo(sockaddr* toAddr, char const* const& buf, std::size_t const& len) {
		return (int)sendto(indexAtContainer, buf, len, 0, toAddr, toAddr->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);
	}



	/***********************************************************************************************************/
	// UdpListener
	/***********************************************************************************************************/

	inline std::unique_ptr<KcpPeer> UdpListener::OnCreatePeer() {
		return xx::TryMakeU<KcpPeer>();
	}

	inline void UdpListener::OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len) {
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
			return;
		}

		// 忽略长度小于 kcp 头的数据包 ( IKCP_OVERHEAD at ikcp.c )
		if (len < 24) return;

		// read conv header
		uint32_t conv;
		memcpy(&conv, buf, sizeof(conv));
		KcpPeer* p = nullptr;

		// 根据 conv 试定位到 peer
		auto&& peerIter = ep->kcps.Find(conv);

		// 如果不存在 就在 shakes 中按 ip:port 找
		if (peerIter == -1) {
			auto&& idx = ep->shakes.Find(ipAndPort);

			// 未找到或 conv 对不上: 忽略
			if (idx == -1 || ep->shakes.ValueAt(idx).first != conv) return;

			// 从握手信息移除
			ep->shakes.RemoveAt(idx);

			// 始创建 peer
			auto peer = OnCreatePeer();
			if (!peer) return;
			p = peer.get();

			// 继续初始化
			peer->ep = ep;
			peer->owner = this;
			peer->conv = conv;
			peer->createMS = NowSteadyEpochMS();

			// 如果初始化 kcp 失败就忽略
			if (peer->InitKcp()) return;

			// 更新地址信息
			memcpy(&peer->addr, fromAddr, fromAddr->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);

			// 放入容器
			p->indexAtContainer = ep->items.Add(std::move(peer));	// 移动. 这之后只能用 p
			ep->kcps.Add(conv, p);

			// 触发事件回调
			OnAccept(p);
		}
		else {
			// 定位到目标 peer
			p = ep->kcps.ValueAt(peerIter);

			// 更新地址信息
			memcpy(&p->addr, fromAddr, fromAddr->sa_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);
		}

		// 将数据灌入 kcp. 进而可能触发 peer->OnReceive
		if (p->Input((uint8_t*)buf, (uint32_t)len)) {
			p->Dispose();	// 之后没代码了, 故安全
		}
	}




	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

	inline TimeoutManager* KcpPeer::GetTimeoutManager() {
		return ep;
	}

	inline KcpPeer::~KcpPeer() {
		SetTimeout(0);
		if (kcp) {
			ikcp_release(kcp);
			kcp = nullptr;
		}
		if (conv) {
			ep->kcps.Remove(conv);
		}
	};

	inline void KcpPeer::OnReceive() {
		(void)Send(recv.buf, recv.len);
		recv.Clear();
	}

	inline int KcpPeer::InitKcp() {
		assert(!kcp);
		// 创建并设置 kcp 的一些参数
		kcp = ikcp_create(conv, this);
		if (!kcp) return -1;
		(void)ikcp_wndsize(kcp, 1024, 1024);
		(void)ikcp_nodelay(kcp, 1, 10, 2, 1);
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

	inline void KcpPeer::UpdateKcpLogic(int64_t const& nowMS) {
		assert(kcp);
		// 计算出当前 ms
		// 已知问题: 受 ikcp uint32 限制, 连接最多保持 50 多天
		auto&& currentMS = uint32_t(nowMS - createMS);

		// 如果 update 时间没到 就退出
		if (nextUpdateMS > currentMS) return;

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
			if (recv.len == recv.cap) {
				OnDisconnect(-1);
				Dispose();
				return;
			}

			// 从 kcp 提取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则下次再说
			auto&& len = ikcp_recv(kcp, (char*)recv.buf + recv.len, (int)(recv.cap - recv.len));
			if (len <= 0) break;

			// 调用用户数据处理函数
			Item_r<KcpPeer> alive(this);
			OnReceive();
			if (!alive) return;
		} while (true);
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
		OnDisconnect(-2);
		Dispose();
	}


	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	inline Context::Context(int const& maxNumFD, std::size_t const& wheelLen)
		: TimeoutManager(wheelLen) {
		assert(maxNumFD >= 1024);

		// 创建 epoll fd
		efd = epoll_create1(0);
		if (-1 == efd) throw - 1;
	}

	inline Context::~Context() {
		// 所有 fd 掐线
		for (auto&& o : fdHandlers) {
			if (o.first) {
				o.first->Dispose();
			}
		}

		// 所有 items 析构
		items.Clear();

		// 关闭 epoll 本身 fd
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
		assert(!fdHandlers[fd].first);

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
			auto&& h = fdHandlers[events[i].data.fd].first;
			assert(h);
			auto e = events[i].events;
			h->OnEpollEvent(e);
		}
		return 0;
	}

	inline void Context::UpdateKcps() {
		auto nowMS = xx::NowSteadyEpochMS();
		for (auto&& data : kcps) {
			data.value->UpdateKcpLogic(nowMS);
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
				UpdateTimeoutWheel();

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
	inline Item_r<L> Context::CreateTcpListener(int const& port, Args&&... args) {
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
		auto o = xx::TryMakeU<L>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -5;
			return nullptr;
		}
		auto op = o.get();

		// 继续初始化并放入容器
		o->ep = this;
		o->indexAtContainer = fd;
		fdHandlers[fd].first = std::move(o);	// 移动后将不可用，故用 op
		fdHandlers[fd].second = ++autoIncVersion;

		// 撤销自动关闭行为并返回结果
		sg.Cancel();
		return op;
	}


	// 创建 拨号器
	template<typename TD, typename ...Args>
	inline Item_r<TD> Context::CreateTcpDialer(Args&&... args) {
		static_assert(std::is_base_of_v<TcpDialer, TD>);

		// 试创建目标类实例
		auto o = xx::TryMakeU<TD>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -1;
			return nullptr;
		}
		auto op = o.get();

		// 继续初始化并放入容器
		o->ep = this;
		o->indexOfContainer = items.Add(std::move(o));	// 移动后将不可用，故用 op

		// 调用用户自定义后续初始化
		return op;
	}


	// 创建 timer
	template<typename T, typename ...Args>
	inline Item_r<T> Context::CreateTimer(int const& interval, std::function<void(Timer* const& timer)>&& cb, Args&&...args) {
		static_assert(std::is_base_of_v<Timer, T>);

		// 试创建目标类实例
		auto o = std::unique_ptr<T>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -1;
			return nullptr;
		}
		auto op = o.get();

		// 设置超时时长
		o->ep = this;
		if (o->SetTimeout(interval)) {
			lastErrorNumber = -2;
			return nullptr;
		}

		// 继续初始化并放入容器
		o->onFire = std::move(cb);
		op->indexAtContainer = items.Add(std::move(o));	// 移动后将不可用，故用 op
		return Item_r<T>(op);
	}



	// 创建 UdpPeer. port 传 0 则自适应( 仅用于发数据 )
	template<typename U, typename ...Args>
	inline Item_r<U> Context::CreateUdpPeer(int const& port, Args&&... args) {
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
		auto o = xx::TryMakeU<U>(std::forward<Args>(args)...);
		if (!o) {
			lastErrorNumber = -5;
			return nullptr;
		}
		auto op = o.get();

		// 继续初始化并放入容器
		o->ep = this;
		o->port = port;
		o->indexAtContainer = fd;
		fdHandlers[fd].first = std::move(o);	// 移动后将不可用，故用 op
		fdHandlers[fd].second = ++autoIncVersion;

		// 撤销自动关闭行为并返回结果
		sg.Cancel();
		return op;
	}



	inline int FillAddress(std::string const& ip, int const& port, sockaddr_in6& addr) {
		memset(&addr, 0, sizeof(addr));

		if (ip.find(':') == std::string::npos) {		// ipv4
			auto a = (sockaddr_in*)&addr;
			a->sin_family = AF_INET;
			a->sin_port = htons((uint16_t)port);
			if (!inet_pton(AF_INET, ip.c_str(), &a->sin_addr)) return -1;
		}
		else {											// ipv6
			auto a = &addr;
			a->sin6_family = AF_INET6;
			a->sin6_port = htons((uint16_t)port);
			if (!inet_pton(AF_INET6, ip.c_str(), &a->sin6_addr)) return -1;
		}

		return 0;
	}

}
