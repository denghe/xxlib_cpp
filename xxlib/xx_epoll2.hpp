#pragma once
#include "xx_epoll2.h"

namespace xx::Epoll {

	/***********************************************************************************************************/
	// Item
	/***********************************************************************************************************/

	inline void Item::Dispose() {
		if (indexAtContainer != -1) {
			// 因为 gcc 傻逼，此处由于要自杀，故先复制参数到栈，避免出异常
			auto ep = this->ep;
			auto indexAtContainer = this->indexAtContainer;
			ep->items.RemoveAt(indexAtContainer);	// 触发析构
		}
	}

	inline Item::~Item() {
		if (fd != -1) {
			assert(ep);
			ep->fdMappings[fd] = nullptr;
			ep->CloseDel(fd);
			fd = -1;
		}
	}

	/***********************************************************************************************************/
	// Ref
	/***********************************************************************************************************/

	template<typename T>
	inline Ref<T>::Ref(T* const& ptr) {
		static_assert(std::is_base_of_v<Item, T>);
		Reset(ptr);
	}

	template<typename T>
	inline Ref<T>::Ref(std::unique_ptr<T> const& ptr) : Ref(ptr.get()) {}


	template<typename T>
	template<typename U>
	Ref<T>& Ref<T>::operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>> const& o) {
		return operator=(*(Ref<T>*) & o);
	}

	template<typename T>
	template<typename U>
	Ref<T>& Ref<T>::operator=(std::enable_if_t<std::is_base_of_v<T, U>, Ref<U>>&& o) {
		return operator=(std::move(*(Ref<T>*) & o));
	}


	template<typename T>
	template<typename U>
	inline Ref<U> Ref<T>::As() const {
		if (!dynamic_cast<U*>(Lock())) return Ref<U>();
		return *(Ref<U>*)this;
	}


	template<typename T>
	inline Ref<T>::operator bool() const {
		return version && items->VersionAt(index) == version;
	}

	template<typename T>
	inline T* Ref<T>::operator->() const {
		if (!operator bool()) throw - 1;		// 空指针
		return (T*)items->ValueAt(index).get();
	}

	template<typename T>
	inline T* Ref<T>::Lock() const {
		return operator bool() ? (T*)items->ValueAt(index).get() : nullptr;
	}

	template<typename T>
	template<typename U>
	inline void Ref<T>::Reset(U* const& ptr) {
		static_assert(std::is_base_of_v<T, U>);
		if (!ptr) {
			items = nullptr;
			index = -1;
			version = 0;
		}
		else {
			assert(ptr->ep);
			assert(ptr->indexAtContainer != -1);
			items = &ptr->ep->items;
			index = ptr->indexAtContainer;
			version = items->VersionAt(index);
		}
	}


	/***********************************************************************************************************/
	// ItemTimeout
	/***********************************************************************************************************/

	// 返回非 0 表示找不到 管理器 或 参数错误
	inline int ItemTimeout::SetTimeout(int const& interval) {
		// 试着从 wheel 链表中移除
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				ep->wheel[timeoutIndex] = timeoutNext;
			}
		}

		// 检查是否传入间隔时间
		if (interval) {
			// 如果设置了新的超时时间, 则放入相应的链表
			// 安全检查
			if (interval < 0 || interval >(int)ep->wheel.size()) return -2;

			// 环形定位到 wheel 元素目标链表下标
			timeoutIndex = (interval + ep->cursor) & ((int)ep->wheel.size() - 1);

			// 成为链表头
			timeoutPrev = nullptr;
			timeoutNext = ep->wheel[timeoutIndex];
			ep->wheel[timeoutIndex] = this;

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

	inline ItemTimeout::~ItemTimeout() {
		if (timeoutIndex != -1) {
			SetTimeout(0);
		}
	}

	/***********************************************************************************************************/
	// Timer
	/***********************************************************************************************************/

	inline void Timer::OnTimeout() {
		if (onFire) {
			Ref<Timer> alive(this);	// 防止在 onFire 中 Dispose timer
			onFire(this);
			if (!alive) return;
		}
		// 非 repeat 模式( 未再次 SetTimeout )直接自杀
		if (timeoutIndex == -1) {
			Dispose();
		}
	}


	/***********************************************************************************************************/
	// Peer
	/***********************************************************************************************************/

	inline void Peer::OnDisconnect(int const& reason) {}

	inline void Peer::OnReceive() {
		if (Send(recv.buf, recv.len)) {
			OnDisconnect(-3);
			Dispose();
		}
		else {
			recv.Clear();
		}
	}

	inline void Peer::OnTimeout() {
		Ref<Peer> alive(this);
		OnDisconnect(-4);
		if (alive) {
			Dispose();
		}
	}


	/***********************************************************************************************************/
	// TcpPeer
	/***********************************************************************************************************/

	inline void TcpPeer::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Ref<TcpPeer> alive(this);
			OnDisconnect(-1);
			if (alive) {
				Dispose();
			}
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
				Ref<TcpPeer> alive(this);
				OnDisconnect(-2);
				if (alive) {
					Dispose();
				}
				return;
			}

			// 通过 fd 从系统网络缓冲区读取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则认为失败 自杀
			auto&& len = read(fd, recv.buf + recv.len, recv.cap - recv.len);
			if (len <= 0) {
				Ref<TcpPeer> alive(this);
				OnDisconnect(-3);
				if (alive) {
					Dispose();
				}
				return;
			}
			recv.len += len;

			// 调用用户数据处理函数
			{
				Ref<TcpPeer> alive(this);
				OnReceive();
				if (!alive) return;
			}
		}
		// write
		if (e & EPOLLOUT) {
			// 设置为可写状态
			writing = false;
			if (int r = Write()) {
				ep->lastErrorNumber = r;
				Ref<TcpPeer> alive(this);
				OnDisconnect(-4);
				if (alive) {
					Dispose();
				}
				return;
			}
		}
	}

	inline int TcpPeer::Send(char const* const& buf, size_t const& len) {
		sendQueue.Push(xx::Buf(buf, len));
		return !writing ? Write() : 0;
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
		else if ((size_t)sentLen == bufLen) {
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



	/***********************************************************************************************************/
	// TcpListener
	/***********************************************************************************************************/

	inline TcpPeer_u TcpListener::OnCreatePeer() {
		return xx::TryMakeU<TcpPeer>();
	}

	inline void TcpListener::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}
		// accept 到 没有 或 出错 为止
		while (Accept(fd) > 0) {};
	}

	inline int TcpListener::Accept(int const& listenFD) {
		// 开始创建 fd
		sockaddr_in6 addr;
		socklen_t len = sizeof(addr);

		// 接收并得到目标 fd
		int fd = accept(listenFD, (sockaddr*)&addr, &len);
		if (-1 == fd) {
			ep->lastErrorNumber = errno;
			if (ep->lastErrorNumber == EAGAIN || ep->lastErrorNumber == EWOULDBLOCK) return 0;
			else return -1;
		}

		// 确保退出时自动关闭 fd
		xx::ScopeGuard sg([&] { close(fd); });

		// 如果 fd 超出最大存储限制就退出。返回 fd 是为了外部能继续执行 accept
		if (fd >= (int)ep->fdMappings.size()) return fd;
		assert(!ep->fdMappings[fd]);

		// 设置非阻塞状态
		if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;

		// 设置一些 tcp 参数( 可选 )
		int on = 1;
		if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) return -3;

		// 纳入 ep 管理
		if (-1 == ep->Ctl(fd, EPOLLIN)) return -4;

		// 确保退出时关闭 fd 并从 epoll 监视移除
		sg.Set([&] { ep->CloseDel(fd); });


		// 创建类容器. 允许创建失败( 比如内存不足，或者刻意控制数量 ). 失败不触发 OnAccept
		auto u = OnCreatePeer();
		if (!u) return fd;

		// 放入容器并拿到指针用, 继续填充
		auto p = ep->AddItem(std::move(u), fd);

		// 填充 ip
		memcpy(&p->addr, &addr, len);

		// 撤销自动关闭
		sg.Cancel();

		// 初始化
		Ref<TcpPeer> alive(p);
		p->Init();
		if (!alive) return fd;

		// 触发事件
		OnAccept(p);
		return fd;
	}



	/***********************************************************************************************************/
	// TcpConn
	/***********************************************************************************************************/

	inline void TcpConn::OnEpollEvent(uint32_t const& e) {
		// 如果 dialer 无法定位到 或 error 事件则自杀
		auto d = this->dialer.Lock();
		if (!d || e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}
		// 读取错误 或者读到错误 都认为是连接失败. 返回非 0 触发 Dispose
		int err;
		socklen_t result_len = sizeof(err);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &result_len) == -1 || err) {
			Dispose();
			return;
		}

		// 连接成功. 自杀前备份变量到栈
		auto fd = this->fd;
		auto ep = this->ep;
		// 设为 -1 以绕开析构函数中的 close
		this->fd = -1;
		// 清除映射关系
		ep->fdMappings[fd] = nullptr;
		Dispose();	// 自杀

		// 这之后只能用 "栈"变量
		d->Stop();
		auto peer = d->OnCreatePeer(Protocol::Tcp);
		if (peer) {
			auto p = ep->AddItem(std::move(peer), fd);	// peer is moved
			// fill address
			result_len = sizeof(p->addr);
			getpeername(fd, (sockaddr*)&p->addr, &result_len);
			d->OnConnect(p);
		}
		else {
			ep->CloseDel(fd);
			d->OnConnect(d->emptyPeer);
		}
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
			// 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
			if (!recv.cap) {
				recv.Reserve(readBufLen);
			}
			socklen_t addrLen = sizeof(addr);
			auto len = recvfrom(fd, recv.buf, recv.cap, 0, (struct sockaddr*) & addr, &addrLen);
			if (len < 0) {
				ep->lastErrorNumber = (int)len;
				Dispose();
				return;
			}
			if (!len) return;
			recv.len = len;
			OnReceive();
		}
		// write: 当前 udp 不启用发送队列
	}

	inline int UdpPeer::Send(xx::Buf&& data) {
		auto r = sendto(fd, data.buf, data.len, 0, (sockaddr*)&addr, sizeof(addr));
		return r > 0 ? 0 : (int)r;
	}

	inline int UdpPeer::Send(char const* const& buf, size_t const& len) {
		auto r = sendto(fd, buf, len, 0, (sockaddr*)&addr, sizeof(addr));
		return r > 0 ? 0 : (int)r;
	}

	inline int UdpPeer::Flush() {
		return 0;
	}



	/***********************************************************************************************************/
	// KcpListener
	/***********************************************************************************************************/

	inline void KcpListener::Init() {
		SetTimeout(1);
	}

	inline KcpPeer_u KcpListener::OnCreatePeer() {
		return xx::TryMakeU<KcpPeer>();
	}

	inline void KcpListener::OnTimeout() {
		// 更新所有 kcps
		for (auto&& data : kcps) {
			data.value->UpdateKcpLogic();
		}

		// 清理超时握手信息
		for (auto&& iter = shakes.begin(); iter != shakes.end(); ++iter) {
			if (iter->value.second < ep->nowMS) {
				iter.Remove();
			}
		}

		// 下帧继续触发
		SetTimeout(1);
	}

	// 清除 kcps
	inline KcpListener::~KcpListener() {
		for (auto&& data : kcps) {
			data.value->Dispose();
		}
	}

	inline void KcpListener::OnReceive() {
		// 确保退出时清掉 已收数据
		xx::ScopeGuard sgRecvClear([this] { recv.Clear(); });

		// sockaddr* 转为 ip:port
		std::string ip_port;
		xx::Append(ip_port, addr);

		// 当前握手方案为 UdpDialer 每秒 N 次不停发送 4 字节数据( serial )过来, 
		// 收到后根据其 ip:port 做 key, 生成 convId
		// 每次收到都向其发送 convId
		if (recv.len == 4) {
			auto&& idx = shakes.Find(ip_port);
			if (idx == -1) {
				idx = shakes.Add(ip_port, std::make_pair(++convId, ep->nowMS + 3000)).index;
			}
			char tmp[8];	// serial + convId
			memcpy(tmp, recv.buf, 4);
			memcpy(tmp + 4, &shakes.ValueAt(idx).first, 4);
			Send(tmp, 8);
			return;
		}

		// 忽略长度小于 kcp 头的数据包 ( IKCP_OVERHEAD at ikcp.c )
		if (recv.len < 24) return;

		// read conv header
		uint32_t conv;
		memcpy(&conv, recv.buf, sizeof(conv));

		// 准备创建或找回 KcpPeer
		KcpPeer* p = nullptr;

		// 根据 conv 试定位到 peer
		auto&& peerIter = kcps.Find(conv);

		// 如果不存在 就在 shakes 中按 ip:port 找
		if (peerIter == -1) {
			auto&& idx = shakes.Find(ip_port);

			// 未找到或 conv 对不上: 忽略
			if (idx == -1 || shakes.ValueAt(idx).first != conv) return;

			// 从握手信息移除
			shakes.RemoveAt(idx);

			// 始创建 peer
			auto peer = OnCreatePeer();
			if (!peer) return;

			// 继续初始化
			peer->ep = ep;
			peer->owner = this;
			peer->conv = conv;
			peer->createMS = ep->nowMS;

			// 如果初始化 kcp 失败就忽略
			if (peer->InitKcp()) return;

			// 更新地址信息
			memcpy(&peer->addr, &addr, sizeof(addr));

			// 放入容器
			p = ep->AddItem(std::move(peer));
			kcps.Add(conv, p);

			// 初始化
			Ref<KcpPeer> alive(p);
			p->Init();
			if (!alive) return;

			// 触发事件回调
			OnAccept(p);
			if (!alive) return;
		}
		else {
			// 定位到目标 peer
			p = kcps.ValueAt(peerIter);

			// 更新地址信息
			memcpy(&p->addr, &addr, sizeof(addr));
		}

		// 将数据灌入 kcp. 进而可能触发 peer->OnReceive 进而 Dispose
		p->Input(recv.buf, (uint32_t)recv.len);

		recv.Clear();
	}



	/***********************************************************************************************************/
	// KcpPeer
	/***********************************************************************************************************/

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
			if (auto o = self->owner.Lock()) {
				o->addr = self->addr;
				return o->Send(inBuf, len);
			}
			return -1;
			});
		return 0;
	}

	inline void KcpPeer::UpdateKcpLogic() {
		assert(kcp);
		// 计算出当前 ms
		// 已知问题: 受 ikcp uint32 限制, 连接最多保持 50 多天
		auto&& currentMS = uint32_t(ep->nowMS - createMS);

		// 如果 update 时间没到 就退出
		if (nextUpdateMS > currentMS) return;

		// 来一发
		ikcp_update(kcp, currentMS);

		// 更新下次 update 时间
		nextUpdateMS = ikcp_check(kcp, currentMS);
	}

	inline void KcpPeer::Input(char* const& buf, size_t const& len) {
		if (ikcp_input(kcp, buf, len)) {
			Dispose();
			return;
		}

		// 开始处理收到的数据
		do {
			// 如果接收缓存没容量就扩容( 通常发生在首次使用时 )
			if (!recv.cap) {
				recv.Reserve(readBufLen);
			}
			// 如果数据长度 == buf限长 就自杀( 未处理数据累计太多? )
			if (recv.len == recv.cap) {
				Ref<KcpPeer> alive(this);
				OnDisconnect(-1);
				if (alive) {
					Dispose();
				}
				return;
			}

			// 从 kcp 提取数据. 追加填充到 recv 后面区域. 返回填充长度. <= 0 则下次再说
			auto&& len = ikcp_recv(kcp, recv.buf + recv.len, (int)(recv.cap - recv.len));
			if (len <= 0) break;
			recv.len += len;

			// 调用用户数据处理函数
			{
				Ref<KcpPeer> alive(this);
				OnReceive();
				if (!alive) return;
			}
		} while (true);
	}

	inline KcpPeer::~KcpPeer() {
		// 回收 kcp 相关
		if (kcp) {
			ikcp_release(kcp);
			kcp = nullptr;
		}
		// 看看 owner 是不是 conn 连接对象。如果是 则 kcp peer 死亡时必须回收它. 同时从 ep->kcps 移除
		// 如果是 KcpListener, 则从 listener->kcps 根据 conv Remove( this )
		if (auto o = owner.Lock()) {
			if (dynamic_cast<KcpConn*>(o)) {
				o->Dispose();
				assert(indexAtKcps != -1);
				XX_SWAP_REMOVE(this, indexAtKcps, ep->kcps);
			}
			else {
				auto listener = dynamic_cast<KcpListener*>(o);
				assert(listener);
				assert(conv);
				listener->kcps.Remove(conv);
			}
		}
	};

	inline int KcpPeer::Send(xx::Buf&& data) {
		return ikcp_send(kcp, (char*)data.buf, (int)data.len);
	}

	inline int KcpPeer::Send(char const* const& buf, size_t const& len) {
		return ikcp_send(kcp, (char*)buf, (int)len);
	}

	inline int KcpPeer::Flush() {
		ikcp_flush(kcp);
		return 0;
	}



	/***********************************************************************************************************/
	// KcpConn
	/***********************************************************************************************************/

	inline void KcpConn::OnTimeout() {
		(void)Send((char*)&serial, 4);
		SetTimeout(ep->SecToFrames(0.2));	// 按 0.2 秒间隔 repeat ( 可能受 cpu 占用影响而剧烈波动 )
	}

	inline void KcpConn::Init() {
		// 递增版本号
		serial = ++ep->autoIncKcpSerial;

		// 发包并激活 timer
		OnTimeout();
	}

	inline void KcpConn::OnReceive() {
		// 拨号状态
		if (!peer && recv.len == 8) {		// 4 bytes serial + 4 bytes conv
			// 序列号对不上：忽略
			if (memcmp(recv.buf, &serial, 4)) {
				recv.Clear();
				return;
			}

			// 连接成功: 将自己从 dialer->conns 移除，创建 KcpPeer，将自己填入 owner, 最后 OnConnect 回调

			// 提取 server 分配的 conv 值 到栈
			uint32_t conv = 0;
			memcpy(&conv, recv.buf + 4, 4);
			recv.Clear();

			// 提取 dialer 到栈( dialer 应该比 conn 活得久 )
			auto d = dialer.Lock();
			assert(d);

			// 从 d->conns 找出自己并移除
			for (int i = 0, n = (int)d->conns.size(); i < n; ++i) {
				if (d->conns[i].index == indexAtContainer) {
					if (i > n - 1) {
						std::swap(d->conns[i], d->conns[n - 1]);
					}
					d->conns.resize(n - 1);
					break;
				}
			}

			// 停止所有拨号行为并清空
			d->Stop();

			// 开始创建 peer
			auto u = d->OnCreatePeer(Protocol::Kcp);
			if (!u) return;
			auto p = dynamic_cast<KcpPeer*>(u.get());
			if (!p) return;

			// 继续初始化 kcp
			p->conv = conv;
			p->createMS = ep->nowMS;
			if (p->InitKcp()) return;

			// 继续填充
			memcpy(&p->addr, &addr, sizeof(addr));

			// 和 kcppeer 双向绑定
			this->peer = p;
			p->owner = this;

			// 放入容器
			ep->AddItem(std::move(u));
			p->indexAtKcps = (int)ep->kcps.size();
			ep->kcps.push_back(p);

			// 初始化
			Ref<KcpConn> alive(this);
			p->Init();
			if (!alive) return;

			//// 通过 kcp 发个包触发 accept
			//(void)p->Send("\x1\0\0\0\0", 5);
			//p->Flush();

			// 触发事件回调
			d->OnConnect(p);
			return;
		}

		// >24: kcp data. 如果拨号未完成则不应该收到
		if (recv.len < 24 || !peer) return;

		// 提取 conv. 如果对不上则 忽略
		uint32_t conv;
		memcpy(&conv, recv.buf, sizeof(conv));
		if (peer->conv != conv) return;

		// 更新地址
		memcpy(&peer->addr, &addr, sizeof(addr));

		// 向 kcppeer 提供数据. 可能导致 this Dispose
		{
			Ref<KcpConn> alive(this);
			peer->Input(recv.buf, recv.len);
			if (alive) {
				recv.Clear();
			}
		}
	}



	/***********************************************************************************************************/
	// Dialer
	/***********************************************************************************************************/

	inline int Dialer::AddAddress(std::string const& ip, int const& port, Protocol const& protocol) {
		auto&& a = addrs.emplace_back();
		if (int r = FillAddress(ip, port, a.first)) {
			addrs.pop_back();
			return r;
		}
		a.second = protocol;
		return 0;
	}

	inline int Dialer::Dial(int const& timeoutFrames) {
		Stop();
		SetTimeout(timeoutFrames);
		for (auto&& a : addrs) {
			int r = 0;
			if (a.second == Protocol::Tcp || a.second == Protocol::Both) {
				r = NewTcpConn(a.first);
			}
			if (r) {
				Stop();
				return r;
			}
			if (a.second == Protocol::Kcp || a.second == Protocol::Both) {
				r = NewKcpConn(a.first);
			}
			if (r) {
				Stop();
				return r;
			}
		}
		return 0;
	}

	inline bool Dialer::Busy() {
		// 用超时检测判断是否正在拨号
		return timeoutIndex != -1;
	}

	inline void Dialer::Stop() {
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

	inline void Dialer::OnTimeout() {
		Stop();
		OnConnect(emptyPeer);
	}

	inline Dialer::~Dialer() {
		Stop();
	}

	inline Peer_u Dialer::OnCreatePeer(Protocol const& protocol) {
		if (protocol == Protocol::Tcp) {
			return xx::TryMakeU<TcpPeer>();
		}
		else {
			return xx::TryMakeU<KcpPeer>();
		}
	}

	inline int Dialer::NewTcpConn(sockaddr_in6 const& addr) {
		// 创建 tcp 非阻塞 socket fd
		auto&& fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		if (fd == -1) return -1;

		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// 检测 fd 存储上限
		if (fd >= (int)ep->fdMappings.size()) return -2;
		assert(!ep->fdMappings[fd]);

		// 设置一些 tcp 参数( 可选 )
		int on = 1;
		if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on))) return -3;

		// 开始连接
		if (connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
			if (errno != EINPROGRESS) return -4;
		}
		// else : 立刻连接上了

		// 纳入 epoll 管理
		if (int r = ep->Ctl(fd, EPOLLIN | EPOLLOUT)) return r;

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { ep->CloseDel(fd); });

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<TcpConn>();
		if (!o_) return -5;

		// 继续初始化并放入容器
		auto o = ep->AddItem(std::move(o_), fd);
		o->dialer = this;
		conns.push_back(o);

		sg.Cancel();
		o->Init();
		return 0;
	}

	inline int Dialer::NewKcpConn(sockaddr_in6 const& addr) {
		// 创建 udp socket fd
		auto&& fd = ep->MakeSocketFD(0, SOCK_DGRAM);
		if (fd < 0) return fd;

		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// fd 纳入 epoll 管理
		if (-1 == ep->Ctl(fd, EPOLLIN)) return -4;

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { ep->CloseDel(fd); });

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<KcpConn>();
		if (!o_) return -5;

		// 继续初始化并放入容器
		auto o = ep->AddItem(std::move(o_), fd);
		o->dialer = this;
		o->addr = addr;
		conns.push_back(o);

		sg.Cancel();
		o->Init();
		return 0;
	}


	/***********************************************************************************************************/
	// CommandHandler
	/***********************************************************************************************************/

	inline CommandHandler::CommandHandler() {
		// 初始化 readline 第三方组件. 通过静态函数 访问静态 self 类 从而调用类成员函数
		self = this;
		rl_attempted_completion_function = (rl_completion_func_t*)&CompleteCallback;
		rl_callback_handler_install("# ", (rl_vcpfunc_t*)&ReadLineCallback);
	}

	inline void CommandHandler::Exec(char const* const& row, size_t const& len) {
		// 读取 row 内容, call ep->cmds[ args[0] ]( args )
		auto&& args = ep->args;
		args.clear();
		std::string s;
		bool jumpSpace = true;
		for (size_t i = 0; i < len; ++i) {
			auto c = row[i];
			if (jumpSpace) {
				if (c != '	' && c != ' '/* && c != 10*/) {
					s += c;
					jumpSpace = false;
				}
				continue;
			}
			else if (c == '	' || c == ' '/* || c == 10*/) {
				args.emplace_back(std::move(s));
				jumpSpace = true;
				continue;
			}
			else {
				s += c;
			}
		}
		if (s.size()) {
			args.emplace_back(std::move(s));
		}

		if (args.size()) {
			auto&& iter = ep->cmds.find(args[0]);
			if (iter != ep->cmds.end()) {
				if (iter->second) {
					iter->second(args);
				}
			}
			else {
				xx::CoutN("unknown command: ", args[0]);
			}
		}
	}

	inline void CommandHandler::ReadLineCallback(char* line) {
		if (!line) return;
		auto len = strlen(line);
		if (len) {
			add_history(line);
		}
		self->Exec(line, len);
		free(line);
	}

	inline char* CommandHandler::CompleteGenerate(const char* text, int state) {
		// This function is called with state=0 the first time; subsequent calls are
		  // with a nonzero state. state=0 can be used to perform one-time
		  // initialization for this completion session.
		static std::vector<std::string> matches;
		static size_t match_index = 0;

		if (state == 0) {
			// During initialization, compute the actual matches for 'text' and keep
			// them in a static vector.
			matches.clear();
			match_index = 0;

			// Collect a vector of matches: vocabulary words that begin with text.
			std::string textstr = std::string(text);
			for (auto&& kv : self->ep->cmds) {
				auto&& word = kv.first;
				if (word.size() >= textstr.size() &&
					word.compare(0, textstr.size(), textstr) == 0) {
					matches.push_back(word);
				}
			}
		}

		if (match_index >= matches.size()) {
			// We return nullptr to notify the caller no more matches are available.
			return nullptr;
		}
		else {
			// Return a malloc'd char* for the match. The caller frees it.
			return strdup(matches[match_index++].c_str());
		}
	}

	inline char** CommandHandler::CompleteCallback(const char* text, int start, int end) {
		// Don't do filename completion even if our generator finds no matches.
		rl_attempted_completion_over = 1;

		// Note: returning nullptr here will make readline use the default filename
		// completer.
		return rl_completion_matches(text, (rl_compentry_func_t*)&CompleteGenerate);
	}

	inline void CommandHandler::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}

		rl_callback_read_char();
	}

	inline CommandHandler::~CommandHandler() {
		rl_callback_handler_remove();
		rl_clear_history();

		epoll_ctl(ep->efd, EPOLL_CTL_DEL, fd, nullptr);
		ep->fdMappings[fd] = nullptr;
		fd = -1;
	}


	/***********************************************************************************************************/
	// INotifyHandler
	/***********************************************************************************************************/

	inline void INotifyHandler::OnEvent(char const* const& fn, uint32_t const& mask) {
		if (onEvent) {
			onEvent(fn, mask);
		}
	}

	inline void INotifyHandler::OnEpollEvent(uint32_t const& e) {
		// error
		if (e & EPOLLERR || e & EPOLLHUP) {
			Dispose();
			return;
		}

		Ref<Item> alive(this);
		buf[sizeof(buf) - 1] = 0;
		ssize_t len;
		while ((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
			ssize_t nread = 0;
			while (len - nread > 0) {
				auto&& event = (inotify_event*)&buf[nread];
				//for (int i = 0; i < 12; i++) {
				//	if ((event->mask >> i) & 1) {
						OnEvent(event->len ? event->name : nullptr, event->mask);
						if (!alive) return;
					//}
				//}
				nread = nread + sizeof(inotify_event) + event->len;
			}
		}
	}



	/***********************************************************************************************************/
	// Context
	/***********************************************************************************************************/

	inline Context::Context(size_t const& wheelLen) {
		// 创建 epoll fd
		efd = epoll_create1(0);
		if (-1 == efd) throw - 1;

		// 初始化时间伦
		wheel.resize(wheelLen);

		// 初始化处理类映射表
		fdMappings.fill(nullptr);
	}

	inline Context::~Context() {
		// 直接清掉内存. 非正常析构.
		recvBB.Reset();

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

		addrinfo hints;
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
		if (fd >= (int)fdMappings.size()) {
			close(fd);
			return -3;
		}
		assert(!fdMappings[fd]);

		return fd;
	}

	inline int Context::Ctl(int const& fd, uint32_t const& flags, int const& op) {
		epoll_event event;
		bzero(&event, sizeof(event));
		event.data.fd = fd;
		event.events = flags;
		return epoll_ctl(efd, op, fd, &event);
	};

	inline int Context::CloseDel(int const& fd) {
		assert(fd != -1);
		epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
		return close(fd);
	}

	inline int Context::Wait(int const& timeoutMS) {
		int n = epoll_wait(efd, events.data(), (int)events.size(), timeoutMS);
		if (n == -1) return errno;
		for (int i = 0; i < n; ++i) {
			auto fd = events[i].data.fd;
			auto h = fdMappings[fd];
			if (!h) {
				xx::CoutN("epoll wait !h. fd = ", fd);
				assert(h);
			}
			assert(!h->fd || h->fd == fd);
			auto e = events[i].events;
			h->OnEpollEvent(e);
		}
		return 0;
	}

	inline void Context::UpdateTimeoutWheel() {
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

	inline void Context::UpdateKcps() {
		if (kcps.size()) {
			// 倒扫方便自杀交焕删除
			for (int i = (int)kcps.size() - 1; i >= 0; --i) {
				kcps[i]->UpdateKcpLogic();
			}
		}
	}

	inline int Context::Run(double const& frameRate) {
		assert(frameRate > 0);
		this->frameRate = frameRate;

		// 稳定帧回调用的时间池
		double ticksPool = 0;

		// 本次要 Wait 的超时时长
		int waitMS = 0;

		// 计算帧时间间隔
		auto ticksPerFrame = 10000000.0 / frameRate;

		// 取当前时间
		auto lastTicks = xx::NowEpoch10m();

		// 更新一下逻辑可能用到的时间戳
		nowMS = xx::NowSteadyEpochMS();

		// 开始循环
		while (running) {

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

				// 更新一下逻辑可能用到的时间戳
				nowMS = xx::NowSteadyEpochMS();

				// 驱动 timerswfffffff
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


	template<typename L, typename ...Args>
	inline Ref<L> Context::CreateTcpListener(int const& port, Args&&... args) {
		static_assert(std::is_base_of_v<TcpListener, L>);

		// 创建监听用 socket fd
		auto&& fd = MakeSocketFD(port);
		if (fd < 0) {
			lastErrorNumber = -1;
			return Ref<L>();
		}
		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// 开始监听
		if (-1 == listen(fd, SOMAXCONN)) {
			lastErrorNumber = -3;
			return Ref<L>();
		}

		// fd 纳入 epoll 管理
		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -4;
			return Ref<L>();
		}

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { CloseDel(fd); });

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<L>(std::forward<Args>(args)...);
		if (!o_) {
			lastErrorNumber = -5;
			return Ref<L>();
		}

		// 撤销自动关闭行为并返回结果
		sg.Cancel();

		// 放入容器, 初始化并返回
		auto o = AddItem(std::move(o_), fd);
		o->Init();
		return o;
	}


	template<typename TD, typename ...Args>
	inline Ref<TD> Context::CreateDialer(Args&&... args) {
		static_assert(std::is_base_of_v<Dialer, TD>);

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<TD>(std::forward<Args>(args)...);
		if (!o_) {
			lastErrorNumber = -1;
			return Ref<TD>();
		}

		// 放入容器, 初始化并返回
		auto o = AddItem(std::move(o_));
		o->Init();
		return o;
	}


	template<typename T, typename ...Args>
	inline Ref<T> Context::CreateTimer(int const& interval, std::function<void(Timer_r const& timer)>&& cb, Args&&...args) {
		static_assert(std::is_base_of_v<Timer, T>);

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<T>(std::forward<Args>(args)...);
		if (!o_) {
			lastErrorNumber = -1;
			return Ref<T>();
		}

		// 放入容器
		auto o = AddItem(std::move(o_));

		// 设置超时时长
		if (o->SetTimeout(interval)) {
			o->Dispose();
			return Ref<T>();
		}

		// 设置回调
		o->onFire = std::move(cb);

		// 初始化
		o->Init();
		return o;
	}



	template<typename U, typename ...Args>
	inline Ref<U> Context::CreateUdpPeer(int const& port, Args&&... args) {
		static_assert(std::is_base_of_v<UdpPeer, U>);

		// 创建 udp socket fd
		auto&& fd = MakeSocketFD(port, SOCK_DGRAM);
		if (fd < 0) {
			lastErrorNumber = fd;
			return Ref<U>();
		}
		// 确保 return 时自动 close
		xx::ScopeGuard sg([&] { close(fd); });

		// fd 纳入 epoll 管理
		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -4;
			return Ref<U>();
		}

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { CloseDel(fd); });

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<U>(std::forward<Args>(args)...);
		if (!o_) {
			lastErrorNumber = -5;
			return Ref<U>();
		}

		// 继续初始化并放入容器
		auto o = AddItem(std::move(o_), fd);

		// 撤销自动关闭行为并返回结果
		sg.Cancel();

		// 初始化
		o->Init();
		return o;
	}



	template<typename L, typename ...Args>
	inline Ref<L> Context::CreateSharedTcpListener(int const& fd, Args&&... args) {
		static_assert(std::is_base_of_v<TcpListener, L>);

		// fd 纳入 epoll 管理
		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -4;
			return Ref<L>();
		}

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<L>(std::forward<Args>(args)...);
		if (!o_) {
			lastErrorNumber = -5;
			return Ref<L>();
		}

		// 放入容器, 初始化并返回
		auto o = AddItem(std::move(o_), fd);
		o->Init();
		return o;
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


	template<typename T>
	inline T* Context::AddItem(std::unique_ptr<T>&& item, int const& fd) {
		static_assert(std::is_base_of_v<Item, T>);
		assert(item);
		auto p = item.get();
		p->indexAtContainer = items.Add(std::move(item));
		p->ep = this;
		if (fd != -1) {
			assert(!fdMappings[fd]);
			p->fd = fd;
			fdMappings[fd] = p;
		}
		return p;
	}

	inline CommandHandler* Context::EnableCommandLine() {
		// 已存在：直接短路返回
		if (fdMappings[STDIN_FILENO]) return (CommandHandler*)fdMappings[STDIN_FILENO];

		// 试将 fd 纳入 epoll 管理
		if (-1 == Ctl(STDIN_FILENO, EPOLLIN)) return nullptr;

		// 创建 stdin fd 的处理类并放入容器
		return AddItem(xx::MakeU<CommandHandler>(), STDIN_FILENO);
	}

	template<typename T, typename ...Args>
	Ref<T> Context::CreateINotifyHandler(char const* const& path, uint32_t const& eventMask) {
		auto&& fd = inotify_init();
		if (fd < 0) {
			lastErrorNumber = -1;
			return Ref<T>();
		}
		xx::ScopeGuard sg([&] { close(fd); });

		// 设置非阻塞状态
		if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) {
			lastErrorNumber = -2;
			return Ref<T>();
		}

		lastErrorNumber = inotify_add_watch(fd, path, eventMask);
		if (lastErrorNumber < 0) return Ref<T>();

		// fd 纳入 epoll 管理
		if (-1 == Ctl(fd, EPOLLIN)) {
			lastErrorNumber = -3;
			return Ref<T>();
		}

		// 确保 return 时自动 close 并脱离 epoll 管理
		sg.Set([&] { CloseDel(fd); });

		// 试创建目标类实例
		auto o_ = xx::TryMakeU<T>(std::forward<Args>(args)...);
		if (!o_) {
			lastErrorNumber = -4;
			return Ref<T>();
		}

		// 撤销自动关闭行为并返回结果
		sg.Cancel();

		// 放入容器, 初始化并返回
		auto o = AddItem(std::move(o_), fd);
		o->Init();
		return o;
	}
}
