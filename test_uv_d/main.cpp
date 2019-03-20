#include "xx_uv.h"
#include "xx_dict.h"

namespace xx {

	// todo: Dict -> unordered_map ?

	// udp peer 必然需要存储 weak 版 kcp peer 一对多的关系字典. key guid
	// kcp peer 强引用 udp peer, 在 Dispose 时从 udp peer 字典中移除自己, 并 reset 强引用
	// uv 用一个字典存储 weak 版 udp peer. key 为 int. listener 的为监听端口, dialer 的为负增长id
	// udp peer 有一个 weak owner, 提供 create peer, accept 功能. 通常为 listener | dialer.

	// udp 握手设计: client 以适当的频率反复发送 1 到 server.  server 根据其 addr( ip+port ) 为 key 在字典 创建记录guid + 创建时间 并下发
	// 如果 key 已存在就直接下发 guid. 每收到一次 1 就下发一次. 
	// client 在收到 guid 之后存起来备用并停止发送 1
	// 之后 server 只认 guid 作为prefix 的 kcp 包. 首次收到数据时如果 guid, kcp 字典中找不到, 则根据 addr 去上面的字典找. 找到后创建 kcp peer, 握手完成
	// dialer 的 req 自发完成上述操作, 并于收到 guid 后产生 OnConnect 回调. 其他 req 则 cancel

	template<typename T>
	using Shared = std::shared_ptr<T>;
	template<typename T>
	using Weak = std::weak_ptr<T>;


	struct KcpPeer;
	// Listener & Dialer 的基类
	struct KcpPeerOwner : UvItem {
		using UvItem::UvItem;
		inline virtual Shared<KcpPeer> CreatePeer() noexcept = 0;
		inline virtual void Accept(Shared<KcpPeer>& peer) noexcept = 0;
	};

	// udp 通信层 基类
	struct KcpUdp : UvUdpBasePeer {
		using UvUdpBasePeer::UvUdpBasePeer;
		KcpPeerOwner* owner = nullptr;				// fill by owner
		int port = 0;								// fill by owner. dict's key. port > 0: listener  < 0: dialer fill by --uv.udpId
		virtual void Update(int64_t const& nowMS) noexcept = 0;
		virtual void Remove(Guid const& g) noexcept = 0;
	};

	// 基于 kcp 的逻辑通信层
	struct KcpPeer : UvItem {
		using UvItem::UvItem;
		Shared<KcpUdp> udp;							// fill by creater
		Guid guid;									// fill by creater
		int64_t createMS = 0;						// fill by creater

		ikcpcb* kcp = nullptr;
		uint32_t currentMS = 0;						// fill before Update by dialer / listener
		uint32_t nextUpdateMS = 0;					// for kcp update interval control. reduce cpu usage
		int serial = 0;
		std::unordered_map<int, std::pair<std::function<int(Object_s&& msg)>, UvTimer_s>> callbacks;
		Buffer buf;
		sockaddr_in6 addr;							// for Send. fill by owner Unpack
		uint32_t receiveTimeoutMS = 0;				// if receiveTimeoutMS > 0 && Update current - lastReceiveMS > receiveTimeoutMS , will Dispose
		uint32_t lastReceiveMS = 0;					// refresh at Update when receive data

		std::function<void()> OnDisconnect;
		inline virtual void Disconnect() noexcept { if (OnDisconnect) OnDisconnect(); }
		// return !0 will Dispose
		std::function<int(Object_s&& msg)> OnReceivePush;
		inline virtual int ReceivePush(Object_s&& msg) noexcept { return OnReceivePush ? OnReceivePush(std::move(msg)) : 0; };
		std::function<int(int const& serial, Object_s&& msg)> OnReceiveRequest;
		inline virtual int ReceiveRequest(int const& serial, Object_s&& msg) noexcept { return OnReceiveRequest ? OnReceiveRequest(serial, std::move(msg)) : 0; };

