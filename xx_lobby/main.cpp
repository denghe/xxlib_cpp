#include <xx_epoll2.hpp>

namespace xx::Epoll {
	// 虚拟 peer ( 通过网关链路 peer 产生 )
	struct SimulatePeer;

	// 增强版. 支持 4字节 长度 包头拆包. 支持发送 Object 对象
	struct TcpPeerEx : TcpPeer {
		// 断线事件
		std::function<void()> onDisconnect;

		// 包处理逻辑覆盖编写
		virtual void OnReceivePackage(uint8_t* const& buf, uint32_t const& len) = 0;

		// 构造内部指令包. cmd string + args...
		template<typename...Args>
		void SendCommand(Args const& ... cmdAndArgs) {
			auto&& bb = ep->sendBB;
			bb.Reserve(64);
			bb.len = 4;	// 空出 长度 头部
			bb.WriteFixed(0xFFFFFFFFu);
			bb.Write(cmdAndArgs...);
			*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);	// 填充数据长度到包头
			this->TcpPeer::Send(bb);
		}

		// 构造正常包( 符合 header + id + data 的结构. 如果 id 传 0 则不包含 id 部分. 常见于服务之间通信 )
		inline void Send(int32_t const& serial, Object_s const& msg, uint32_t const& id = 0) {
			auto&& bb = ep->sendBB;
			bb.Reserve(1024);
			bb.len = 4;	// 空出 长度 头部
			if (id) {
				bb.WriteFixed(id);
			}
			bb.Write(serial);
			bb.WriteRoot(msg);
			*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);	// 填充数据长度到包头
			this->TcpPeer::Send(bb);
		}

		// 数据接收事件: 从 recv 拿数据, 根据 id 定位到 SimulatePeer 进一步 call 其 OnReceive
		virtual void OnReceive() override {
			// 取出指针备用
			auto buf = (uint8_t*)this->recv.buf;
			auto end = (uint8_t*)this->recv.buf + this->recv.len;

			// 死亡判断变量
			Ref<Item> alive(this);

			// 确保包头长度充足
			while (buf + 4 <= end) {
				// 取出数据区长度 并 判断合法性
				auto dataLen = *(uint32_t*)buf;

				// 长度保护判断
				if (dataLen > this->ep->maxPackageLength) {
					OnDisconnect(-5);
					// 如果当前类实例已自杀则退出
					if (!alive) return;
					Dispose();
					return;
				}

				// 数据区不完整就退出
				if (buf + 4 + dataLen > end) break;

				// 跳到数据区开始调用处理回调
				buf += 4;
				{
					// 分发
					OnReceivePackage(buf, dataLen);

					// 如果当前类实例已自杀则退出
					if (!alive) return;
				}
				// 跳到下一个包的开头
				buf += dataLen;
			}

			// 移除掉已处理的数据
			this->recv.RemoveFront((char*)buf - this->recv.buf);
		}

		// 调用用户绑定的事件处理
		virtual void OnDisconnect(int const& reason) override {
			if (onDisconnect) {
				onDisconnect();
			}
		}
	};

	// 与 gateway 交互的链路 peer
	struct FromToGatewayPeer : TcpPeerEx {
		// 处理内部指令包( 4 字节长度 + 0xFFFFFFFF + 内容( 通常为 cmd string + args... ) )
		std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceiveCommand;

		// 处理一般数据包( 4 字节长度 + 4字节地址 + 数据 )
		std::function<void(uint32_t const& id, uint8_t* const& buf, std::size_t const& len)> onReceivePackage;

		// 指向所在容器( 建立连接后沟通填充 )
		std::unordered_map<uint32_t, FromToGatewayPeer*>* gatewayPeers = nullptr;

		// 内部网关编号, 建立连接后沟通填充
		uint32_t gatewayId = 0xFFFFFFFFu;

		// 挂在当前 peer 下通信的虚拟 peers. key 为 id
		// 由于 sp 析构时会从这里移除, 故指针一定有效
		std::unordered_map<uint32_t, SimulatePeer*> simulatePeers;

		// 用于级联调用 sim peers Update
		Timer_r simulatePeersUpdater;

		// 初始化 timer
		FromToGatewayPeer();

		// 清理 timer
		~FromToGatewayPeer();

		void SendCommand_Open(uint32_t const& clientId) {
			SendCommand("open", clientId);
		}
		void SendCommand_Close(uint32_t const& clientId) {
			SendCommand("close", clientId);
		}
		void SendCommand_Kick(uint32_t const& clientId, int64_t const& delayMS) {
			SendCommand("kick", clientId, delayMS);
		}
		void SendCommand_Ping(int64_t const& ticks) {
			SendCommand("ping", ticks);
		}

		// 包结构: header + id + data. 投递到此的部分为 id + data
		virtual void OnReceivePackage(uint8_t* const& buf, uint32_t const& len) override {
			// dataLen 至少应包含 4 字节 id 长
			if (len < 4) {
				Dispose();
				return;
			}
			// 判断是否为内部指令
			if (*(uint32_t*)buf == 0xFFFFFFFFu) {
				onReceiveCommand(buf + 4, len - 4);
			}
			else {
				onReceivePackage(*(uint32_t*)buf, buf + 4, len - 4);
			}
		}
	};

	// service 监听 gateways 专用
	struct FromGatewayListener : TcpListener {
		inline virtual TcpPeer_u OnCreatePeer() noexcept override {
			return xx::TryMakeU<FromToGatewayPeer>();
		}
	};

	// 虚拟 peer ( 通过网关链路 peer 产生 )
	struct SimulatePeer : ItemTimeout {
		// 指向总容器. 由于 gp 析构时会 Dispose 所有 SimulatePeer, 故该指针一定有效( 创建时赋值 )
		FromToGatewayPeer* gatewayPeer = nullptr;

		// 存放从 gateway 分配到的自增 id( 创建时赋值 )
		uint32_t id = 0xFFFFFFFFu;

		// 存放 SendRequest 的回调
		typedef std::function<int(Object_s && msg)> RequestCB;
		std::unordered_map<int, std::pair<RequestCB, int64_t>> callbacks;

		// 用于自增生成 SendRequest 的业务号
		int serial = 0;

		// 断开时触发
		std::function<void()> onDisconnect;

		// 收到 push 包时触发
		std::function<void(Object_s && msg)> onReceivePush;

		// 收到 request 包时触发
		std::function<void(int const& serial, Object_s && msg)> onReceiveRequest;

		~SimulatePeer() {
			if (gatewayPeer && id != 0xFFFFFFFFu) {
				gatewayPeer->simulatePeers.erase(id);
			}
		}

		inline virtual void OnDisconnect() noexcept {
			if (onDisconnect) {
				onDisconnect();
			}
		}
		inline virtual void OnReceivePush(Object_s&& msg) noexcept {
			if (onReceivePush) {
				onReceivePush(std::move(msg));
			}
		}
		inline virtual void OnReceiveRequest(int const& serial, Object_s&& msg) noexcept {
			if (onReceiveRequest) {
				onReceiveRequest(serial, std::move(msg));
			}
		}

		inline void SendPush(Object_s const& msg) noexcept {
			gatewayPeer->Send(0, msg, id);
		}
		inline void SendResponse(int32_t const& serial, Object_s const& msg) noexcept {
			gatewayPeer->Send(serial, msg, id);
		}
		inline void SendRequest(Object_s const& msg, RequestCB&& cb, uint32_t const& timeoutMS) noexcept {
			serial = (serial + 1) & 0x7FFFFFFF;
			callbacks[serial] = { std::move(cb), ep->nowMS + (timeoutMS ? (int64_t)timeoutMS : ep->defaultRequestTimeoutMS) };
			gatewayPeer->Send(-serial, msg, id);	// serial 取负值发出
		}

		inline void OnReceivePackage(uint8_t* const& buf, uint32_t const& len) noexcept {
			auto& bb = ep->recvBB;
			bb.Reset((uint8_t*)buf, len);

			int serial = 0;
			Object_s msg;
			if (bb.Read(serial) || bb.ReadRoot(msg)) {
				Ref<Item> alive(this);
				OnDisconnect();
				if (!alive) return;
				Dispose();
				return;
			}
			else if (serial == 0) {
				OnReceivePush(std::move(msg));
			}
			else if (serial < 0) {
				OnReceiveRequest(-serial, std::move(msg));
			}
			else {
				auto&& iter = callbacks.find(serial);
				if (iter == callbacks.end()) return;
				auto&& a = std::move(iter->second.first);
				callbacks.erase(iter);
				a(std::move(msg));
			}
		}

		// 每帧检查回调是否超时
		void Update() {
			for (auto&& iter = callbacks.begin(); iter != callbacks.end();) {
				if (iter->second.second < ep->nowMS) {
					auto&& a = std::move(iter->second.first);
					iter = callbacks.erase(iter);
					a(nullptr);
				}
				else {
					++iter;
				}
			}
		}
	};

	// 与 gateway 接洽的服务基类
	// 使用方法：继承并覆盖 AcceptSimulatePeer. 之后 InitGatewayListener( listenIP, port )
	struct ServiceBase {
		Context ep;
		// 创建虚拟 peer 事件逻辑
		virtual void AcceptSimulatePeer(Ref<SimulatePeer>& sp) = 0;

		// 如果需要做接入过滤可覆盖此函数( 当前设计中只有 0 号服务才会收到 accept 通知 )
		virtual int ConnectIncomming(uint32_t const& clientId, std::string const& ip) { return 0; };

		ServiceBase() = default;
		virtual ~ServiceBase() {}
		ServiceBase(ServiceBase const&) = delete;
		ServiceBase& operator=(ServiceBase const&) = delete;

		// gateway 专用监听器
		Ref<FromGatewayListener> gatewayListener;

		// key: gatewayId
		// todo: remove from gatewayPeers when FromToGatewayPeer Dispose
		std::unordered_map<uint32_t, FromToGatewayPeer*> gatewayPeers;
	};

	inline FromToGatewayPeer::FromToGatewayPeer() {
		simulatePeersUpdater = ep->CreateTimer(10, [this](auto t) {
			for (auto&& iter = simulatePeers.begin(); iter != simulatePeers.end();) {
				// 可能因析构导致从 simulatePeers erase 进而导致 iter 失效. 故先算 next
				auto&& next = std::next(iter);
				iter->second->Update();
				iter = next;
			}
			t->SetTimeout(10);
		});
	}

	inline FromToGatewayPeer::~FromToGatewayPeer() {
		if (simulatePeersUpdater) {
			simulatePeersUpdater->Dispose();
		}
		if (gatewayPeers) {
			gatewayPeers->erase(gatewayId);
		}
	}
}

