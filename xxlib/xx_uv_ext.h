#pragma once
#include "xx_uv.h"
namespace xx {

	// gateway 相关 peer 基类
	// 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvCommandPeer : UvPeer {
		using UvPeer::UvPeer;

		// 当收到内部指令包时 投递到该事件回调( bb 为 uv.recvBB )
		// 需要在派生类中 Dispose
		std::function<int(BBuffer& bb)> onReceiveCommand;

		// 构造以 id == 0xFFFFFFFF 打头的内部指令包( 符合 header + id + data 的结构 )
		template<typename...Args>
		inline int SendCommand(Args const& ... args) {
			auto&& bb = uv.sendBB;
			peerBase->SendPrepare(bb, 1024);
			bb.WriteFixed((uint32_t)0xFFFFFFFF);
			bb.Write(args...);
			return peerBase->SendAfterPrepare(bb);
		}
	};




	// 虚拟 peer ( 通过网关链路 peer 产生 )
	struct UvSimulatePeer;

	// 连上 gateway 后产生的链路 peer. 为 模拟的 peers 的父容器
	struct UvFromToGatewayBasePeer : UvCommandPeer {

		std::function<int(uint32_t const& id, uint8_t* const& buf, size_t const& len)> onReceive;

		std::unordered_map<uint32_t, std::shared_ptr<UvSimulatePeer>> simulatePeers;

		// for simulatePeers Update
		xx::UvTimer_s timer;

		UvFromToGatewayBasePeer(xx::Uv& uv);

		inline virtual bool Dispose(int const& flag = 1) noexcept override {
			if (Disposed()) return false;
			xx::UvItem_s holder;
			if (flag != -1) {
				holder = shared_from_this();
			}
			this->UvCommandPeer::Dispose(flag);
			if (flag == -1) return true;
			onReceive = nullptr;
			onReceiveCommand = nullptr;
			return true;
		}

		template<typename PeerType = UvSimulatePeer>
		std::shared_ptr<PeerType> CreateSimulatePeer(uint32_t const& id);
		void DisconnectSimulatePeers();
		void DisconnectSimulatePeer(uint32_t const& id);

		inline int SendTo(uint32_t const& id, int32_t const& serial, Object_s const& msg) {
			if (!peerBase) return -1;
			auto&& bb = uv.sendBB;
			peerBase->SendPrepare(bb, 1024);
			bb.WriteFixed(id);
			bb.Write(serial);
			bb.WriteRoot(msg);
			return peerBase->SendAfterPrepare(bb);
		}

		inline int SendTo(uint32_t const& id, int32_t const& serial, BBuffer const& data) {
			if (!peerBase) return -1;
			auto&& bb = uv.sendBB;
			peerBase->SendPrepare(bb, 1024);
			bb.WriteFixed(id);
			bb.Write(serial);
			bb.AddRange(data.buf, data.len);
			return peerBase->SendAfterPrepare(bb);
		}


	protected:

		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			auto& bb = uv.recvBB;
			bb.Reset(recvBuf, recvLen);

			uint32_t id = 0;
			if (int r = bb.ReadFixed(id)) return r;
			if (id == 0xFFFFFFFFu) {
				return onReceiveCommand ? onReceiveCommand(bb) : 0;
			}

			return onReceive ? onReceive(id, recvBuf + bb.offset, recvLen - bb.offset) : 0;
		}
	};






	// service 被 gateway 连上后产生
	struct UvFromGatewayPeer : UvFromToGatewayBasePeer {
		using UvFromToGatewayBasePeer::UvFromToGatewayBasePeer;

		// 内部网关编号, 建立连接后沟通填充
		uint32_t gatewayId = 0xFFFFFFFFu;

		int SendCommand_Open(uint32_t const& clientId) {
			return SendCommand("open", clientId);
		}
		int SendCommand_Close(uint32_t const& clientId) {
			return SendCommand("close", clientId);
		}
		int SendCommand_Kick(uint32_t const& clientId, int64_t const& delayMS) {
			return SendCommand("kick", clientId, delayMS);
		}
		int SendCommand_Ping(int64_t const& ticks) {
			return SendCommand("ping", ticks);
		}
	};

	// service 监听 gateways 专用
	struct UvFromGatewayListener : UvListener {
		UvFromGatewayListener(Uv& uv, std::string const& ip, int const& port)
			: UvListener(uv, ip, port, 0) {
		}

		// 注意：该函数废掉了 onCreatePeer 调用
		inline virtual UvPeer_s CreatePeer() noexcept override {
			return xx::TryMake<UvFromGatewayPeer>(uv);
		}
	};






	// services 互连 专用
	struct UvFromToServicePeer : xx::UvPeer {
		using UvPeer::UvPeer;

		// 内部服务编号, 建立连接后沟通填充
		uint32_t serviceId = 0xFFFFFFFFu;

		int SendCommand_Accept(uint32_t const& gatewayId, uint32_t const& clientId) {
			auto&& bb = uv.sharedBB;
			bb->Clear();
			bb->Write("accept", gatewayId, clientId);
			return SendPush(bb);
		}

		int SendPush_Command_ServiceId(uint32_t const& serviceId) {
			auto&& bb = uv.sharedBB;
			bb->Clear();
			bb->Write("serviceId", serviceId);
			return SendPush(bb);
		}
	};

	// service 监听 services 专用
	struct UvFromServiceListener : UvListener {
		UvFromServiceListener(Uv& uv, std::string const& ip, int const& port)
			: UvListener(uv, ip, port, 0) {
		}

		// 注意：该函数废掉了 onCreatePeer 调用
		inline virtual UvPeer_s CreatePeer() noexcept override {
			return xx::TryMake<UvFromToServicePeer>(uv);
		}
	};

	// service 连 services 专用
	struct UvToServiceDialer : xx::UvDialer {
		UvToServiceDialer(xx::Uv& uv)
			: UvDialer(uv, 0) {
		}

		// 注意：该函数废掉了 onCreatePeer 调用
		inline virtual UvPeer_s CreatePeer() noexcept override {
			auto&& p = xx::TryMake<UvFromToServicePeer>(uv);
			if (p) {
				p->serviceId = this->serviceId;
			}
			return p;
		}

		// store for redial
		uint32_t serviceId = 0xFFFFFFFFu;
		std::string ip;
		int port = 0;

		inline int Dial(uint64_t const& timeoutMS = 2000) {
			return this->UvDialer::Dial(ip, port, timeoutMS);
		}
	};








	// 模拟的 peer
	struct UvSimulatePeer : UvItem {
		// 指向总容器
		std::weak_ptr<UvFromToGatewayBasePeer> gatewayPeer;
		uint32_t id = 0xFFFFFFFFu;
		bool disposed = false;

		// 下面代码抄自 UvPeer 并小改. peerBase 变为了 gatewayPeer

		Dict<int, std::pair<std::function<int(Object_s&& msg)>, int64_t>> callbacks;
		int serial = 0;
		int64_t timeoutMS = 0;
		std::function<void()> onDisconnect;
		std::function<int(Object_s&& msg)> onReceivePush;
		std::function<int(int const& serial, Object_s&& msg)> onReceiveRequest;


		UvSimulatePeer(std::shared_ptr<UvFromToGatewayBasePeer> gp, uint32_t const& id)
			: UvItem(gp->uv)
			, id(id) {
			gatewayPeer = gp;
		}

		inline virtual void Disconnect() noexcept {
			if (onDisconnect) {
				onDisconnect();
			}
		}

		inline virtual int ReceivePush(Object_s&& msg) noexcept {
			return onReceivePush ? onReceivePush(std::move(msg)) : 0;
		}

		inline virtual int ReceiveRequest(int const& serial, Object_s&& msg) noexcept {
			return onReceiveRequest ? onReceiveRequest(serial, std::move(msg)) : 0;
		}

		inline int SendPush(Object_s const& msg) noexcept {
			return SendResponse(0, msg);
		}

		inline int SendResponse(int32_t const& serial, Object_s const& msg) noexcept {
			auto&& gp = gatewayPeer.lock();
			if (!gp) return -1;
			return gp->SendTo(id, serial, msg);
		}

		inline int SendRequest(Object_s const& msg, std::function<int(Object_s&& msg)>&& cb, uint64_t const& timeoutMS) noexcept {
			if (disposed) return -1;
			std::pair<std::function<int(Object_s && msg)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			v.second = NowSteadyEpochMS() + (timeoutMS ? timeoutMS : uv.defaultRequestTimeoutMS);
			if (int r = SendResponse(-serial, msg)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			auto& bb = uv.recvBB;
			bb.Reset((uint8_t*)recvBuf, recvLen);

			int serial = 0;
			if (int r = bb.Read(serial)) return r;

			Object_s msg;
			if (int r = bb.ReadRoot(msg)) return r;

			if (serial == 0) {
				return ReceivePush(std::move(msg));
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, std::move(msg));
			}
			else {
				auto&& idx = callbacks.Find(serial);
				if (idx == -1) return 0;
				auto a = std::move(callbacks.ValueAt(idx).first);
				callbacks.RemoveAt(idx);
				return a(std::move(msg));
			}
		}

		// call by gatewayPeer's timer
		inline virtual int Update(int64_t const& nowMS) noexcept {
			if (disposed) return -1;

			if (timeoutMS && timeoutMS < nowMS) {
				Dispose();
				return -1;
			}

			for (auto&& iter = callbacks.begin(); iter != callbacks.end(); ++iter) {
				auto&& v = iter->value;
				if (v.second < nowMS) {
					auto a = std::move(v.first);
					iter.Remove();
					a(nullptr);
				}
			}

			return 0;
		}

		inline virtual bool Disposed() const noexcept override {
			return disposed;
		}

		inline virtual bool Dispose(int const& flag = 1) noexcept override {
			if (disposed) return false;
			disposed = true;
			gatewayPeer.reset();
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			if (flag == -1) return true;
			auto holder = shared_from_this();
			if (flag) {
				Disconnect();
			}
			onDisconnect = nullptr;
			onReceivePush = nullptr;
			onReceiveRequest = nullptr;
			id = 0xFFFFFFFFu;
			return true;
		}
	};

	inline UvFromToGatewayBasePeer::UvFromToGatewayBasePeer(xx::Uv& uv) : UvCommandPeer(uv) {
		xx::MakeTo(timer, uv, 10, 10, [this] {
			auto&& nowMS = xx::NowSteadyEpochMS();
			auto&& iter = simulatePeers.begin();
			for (;iter != simulatePeers.end();) {
				if (iter->second && !iter->second->Disposed()) {
					(void)iter->second->Update(nowMS);
					iter++;
				}
				else {
					simulatePeers.erase(iter++);
				}
			}
		});
	}


	inline void UvFromToGatewayBasePeer::DisconnectSimulatePeers() {
		for (auto&& p : simulatePeers) {
			if (p.second) {
				p.second->Dispose();
			}
		}
		simulatePeers.clear();
	}

	inline void UvFromToGatewayBasePeer::DisconnectSimulatePeer(uint32_t const& id) {
		auto&& iter = simulatePeers.find(id);
		if (iter == simulatePeers.end()) return;
		if (iter->second) {
			iter->second->Dispose();
		}
		simulatePeers.erase(iter);
	}

	template<typename PeerType>
	inline std::shared_ptr<PeerType> UvFromToGatewayBasePeer::CreateSimulatePeer(uint32_t const& id) {
		// 创建
		auto&& self = As<UvFromToGatewayBasePeer>(shared_from_this());
		auto&& p = Make<PeerType>(self, id);

		// 放入( 断开已存在的 )
		auto&& iter = simulatePeers.find(id);
		if (iter != simulatePeers.end()) {
			if (iter->second && !iter->second->Disposed()) {
				iter->second->Dispose();
			}
			iter->second = p;
		}
		else {
			simulatePeers.emplace(id, p);
		}
		return p;
	}






	// 基于帧逻辑特性, 通常需要在特定生命周期才开始处理数据包, 故先塞 recvs. 处理点自己 遍历 + clear
	struct UvFrameSimulatePeer : UvSimulatePeer {
		using UvSimulatePeer::UvSimulatePeer;

		std::vector<std::pair<uint32_t, Object_s>> recvs;			// 收到的 Push & Request 都在这

		inline virtual int ReceivePush(Object_s&& msg) noexcept override {
			recvs.emplace_back(0, std::move(msg));
			return 0;
		}

		inline virtual int ReceiveRequest(int const& serial, Object_s&& msg) noexcept override {
			recvs.emplace_back(serial, std::move(msg));
			return 0;
		}
	};






	// client 连 gateways 专用
	template<typename SimulatePeerType>
	struct UvToGatewayDialer : UvDialer {
		using UvDialer::UvDialer;

		// 物理链路 peer
		std::shared_ptr<UvFromToGatewayBasePeer> peer;

		// 默认自动掐线超时毫秒( 可 bind 到 lua 于 Dial 前设置 )
		int peerTimeoutMS = 10000;

		// peer check 检测时间间隔 ms( 可 bind 到 lua 于 PeerAlive 前设置 )
		int peerCheckIntervalMS = 5000;

		std::function<void(std::shared_ptr<SimulatePeerType>&)> onAcceptSimulatePeer;
		inline virtual UvPeer_s CreatePeer() noexcept override {
			return TryMake<UvFromToGatewayBasePeer>(uv);
		}
		inline virtual void Accept(UvPeer_s peer_) noexcept override {
			if (!peer_) return;
			peerChecking = false;
			peer = As<UvFromToGatewayBasePeer>(peer_);

			peer->onReceiveCommand = [this](BBuffer& bb)->int {
				peer->ResetTimeoutMS(peerTimeoutMS);
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;
				if (cmd == "ping") {
					int64_t lastMS = 0;
					if (int r = bb.Read(lastMS)) return r;
					this->peerChecking = false;
					this->peerPing = (int)(xx::NowSteadyEpochMS() - (this->peerCheckNextMS - this->peerCheckIntervalMS));
					return 0;
				}

				else if (cmd == "open") {
					uint32_t serviceId = 0;
					if (int r = bb.Read(serviceId)) return r;

					auto&& sp = peer->CreateSimulatePeer<SimulatePeerType>(serviceId);
					onAcceptSimulatePeer(sp);

					//CoutN("UvToGatewayPeer recv cmd open: serviceId = ", serviceId);
					return 0;
				}

				else if (cmd == "close") {
					uint32_t id = 0;
					if (int r = bb.Read(id)) return r;

					if (id) {
						peer->DisconnectSimulatePeer(id);
					}
					// id == 0 意思是直接自杀 以加速关闭的速度, 降低靠 ping 探测延迟发现已断开的时长( 通常需要几秒 )
					else 
						return -1;

					//CoutN("UvToGatewayPeer recv cmd disconnect: clientId = ", clientId);
					return 0;
				}

				else {
					return -3;
				}
				return 0;
			};

			peer->onReceive = [this](uint32_t const& id, uint8_t* const& buf, size_t const& len)->int {
				peer->ResetTimeoutMS(peerTimeoutMS);
				auto&& iter = peer->simulatePeers.find(id);
				if (iter == peer->simulatePeers.end()
					|| !iter->second
					|| iter->second->Disposed()) {
					return -1;
				}
				int r = iter->second->HandlePack(buf, (uint32_t)len);
				if (r) {
					peer->DisconnectSimulatePeer(id);
					return r;
				}
				return 0;
			};

			peer->onDisconnect = [this] {
				peer->DisconnectSimulatePeers();
			};

			peer->ResetTimeoutMS(peerTimeoutMS);
		}


		// 标志位. true: 正在等待 gateway 回包
		bool peerChecking = false;

		// peer 下次可发送 check 的时间. 同时也是等待超时的时间.
		int64_t peerCheckNextMS = 0;

		// 存放 peer check 回包后计算出来的延迟 MS( 可 bind 到 lua 方便读取 )
		int peerPing = 0;

		// 返回 >= 0 表示 peer 状态正常 且该值为上次检测到的 ping 值，返回 -1 表示 peer 已断。
		inline int PeerAlive() {
			// 如果已断开 直接返回 false
			if (!peer || peer->Disposed()) 
				return -1;
			auto&& now = xx::NowSteadyEpochMS();
			// 如果判断时间点没到 直接返回 true
			if (now < peerCheckNextMS) return peerPing;
			// 如果正在等回包，则为超时，干掉 Peer 并返回 false（ 网络正常情况下会在 peerNextCheckTime 前收到回包并 修改 peerChecking 为 false ）
			if (peerChecking) {
				peer->Dispose();
				peer.reset();
				peerChecking = false;
				return -1;
			}
			// 发送当前 ms 并设置正在发送标志，计算下一个检测时间点
			peer->SendCommand("ping", now);
			peerChecking = true;
			peerCheckNextMS = now + peerCheckIntervalMS;
			return peerPing;
		}

		// 直接干掉 peer
		inline void PeerDispose() {
			if (peer) {
				peer->Dispose();
				peer.reset();
			}
		}
	};








	// 主要提供给 lua 用, 发送的数据为 serial + bbuffer
	// 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvSerialBBufferSimulatePeer : UvSimulatePeer {
		using UvSimulatePeer::UvSimulatePeer;
		Dict<int, std::pair<std::function<int(BBuffer* const& data)>, int64_t>> callbacks;

		std::function<int(BBuffer& data)> onReceivePush;
		inline int ReceivePush(BBuffer& data) noexcept { return onReceivePush ? onReceivePush(data) : 0; };
		std::function<int(int const& serial, BBuffer& data)> onReceiveRequest;
		inline int ReceiveRequest(int const& serial, BBuffer& data) noexcept { return onReceiveRequest ? onReceiveRequest(serial, data) : 0; };

		inline int SendPush(BBuffer const& data) {
			return SendResponse(0, data);
		}

		inline int SendResponse(int32_t const& serial, BBuffer const& data) {
			auto&& gp = gatewayPeer.lock();
			if (!gp) return -1;
			return gp->SendTo(id, serial, data);
		}

		inline int SendRequest(BBuffer const& data, std::function<int(BBuffer* const& data)>&& cb, uint64_t const& timeoutMS = 0) {
			if (Disposed()) return -1;
			std::pair<std::function<int(BBuffer * const& data)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			v.second = NowSteadyEpochMS() + (timeoutMS ? timeoutMS : uv.defaultRequestTimeoutMS);
			if (int r = SendResponse(-serial, data)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline virtual bool Dispose(int const& flag = 1) noexcept override {
			if (Disposed()) return false;
			xx::UvItem_s holder;
			if (flag != -1) {
				holder = shared_from_this();
			}
			this->UvSimulatePeer::Dispose(flag);
			if (flag == -1) return true;
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			onReceivePush = nullptr;
			onReceiveRequest = nullptr;
			return true;
		}

	protected:
		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			auto& recvBB = uv.recvBB;
			recvBB.Reset(recvBuf, recvLen);

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;

			// 令 recvBB 只含有数据部分
			recvBB.Reset(recvBuf + recvBB.offset, recvLen - recvBB.offset);

			if (serial == 0) {
				return ReceivePush(recvBB);
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, recvBB);
			}
			else {
				auto&& idx = callbacks.Find(serial);
				if (idx == -1) return 0;
				auto a = std::move(callbacks.ValueAt(idx).first);
				callbacks.RemoveAt(idx);
				return a(&recvBB);
			}
		}

		inline virtual int Update(int64_t const& nowMS) noexcept override {
			if (Disposed()) return -1;

			if (timeoutMS && timeoutMS < nowMS) {
				Dispose();
				return -1;
			}

			for (auto&& iter = callbacks.begin(); iter != callbacks.end(); ++iter) {
				auto&& v = iter->value;
				if (v.second < nowMS) {
					auto a = std::move(v.first);
					iter.Remove();
					a(nullptr);
				}
			}

			return 0;
		}
	};
	using UvSerialBBufferSimulatePeer_s = std::shared_ptr<UvSerialBBufferSimulatePeer>;






	// 与 gateway 接洽的服务基类
	// 使用方法：继承并覆盖 AcceptSimulatePeer. 之后 InitGatewayListener( listenIP, port )
	template<typename PeerType = xx::UvSimulatePeer, bool printLog = false>
	struct UvServiceBase {
		xx::Uv uv;

		// 创建虚拟 peer 事件逻辑
		virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& sp) = 0;

		// 如果需要做接入过滤可覆盖此函数( 当前设计中只有 0 号服务才会收到 accept 通知 )
		virtual int ConnectIncomming(uint32_t const& clientId, std::string const& ip) { return 0; };

		UvServiceBase() = default;
		virtual ~UvServiceBase() {}
		UvServiceBase(UvServiceBase const&) = delete;
		UvServiceBase& operator=(UvServiceBase const&) = delete;

		// gateway 专用监听器
		std::shared_ptr<xx::UvFromGatewayListener> gatewayListener;

		// key: gatewayId
		std::unordered_map<uint32_t, std::shared_ptr<xx::UvFromGatewayPeer>> gatewayPeers;

		// 初始化网关监听器. 成功返回 0
		int InitGatewayListener(char const* const& ip, int const& port) {
			xx::TryMakeTo(gatewayListener, uv, ip, port);
			if (!gatewayListener) return -1;
			gatewayListener->onAccept = [this](xx::UvPeer_s peer) {
				auto&& gp = xx::As<xx::UvFromGatewayPeer>(peer);
				gp->ResetTimeoutMS(10L * 1000L);
				gp->onReceiveCommand = [this, gp](xx::BBuffer& bb)->int {
					std::string cmd;
					if (int r = bb.Read(cmd)) return r;
					if (cmd == "gatewayId") {
						uint32_t gatewayId = 0;
						if (int r = bb.Read(gatewayId)) return r;

						// gatewayId 已存在: 已注册, 直接断开新连接
						if (gp->gatewayId != 0xFFFFFFFFu) return -1;
						auto&& iter = gatewayPeers.find(gatewayId);
						if (iter != gatewayPeers.end()) return -2;

						// 注册在案
						gp->gatewayId = gatewayId;
						gatewayPeers.emplace(gatewayId, gp);

						if constexpr (printLog) {
							xx::CoutN("UvFromGatewayPeer recv cmd gatewayId: gatewayId = ", gatewayId);
						}
						return 0;
					}
					
					else if (cmd == "disconnect") {
						uint32_t clientId = 0;
						if (int r = bb.Read(clientId)) return r;

						// 及时断开 特定peer
						gp->DisconnectSimulatePeer(clientId);

						if constexpr (printLog) {
							xx::CoutN("UvFromGatewayPeer recv cmd disconnect: clientId = ", clientId);
						}
						return 0;
					}					
					else if (cmd == "ping") {
						int64_t ticks = 0;
						if (int r = bb.Read(ticks)) return r;
						gp->ResetTimeoutMS(10L * 1000L);
						return gp->SendCommand_Ping(ticks);
					}

					else if (cmd == "accept") {
						uint32_t clientId = 0;
						std::string ip;
						if (int r = bb.Read(clientId, ip)) return r;

						if constexpr (printLog) {
							xx::CoutN("UvFromGatewayPeer recv cmd accept: clientId: ", clientId, ", ip = ", ip);
						}

						// 如果允许接入
						if (!ConnectIncomming(clientId, ip)) {
							// 打开当前服务到 client 的端口
							gp->SendCommand_Open(clientId);

							// 创建虚拟 peer ( 如果已存在就会被顶下线 )
							auto&& cp = gp->CreateSimulatePeer<PeerType>(clientId);
							AcceptSimulatePeer(cp);
						}
						return 0;
					}

					else {
						return -3;
					}
					return 0;
				};

				gp->onReceive = [this, gp](uint32_t const& id, uint8_t* const& buf, size_t const& len)->int {
					auto&& iter = gp->simulatePeers.find(id);
					if (iter == gp->simulatePeers.end()
						|| !iter->second
						|| iter->second->Disposed()) {
						// 向 gp 发 close
						gp->SendCommand_Close(id);
						return 0;
					}
					int r = iter->second->HandlePack(buf, (uint32_t)len);
					if (r) {
						gp->DisconnectSimulatePeer(id);
						// 向 gp 发 close
						gp->SendCommand_Close(id);
					}
					return 0;
				};

				gp->onDisconnect = [this, gp] {
					if (gp->gatewayId != 0xFFFFFFFFu) {
						this->gatewayPeers.erase(gp->gatewayId);
					}

					// 及时断开 peers
					gp->DisconnectSimulatePeers();

					if constexpr (printLog) {
						xx::CoutN("UvFromGatewayPeer disconnected: ip = ", gp->GetIP(), ", gatewayId = ", gp->gatewayId);
					}
				};

				if constexpr (printLog) {
					xx::CoutN("gateway peer connected: ip = ", gp->GetIP());
				}
			};
			return 0;
		}
	};












	// 直连服务器专用
	// 主要提供给 lua 用, 发送的数据为 serial + bbuffer
	// 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvSerialBBufferPeer : UvPeer {
		using UvPeer::UvPeer;
		Dict<int, std::pair<std::function<int(BBuffer* const& data)>, int64_t>> callbacks;

		std::function<int(BBuffer & data)> onReceivePush;
		inline int ReceivePush(BBuffer& data) noexcept { return onReceivePush ? onReceivePush(data) : 0; };
		std::function<int(int const& serial, BBuffer & data)> onReceiveRequest;
		inline int ReceiveRequest(int const& serial, BBuffer& data) noexcept { return onReceiveRequest ? onReceiveRequest(serial, data) : 0; };

		inline int SendPush(BBuffer const& data) {
			return SendResponse(0, data);
		}
		inline int SendResponse(int32_t const& serial, BBuffer const& data) {
			auto&& bb = uv.sendBB;											// 借壳
			peerBase->SendPrepare(bb, data.len);							// 预处理
			bb.Write(serial);												// 写入 serial
			bb.AddRange(data.buf, data.len);								// 写入数据
			return peerBase->SendAfterPrepare(bb);
		}
		inline int SendRequest(BBuffer const& data, std::function<int(BBuffer* const& data)>&& cb, uint64_t const& timeoutMS = 0) {
			if (Disposed()) return -1;
			std::pair<std::function<int(BBuffer* const& data)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			v.second = NowSteadyEpochMS() + (timeoutMS ? timeoutMS : uv.defaultRequestTimeoutMS);
			if (int r = SendResponse(-serial, data)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline virtual bool Dispose(int const& flag = 1) noexcept override {
			if (Disposed()) return false;
			xx::UvItem_s holder;
			if (flag != -1) {
				holder = shared_from_this();
			}
			this->UvPeer::Dispose(flag);
			if (flag == -1) return true;
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			onReceivePush = nullptr;
			onReceiveRequest = nullptr;
			return true;
		}

	protected:
		inline virtual int HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
			auto& recvBB = uv.recvBB;
			recvBB.Reset(recvBuf, recvLen);

			int serial = 0;
			if (int r = recvBB.Read(serial)) return r;

			// 令 recvBB 只含有数据部分
			recvBB.Reset(recvBuf + recvBB.offset, recvLen - recvBB.offset);

			if (serial == 0) {
				return ReceivePush(recvBB);
			}
			else if (serial < 0) {
				return ReceiveRequest(-serial, recvBB);
			}
			else {
				auto&& idx = callbacks.Find(serial);
				if (idx == -1) return 0;
				auto a = std::move(callbacks.ValueAt(idx).first);
				callbacks.RemoveAt(idx);
				return a(&recvBB);
			}
		}

		// 因为重新定义了新的 callbacks 类型 所以需要
		// 代码直接从基类复制不修改. 
		inline virtual int Update(int64_t const& nowMS) noexcept override {
			assert(peerBase);
			if (timeoutMS && timeoutMS < nowMS) {
				Dispose();
				return -1;
			}

			if (int r = peerBase->Update(nowMS)) return r;

			for (auto&& iter = callbacks.begin(); iter != callbacks.end(); ++iter) {
				auto&& v = iter->value;
				if (v.second < nowMS) {
					auto a = std::move(v.first);
					iter.Remove();
					a(nullptr);
				}
			}

			return 0;
		}
	};

	using UvSerialBBufferPeer_s = std::shared_ptr<UvSerialBBufferPeer>;

}