		// 填充 guid 之后调用
		inline int InitKcp() {
			if (kcp) return -1;
			kcp = ikcp_create(guid, this);
			if (!kcp) return -1;
			ScopeGuard sgKcp([&] { ikcp_release(kcp); kcp = nullptr; });
			if (int r = ikcp_wndsize(kcp, 128, 128)) return r;
			if (int r = ikcp_nodelay(kcp, 1, 10, 2, 1)) return r;
			kcp->rx_minrto = 10;
			ikcp_setoutput(kcp, [](const char *inBuf, int len, ikcpcb *kcp, void *user)->int {
				auto self = ((KcpPeer*)user);
				return self->udp->Send((uint8_t*)inBuf, len, (sockaddr*)&self->addr);
			});
			sgKcp.Cancel();
			return 0;
		}

		virtual bool Disposed() const noexcept override {
			return !kcp;
		}
		virtual void Dispose(int const& flag = 1) noexcept override {
			if (!kcp) return;
			ikcp_release(kcp);
			kcp = nullptr;
			udp->Remove(guid);						// remove self from container
			udp.reset();							// unbind
			for (auto&& kv : callbacks) {
				kv.second.first(nullptr);
			}
			callbacks.clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();						// maybe unhold memory here
				OnDisconnect = nullptr;
				OnReceivePush = nullptr;
				OnReceiveRequest = nullptr;
			}
		}
		~KcpPeer() {
			this->Dispose(0);
		}

		// called by udp kcp dialer or listener
		// put data to kcp when udp receive 
		inline int Input(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			if (!kcp) return -1;
			return ikcp_input(kcp, (char*)recvBuf, recvLen);
		}

		// called by udp kcp dialer or listener
		// timer call this for recv data from kcp
		inline void Update(int64_t const& nowMS) noexcept {
			assert(kcp);
			currentMS = uint32_t(nowMS - createMS);						// known issue: 超出 uint32 限制. 理论上只能持续连接几十天
			if (receiveTimeoutMS && receiveTimeoutMS + lastReceiveMS < currentMS) {		// receive timeout
				Dispose();
				return;
			}

			if (nextUpdateMS > currentMS) return;						// reduce cpu usage
			ikcp_update(kcp, currentMS);
			if (!kcp) return;
			nextUpdateMS = ikcp_check(kcp, currentMS);

			do {
				int recvLen = ikcp_recv(kcp, uv.recvBuf.data(), (int)uv.recvBuf.size());
				if (recvLen <= 0) break;
				if (int r = Unpack((uint8_t*)uv.recvBuf.data(), recvLen)) {
					Dispose();
					return;
				}
			} while (true);
		}

		inline int SendPush(Object_s const& data) {
			return SendPackage(data);
		}
		inline int SendResponse(int32_t const& serial, Object_s const& data) {
			return SendPackage(data, serial);
		}
		inline int SendRequest(Object_s const& msg, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS = 0) {
			return SendRequest(msg, 0, std::move(cb), timeoutMS);
		}

		// send data immediately ( no wait for more data combine send )
		inline void Flush() {
			if (!kcp) return;
			ikcp_flush(kcp);
		}

		// set timeout check duration. 0 = disable check
		inline void SetReceiveTimeoutMS(uint32_t const& receiveTimeoutMS) {
			this->receiveTimeoutMS = receiveTimeoutMS;
		}

		// refresh lastReceiveMS when receive a valid message
		inline void ResetLastReceiveMS() {
			lastReceiveMS = currentMS;
		}