int main() {

	return 0;
}











//Lobby o;
//o.uv.Run();
//xx::CoutN("lobby: exit");

//struct Lobby : xx::UvServiceBase<xx::UvSimulatePeer> {
//
//	// 当前服务的id ( 可能填充自配置或者其他 )
//	uint32_t serviceId = 1;
//
//	Lobby() {
//		InitGatewayListener("0.0.0.0", 22222);
//		InitService0Dialer();
//	}
//
//	/***********************************************************************************************/
//	// service0
//	/***********************************************************************************************/
//
//	// 拨号用 timer
//	std::shared_ptr<xx::UvTimer> service0DialTimer;
//
//	// 拨号器
//	std::shared_ptr<xx::UvToServiceDialer> service0Dialer;
//
//	// 与 service0 的连接
//	std::shared_ptr<xx::UvFromToServicePeer> service0Peer;
//
//
//	void InitService0Dialer() {
//		xx::MakeTo(service0DialTimer, uv, 500, 500, [this] {
//			if (!service0Peer || service0Peer->Disposed()) {
//				if (!service0Dialer->Busy()) {
//					service0Dialer->Dial();
//				}
//			}
//			});
//
//		xx::MakeTo(service0Dialer, uv);
//		service0Dialer->ip = "127.0.0.1";
//		service0Dialer->port = 22222;
//
//		service0Dialer->onConnect = [this](xx::UvPeer_s peer) {
//			if (!peer) return;
//
//			service0Peer = xx::As<xx::UvFromToServicePeer>(peer);
//
//			service0Peer->onDisconnect = [this] {
//				service0Peer.reset();
//				xx::CoutN("service peer disconnect: ", service0Peer->GetIP());
//			};
//
//			service0Peer->onReceivePush = [this](xx::Object_s&& msg)->int {
//				auto&& bb = xx::As<xx::BBuffer>(msg);
//				if (!bb) return -1;
//				std::string cmd;
//				if (int r = bb->Read(cmd)) return r;
//				if (cmd == "accept") {
//					uint32_t gatewayId = 0;
//					uint32_t clientId = 0;
//					if (int r = bb->Read(gatewayId, clientId)) return r;
//
//					// 定位到目标 gateway peer
//					auto&& iter = gatewayPeers.find(gatewayId);
//					if (iter == gatewayPeers.end()) return 0;
//					auto&& gp = iter->second;
//					if (!gp || gp->Disposed()) return 0;
//
//					// 创建虚拟 peer ( 如果已存在就会被顶下线 )
//					auto&& cp = gp->CreateSimulatePeer<xx::UvSimulatePeer>(clientId);
//					AcceptSimulatePeer(cp);
//
//					xx::CoutN("service peer recv cmd accept: gatewayId = ", gatewayId, ", clientId: ", clientId);
//					return 0;
//				}
//
//				else {
//					return -3;
//				}
//				return 0;
//			};
//
//			service0Peer->SendPush_Command_ServiceId(serviceId);
//
//			xx::CoutN("service peer onConnect: ", service0Peer->GetIP());
//		};
//	}
//
//	virtual void AcceptSimulatePeer(std::shared_ptr<xx::UvSimulatePeer>& cp) override {
//		// echo
//		cp->onReceivePush = [this, cp](xx::Object_s&& msg)->int {
//			return cp->SendPush(msg);
//		};
//		cp->onDisconnect = [this, cp] {
//			xx::CoutN("UvSimulatePeer disconnected. clientId = ", cp->id);
//		};
//	}
//
//	// todo: listener ......
//};
