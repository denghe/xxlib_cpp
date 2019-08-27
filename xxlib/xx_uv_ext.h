#pragma once
#include "xx_uv.h"
namespace xx {

	// gateway 相关 peer 基类
	// 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvCommandPeer : UvPeer {
		using UvPeer::UvPeer;

		// 当收到内部指令包时 投递到该事件回调( bb 为 uv.recvBB )
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
		using UvCommandPeer::UvCommandPeer;

		std::function<int(uint32_t const& id, uint8_t* const& buf, size_t const& len)> onReceive;

		std::unordered_map<uint32_t, std::shared_ptr<UvSimulatePeer>> simulatePeers;

		// 这段代码从 UvPeer 复制
		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!peerBase) return;
			peerBase.reset();
			timer.reset();
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();
				onDisconnect = nullptr;

				onReceive = nullptr;			//
				onReceiveCommand = nullptr;		//
			}
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

		// 下面代码抄自 UvPeer 并小改. peerBase 变为了 gatewayPeer

		Dict<int, std::pair<std::function<int(Object_s&& msg)>, int64_t>> callbacks;
		int serial = 0;
		int64_t timeoutMS = 0;
		UvTimer_s timer;
		std::function<void()> onDisconnect;
		std::function<int(Object_s&& msg)> onReceivePush;
		std::function<int(int const& serial, Object_s&& msg)> onReceiveRequest;


		UvSimulatePeer(std::shared_ptr<UvFromToGatewayBasePeer> gp, uint32_t const& id)
			: UvItem(gp->uv)
			, id(id) {
			gatewayPeer = gp;
			// todo: 将就用 gp 的 timer, gp 的 Update 增加对 sim peers 的 Update 调用
			MakeTo(timer, uv, 10, 10, [this] {
				Update(NowSteadyEpochMS());
				});
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
			if (id == 0xFFFFFFFFu) return -1;
			std::pair<std::function<int(Object_s && msg)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			if (timeoutMS) {
				v.second = NowSteadyEpochMS() + timeoutMS;
			}
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

		// call by timer
		inline virtual int Update(int64_t const& nowMS) noexcept {
			if (id == 0xFFFFFFFFu) return -1;

			if (timeoutMS && timeoutMS < nowMS) {
				Dispose(1);
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
			return id == 0xFFFFFFFFu;
		}

		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (id == 0xFFFFFFFFu) return;
			gatewayPeer.reset();
			timer.reset();
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();
				onDisconnect = nullptr;
				onReceivePush = nullptr;
				onReceiveRequest = nullptr;
			}
		}
	};

	inline void UvFromToGatewayBasePeer::DisconnectSimulatePeers() {
		for (auto&& p : simulatePeers) {
			if (p.second) {
				p.second->Disconnect();	// todo: 如果有从 simulatePeers 移除的代码 就不安全
			}
		}
		simulatePeers.clear();
	}

	inline void UvFromToGatewayBasePeer::DisconnectSimulatePeer(uint32_t const& id) {
		auto&& iter = simulatePeers.find(id);
		if (iter == simulatePeers.end()) return;
		if (iter->second) {
			iter->second->Disconnect();
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
				iter->second->Disconnect();
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

		std::shared_ptr<UvFromToGatewayBasePeer> peer;
		std::function<void(std::shared_ptr<SimulatePeerType>&)> onAcceptSimulatePeer;

		inline virtual UvPeer_s CreatePeer() noexcept override {
			return TryMake<UvFromToGatewayBasePeer>(uv);
		}

		inline virtual void Accept(UvPeer_s peer_) noexcept override {
			if (!peer_) return;

			peer = As<UvFromToGatewayBasePeer>(peer_);

			peer->onReceiveCommand = [this](BBuffer& bb)->int {
				peer->ResetTimeoutMS(5000);
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;
				if (cmd == "open") {
					uint32_t serviceId = 0;
					if (int r = bb.Read(serviceId)) return r;

					auto&& cp = peer->CreateSimulatePeer<SimulatePeerType>(serviceId);
					onAcceptSimulatePeer(cp);

					//CoutN("UvToGatewayPeer recv cmd open: serviceId = ", serviceId);
					return 0;
				}

				else if (cmd == "close") {
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					peer->DisconnectSimulatePeer(clientId);

					//CoutN("UvToGatewayPeer recv cmd disconnect: clientId = ", clientId);
					return 0;
				}

				else {
					return -3;
				}
				return 0;
			};

			peer->onReceive = [this](uint32_t const& id, uint8_t* const& buf, size_t const& len)->int {
				peer->ResetTimeoutMS(5000);
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

			peer->ResetTimeoutMS(5000);
		}

		inline bool PeerAlive() {
			return peer && !peer->Disposed();
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
			if (timeoutMS) {
				v.second = NowSteadyEpochMS() + timeoutMS;
			}
			if (int r = SendResponse(-serial, data)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (id == 0xFFFFFFFFu) return;
			gatewayPeer.reset();
			timer.reset();
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();
				onDisconnect = nullptr;
				onReceivePush = nullptr;
				onReceiveRequest = nullptr;
			}
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
			if (id == 0xFFFFFFFFu) return -1;

			if (timeoutMS && timeoutMS < nowMS) {
				Dispose(1);
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


	// 主要提供给 lua 用, 发送的数据为 serial + bbuffer
	// 基类的 SendXxxx, onReceiveXxxxx 不要用
	struct UvSerialBBufferPeer : UvPeer {
		using UvPeer::UvPeer;
		Dict<int, std::pair<std::function<int(BBuffer* const& data)>, int64_t>> callbacks;

		std::function<int(BBuffer& data)> onReceivePush;
		inline int ReceivePush(BBuffer& data) noexcept { return onReceivePush ? onReceivePush(data) : 0; };
		std::function<int(int const& serial, BBuffer& data)> onReceiveRequest;
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
			std::pair<std::function<int(BBuffer * const& data)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;			// uint circle use
			if (timeoutMS) {
				v.second = NowSteadyEpochMS() + timeoutMS;
			}
			if (int r = SendResponse(-serial, data)) return r;
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
			return 0;
		}

		inline virtual void Dispose(int const& flag = 1) noexcept override {
			if (!peerBase) return;
			peerBase.reset();
			timer.reset();
			for (auto&& kv : callbacks) {
				kv.value.first(nullptr);
			}
			callbacks.Clear();
			if (flag) {
				auto holder = shared_from_this();
				Disconnect();
				onDisconnect = nullptr;
				onReceivePush = nullptr;
				onReceiveRequest = nullptr;
			}
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
				Dispose(1);
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