	protected:
		// 4 bytes len header. can override for custom header format.
		inline virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			buf.AddRange(recvBuf, recvLen);
			uint32_t offset = 0;
			while (offset + 4 <= buf.len) {							// ensure header len( 4 bytes )
				auto len = buf[offset + 0] + (buf[offset + 1] << 8) + (buf[offset + 2] << 16) + (buf[offset + 3] << 24);
				if (len <= 0 /* || len > maxLimit */) return -1;	// invalid length
				if (offset + 4 + len > buf.len) break;				// not enough data

				offset += 4;
				if (int r = HandlePack(buf.buf + offset, len)) return r;
				offset += len;
			}
			buf.RemoveFront(offset);
			return 0;
		}

		// unpack & dispatch
		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			auto& recvBB = uv.recvBB;
			recvBB.Reset((uint8_t*)recvBuf, recvLen);

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;
			Object_s msg;
			if (int r = recvBB.ReadRoot(msg)) return r;

			if (serial == 0) {
				return ReceivePush(std::move(msg));
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, std::move(msg));
			}
			else {
				auto iter = callbacks.find(serial);
				if (iter == callbacks.end()) return 0;
				int r = iter->second.first(std::move(msg));
				callbacks.erase(iter);
				return r;
			}
		}

		// put send data into kcp. though ikcp_setoutput func send.
		inline int Send(uint8_t const* const& buf, ssize_t const& dataLen) noexcept {
			if (!kcp) return -1;
			return ikcp_send(kcp, (char*)buf, (int)dataLen);
		}

		// serial == 0: push    > 0: response    < 0: request
		inline int SendPackage(Object_s const& data, int32_t const& serial = 0, int const& tar = 0) {
			if (!kcp) return -1;
			auto& sendBB = uv.sendBB;
			sendBB.Resize(4);						// header len( 4 bytes )
			if (tar) sendBB.WriteFixed(tar);		// for router
			sendBB.Write(serial);
			sendBB.WriteRoot(data);
			auto buf = sendBB.buf;
			auto len = sendBB.len - 4;
			buf[0] = uint8_t(len);					// fill package len
			buf[1] = uint8_t(len >> 8);
			buf[2] = uint8_t(len >> 16);
			buf[3] = uint8_t(len >> 24);
			return Send(buf, sendBB.len);
		}

		inline int SendRequest(Object_s const& data, int const& tar, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS = 0) {
			if (!kcp) return -1;
			std::pair<std::function<int(Object_s&& msg)>, UvTimer_s> v;
			++serial;
			if (timeoutMS) {
				v.second = TryMake<UvTimer>(uv);
				if (!v.second) return -1;
				if (int r = v.second->Start(timeoutMS, 0, [this, serial = this->serial]() {
					TimeoutCallback(serial);
				})) return r;
			}
			if (int r = SendPackage(data, -serial, tar)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline void TimeoutCallback(int const& serial) {
			auto iter = callbacks.find(serial);
			if (iter == callbacks.end()) return;
			iter->second.first(nullptr);
			callbacks.erase(iter);
		}
	};

	// 扩展 uv, 主要为为 kcp 物理通信层提供 update 服务
	struct KcpUv : Uv {
		Dict<int, Weak<KcpUdp>> udps;
		int udpId = 0;					// client udp peer port 生成: --udpId
		UvTimer_s updater;
		std::chrono::steady_clock::time_point createTime = std::chrono::steady_clock::now();
		int64_t nowMS = 0;
		KcpUv() {
			xx::MakeTo(updater, *this, 10, 10, [this] {
				nowMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->createTime).count();
				for (auto&& kv : udps) {
					kv.value.lock()->Update(nowMS);
				}
			});
		}
	};

	// listener 专用的 udp 通信层
	struct KcpListenerUdp : KcpUdp {
		using KcpUdp::KcpUdp;
		Dict<Guid, Weak<KcpPeer>> peers;
		Dict<std::string, std::pair<Guid, int64_t>> shakes;	// key: ip:port   value: guid,nowMS
		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!this->uvUdp) return;
			this->UvUdpBasePeer::Dispose(flag);
			for (auto&& kv : peers) {
				if (auto&& peer = kv.value.lock()) {
					peer->Dispose(flag);
				}
			}
			peers.Clear();
			((KcpUv&)uv).udps.Remove(port);
		}

		~KcpListenerUdp() {
			this->Dispose(0);
		}
		inline virtual void Update(int64_t const& nowMS) noexcept override {
			for (auto&& kv : peers) {
				kv.value.lock()->Update(nowMS);
			}
			for (auto&& iter = shakes.begin(); iter != shakes.end(); ++iter) {
				if ((*iter).value.second < nowMS) {
					shakes.RemoveAt(iter.i);
				}
			}
		}
		inline virtual void Remove(Guid const& g) noexcept override {
			peers.Remove(g);
		}

	protected:
		inline virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen, sockaddr const* const& addr) noexcept override {
			assert(port);
			// 看看是不是握手包 且这个 udp peer 的 owner 是健在的 listener
			if (recvLen == 4 && owner) {						// 握手包含有 4 字节自增序列号
				auto&& ipAndPort = Uv::ToIpPortString(addr);
				// ip_port : guid, createMS
				auto&& idx = shakes.Find(ipAndPort);
				if (idx == -1) {
					idx = shakes.Add(ipAndPort, std::make_pair(Guid(), ((KcpUv&)uv).nowMS + 3000)).index;	// 暂定写死握手 3 秒超时
				}
				memcpy(recvBuf + 4, &shakes.ValueAt(idx).first, 16);	// 序列号携带 guid 一起返回( 这里临时用一下 recvBuf 是安全的, 长度足够 )
				return this->Send(recvBuf, 20, addr);
			}

			// header 至少有 IKCP_OVERHEAD 字节长( kcp 头 ). 少于 IKCP_OVERHEAD 的直接忽略
			if (recvLen < IKCP_OVERHEAD) {
				return 0;
			}

			// 前 16 字节转为 Guid
			Guid g(false);
			g.Fill(recvBuf);
			Shared<KcpPeer> peer;

			// 去字典中找. 如果在握手队列中发现就创建
			auto&& idx = peers.Find(g);
			if (idx == -1) {									// guid 未找到: 如果是 listener: 用 addr 进一步去 shakes 找
				if (!owner || owner->Disposed()) return 0;		// listener 已经没了: 忽略
				auto&& ipAndPort = Uv::ToIpPortString(addr);
				idx = shakes.Find(ipAndPort);
				if (idx == -1 || shakes.ValueAt(idx).first != g) return 0;	// addr 没找到 或 guid 对不上: 忽略
				shakes.RemoveAt(idx);							// 从握手队列移除
				peer = owner->CreatePeer();						// 创建 kcp peer 并填充基础数据
				if (!peer) return 0;
				peer->udp = std::move(xx::As<KcpUdp>(shared_from_this()));
				peer->guid = g;
				peer->createMS = NowEpoch10m() / 10000;
				memcpy(&peer->addr, addr, sizeof(sockaddr_in6));// 更新 peer 的目标 ip 地址
				if (peer->InitKcp()) return 0;					// 初始化 kcp 失败直接忽略
				peers[g] = peer;								// 塞字典
				owner->Accept(peer);							// 触发 accept 回调
			}
			else {
				auto&& peer_w = peers.ValueAt(idx);
				peer = peer_w.lock();
				if (!peer || peer->Disposed()) return 0;		// 如果 kcp peer 已经没了就忽略
			}

			memcpy(&peer->addr, addr, sizeof(sockaddr_in6));	// 更新 peer 的目标 ip 地址
			if (peer->Input(recvBuf, recvLen)) {
				peer->Dispose();								// peer 自己会从 peers 中移除
			}
			return 0;
		}
	};

	// dialer 专用的 udp 通信层
	struct KcpDialerUdp : KcpUdp {
		using KcpUdp::KcpUdp;
		int i = 0;
		bool connected = false;
		Weak<KcpPeer> peer_w;

		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!this->uvUdp) return;
			this->UvUdpBasePeer::Dispose(flag);
			if (auto&& peer = peer_w.lock()) {
				peer->Dispose(flag);
			}
			((KcpUv&)uv).udps.Remove(port);
		}
		~KcpDialerUdp() {
			this->Dispose(0);
		}
		inline virtual void Update(int64_t const& nowMS) noexcept override {
			if (connected) {
				if (auto&& peer = peer_w.lock()) {
					peer->Update(nowMS);
				}
				else {
					Dispose();
				}
			}
			else {
				++i;
				if ((i & 0xFu) == 0) {		// 每 16 帧发送一次
					if (int r = Send((uint8_t*)&port, sizeof(port))) {
						Dispose();
					}
				}
			}
		}
		inline virtual void Remove(Guid const& g) noexcept override {
			Dispose();
		}

	protected:
		virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen, sockaddr const* const& addr) noexcept override {
			assert(owner || peer_w.lock());

			// 看看是不是握手回应包
			if (recvLen == 20 && owner) {						// 握手回应包含有 4 字节自增序列号 + 16 字节 guid
				if (memcmp(recvBuf, &port, 4)) return 0;		// 序列号对不上
				auto&& p = owner->CreatePeer();
				peer_w = p;										// bind
				p->udp = std::move(xx::As<KcpUdp>(shared_from_this()));
				memcpy(&p->guid, recvBuf + 4, 16);
				memcpy(&p->addr, addr, sizeof(sockaddr_in6));	// 更新 peer 的目标 ip 地址
				p->createMS = NowEpoch10m() / 10000;
				if (p->InitKcp()) return 0;						// 初始化 kcp 失败直接忽略
				connected = true;								// 标记为已连接
				owner->Accept(p);								// cleanup all reqs( 当前 udp 已经 bind 到 kcp 上且 kcp 被 dialer 持有, 并不会被 dispose )
				return 0;
			}

			// header 至少有 IKCP_OVERHEAD 字节长( kcp 头 ). 少于 IKCP_OVERHEAD 的直接忽略
			if (recvLen < IKCP_OVERHEAD) {
				return 0;
			}

			auto&& peer = peer_w.lock();
			if (!peer) return 0;								// 握手没完成? 忽略

			// 前 16 字节转为 Guid
			Guid g(false);
			g.Fill(recvBuf);
			if (peer->guid != g) return 0;						// guid 对不上? 忽略

			memcpy(&peer->addr, addr, sizeof(sockaddr_in6));	// 更新 peer 的目标 ip 地址
			if (peer->Input(recvBuf, recvLen)) {				// 数据输入
				peer->Dispose();								// peer 自己调用 Remove
			}
			return 0;
		}
	};


	template<typename PeerType = KcpPeer>
	struct KcpListener : KcpPeerOwner {
		Shared<KcpListenerUdp> udp;

		std::function<Shared<PeerType>(Uv& uv)> OnCreatePeer;
		inline virtual Shared<KcpPeer> CreatePeer() noexcept override { return OnCreatePeer ? OnCreatePeer(uv) : TryMake<PeerType>(uv); }
		std::function<void(Shared<PeerType>& peer)> OnAccept;
		inline virtual void Accept(Shared<KcpPeer>& peer) noexcept override { if (OnAccept) OnAccept(xx::As<PeerType>(peer)); }

		KcpListener(Uv& uv, std::string const& ip, int const& port)
			: KcpPeerOwner(uv) {
			auto&& udps = ((KcpUv&)uv).udps;
			auto idx = udps.Find(port);
			if (idx > -1) {
				udp = xx::As<KcpListenerUdp>(udps.ValueAt(idx).lock());
				if (udp->owner) throw - 1;			// same port listener already exists?
			}
			else {
				MakeTo(udp, uv, ip, port, true);
				udp->port = port;
				udp->owner = this;
				udps[port] = udp;
			}
			udp->owner = this;
		}
		~KcpListener() { this->Dispose(0); }
		virtual bool Disposed() const noexcept override {
			return !udp;
		}
		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!udp) return;
			udp->owner = nullptr;								// unbind
			udp.reset();
			if (flag) {
				auto holder = shared_from_this();
				OnCreatePeer = nullptr;
				OnAccept = nullptr;
			}
		}
	};

	template<typename PeerType = KcpPeer>
	struct KcpDialer : KcpPeerOwner {
		using KcpPeerOwner::KcpPeerOwner;
		std::unordered_map<int, Shared<KcpDialerUdp>> reqs;		// key: port
		UvTimer_s timeouter;
		Shared<PeerType> peer;

		std::function<Shared<PeerType>(Uv& uv)> OnCreatePeer;
		inline virtual Shared<KcpPeer> CreatePeer() noexcept override { return OnCreatePeer ? OnCreatePeer(uv) : TryMake<PeerType>(uv); }
		std::function<void(Shared<PeerType>& peer)> OnAccept;

		inline virtual void Accept(Shared<PeerType>& peer) noexcept override {
			assert(!this->peer);
			if (peer) {
				timeouter->Stop();
				auto&& udp = xx::As<KcpDialerUdp>(peer->udp);
				udp->owner = nullptr;
				this->peer = std::move(peer);
				reqs.clear();
			}
			if (this->OnAccept) {
				this->OnAccept(this->peer);
			}
		}
		KcpDialer(Uv& uv)
			: KcpPeerOwner(uv) {
			timeouter = Make<UvTimer>(uv);
		}
		~KcpDialer() { this->Dispose(0); }
		virtual bool Disposed() const noexcept override {
			return !timeouter;
		}
		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!timeouter) return;
			Cancel();
			timeouter.reset();
			if (flag) {
				auto holder = shared_from_this();
				OnCreatePeer = nullptr;
				OnAccept = nullptr;
			}
		}

		inline int Dial(std::string const& ip, int const& port, uint64_t const& timeoutMS = 0, bool cleanup = true) noexcept {
			if (!timeouter) return -1;
			if (cleanup) {
				Cancel();
			}
			if (int r = SetTimeout(timeoutMS)) return r;
			auto req = TryMake<KcpDialerUdp>(uv, ip, port, false);
			if (!req) return -2;
			req->owner = this;
			req->port = --((KcpUv&)uv).udpId;
			((KcpUv&)uv).udps[req->port] = req;
			reqs[req->port] = std::move(req);
			return 0;
		}

		// todo: batch dial

		inline void Cancel(bool resetPeer = true) noexcept {
			if (!timeouter) return;
			timeouter->Stop();
			if (resetPeer) {
				peer.reset();
			}
			reqs.clear();
		}

	protected:
		inline int SetTimeout(uint64_t const& timeoutMS = 0) noexcept {
			if (!timeouter) return -1;
			int r = timeouter->Stop();
			if (!timeoutMS) return r;
			return timeouter->Start(timeoutMS, 0, [self_w = AsWeak<KcpDialer>(shared_from_this())]{
				if (auto self = self_w.lock()) {
					self->Cancel(true);
					self->Accept(self->peer);
				}
				});
		}
	};
}

int main(int argc, char* argv[]) {
	xx::KcpUv uv;
	auto&& listener = xx::Make<xx::KcpListener<>>(uv, "0.0.0.0", 12345);
	listener->OnAccept = [](xx::Shared<xx::KcpPeer>& peer) {
		xx::CoutN("listener ", peer ? "accept" : "timeout");
		peer->OnReceivePush = [peer](xx::Object_s&& msg) {
			xx::CoutN("listener peer recv ", msg);
			return peer->SendPush(msg);
		};
	};

	auto&& dialer = xx::Make<xx::KcpDialer<>>(uv);
	dialer->OnAccept = [](xx::Shared<xx::KcpPeer> peer) {
		xx::CoutN("dialer ", peer ? "accept" : "timeout");
		peer->OnReceivePush = [peer](xx::Object_s&& msg) {
			xx::CoutN("dialer peer recv ", msg);
			return peer->SendPush(msg);
		};
		peer->SendPush(xx::Make<xx::BBuffer>());
	};
	auto&& r = dialer->Dial("127.0.0.1", 12345, 3000);
	xx::CoutN(r);
	uv.Run();
	return 0;
}
