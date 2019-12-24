#include <xx_epoll2.hpp>

namespace xx::Epoll {
	// 虚拟 peer ( 通过网关链路 peer 产生 )
	struct SimulatePeer;

	// 基类增强类. 放置一些公用函数
	struct TcpPeerEx : TcpPeer {
		// 断线事件
		std::function<void()> onDisconnect;

		// 调用用户绑定的事件处理
		virtual void OnDisconnect(int const& reason) override {
			if (onDisconnect) {
				onDisconnect();
			}
		}

		// 开始向 bb 写包. 空出 长度 头部
		static void WritePackageBegin(xx::BBuffer& bb, size_t const& cap, uint32_t const& id) {
			bb.Reserve(4 + cap);
			bb.len = 4;
			bb.WriteFixed(id);
		}

		// 结束 bb 写包。根据数据长度 填写 包头
		static void WritePackageEnd(xx::BBuffer& bb) {
			*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);
		}
	};

	// 与 gateway 交互的链路 peer
	struct FromToGatewayPeer : TcpPeerEx {
		// 处理内部指令包( 4 字节长度 + 0xFFFFFFFF + 内容( 通常为 cmd string + args... ) )
		std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceiveCommand;

		// 处理一般数据包( 4 字节长度 + 4字节地址 + 数据 )
		std::function<void(uint32_t const& id, uint8_t* const& buf, std::size_t const& len)> onReceivePackage;

		// 构造内部指令包. cmd string + args...
		template<typename...Args>
		void SendCommand(Args const& ... cmdAndArgs) {
			auto&& bb = ep->sendBB;
			WritePackageBegin(bb, 64, 0xFFFFFFFFu);
			bb.Write(cmdAndArgs...);
			WritePackageEnd(bb);
			Send(bb);
		}

		// 构造正常包( 符合 header + id + data 的结构 )
		inline void SendTo(uint32_t const& id, int32_t const& serial, Object_s const& msg) {
			auto&& bb = ep->sendBB;
			WritePackageBegin(bb, 1024, id);
			bb.Write(serial);
			bb.WriteRoot(msg);
			WritePackageEnd(bb);
			Send(bb);
		}

		// 数据接收事件: 从 recv 拿数据, 根据 id 定位到 SimulatePeer 进一步 call 其 OnReceive
		virtual void OnReceive() override {
			// 取出指针备用
			auto buf = (uint8_t*)this->recv.buf;
			auto end = (uint8_t*)this->recv.buf + this->recv.len;

			// 确保包头长度充足
			while (buf + 4 <= end) {

				// 取出数据区长度 并 判断合法性
				auto dataLen = *(uint32_t*)buf;

				// dataLen 至少应包含 4 字节 id 长
				if (dataLen < 4 || dataLen > this->ep->maxPackageLength) {
					OnDisconnect(-5);
					Dispose();
					return;
				}

				// 数据区不完整就退出
				if (buf + 4 + dataLen > end) break;

				// 跳到数据区开始调用处理回调
				buf += 4;
				{
					// 死亡判断变量
					Ref<Item> alive(this);

					// 判断是否为内部指令
					if (*(uint32_t*)buf == 0xFFFFFFFFu) {
						onReceiveCommand(buf + 4, dataLen - 4);
					}
					else {
						onReceivePackage(*(uint32_t*)buf, buf + 4, dataLen - 4);
					}

					// 如果当前类实例已自杀则退出
					if (!alive) return;
				}
				// 跳到下一个包的开头
				buf += dataLen;
			}

			// 移除掉已处理的数据
			this->recv.RemoveFront((char*)buf - this->recv.buf);
		}
	};

	// service 被 gateway 连上后产生
	struct FromGatewayPeer : FromToGatewayPeer {
		using FromToGatewayPeer::FromToGatewayPeer;

		// 内部网关编号, 建立连接后沟通填充
		uint32_t gatewayId = 0xFFFFFFFFu;

		// 挂在当前 peer 下通信的虚拟 peers
		std::unordered_map<uint32_t, Ref<SimulatePeer>> simulatePeers;

		// 用于级联调用 sim peers Update
		Timer_r simulatePeersUpdater;

		// 初始化 timer 啥的
		FromGatewayPeer();
		~FromGatewayPeer();

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
	};

	// service 监听 gateways 专用
	struct FromGatewayListener : TcpListener {
		inline virtual TcpPeer_u OnCreatePeer() noexcept override {
			return xx::TryMakeU<FromGatewayPeer>();
		}
	};

	// 虚拟 peer ( 通过网关链路 peer 产生 )
	struct SimulatePeer : ItemTimeout {
		// 指向总容器( 创建时赋值 )
		Ref<FromToGatewayPeer> gatewayPeer;

		// 存放从 gateway 分配到的自增 id( 创建时赋值 )
		uint32_t id = 0xFFFFFFFFu;

		// 存放 SendRequest 的回调
		Dict<int, std::pair<std::function<int(Object_s && msg)>, int64_t>> callbacks;

		// 用于自增生成 SendRequest 的业务号
		int serial = 0;

		// 断开时触发
		std::function<void()> onDisconnect;

		// 收到 push 包时触发
		std::function<void(Object_s && msg)> onReceivePush;

		// 收到 request 包时触发
		std::function<void(int const& serial, Object_s && msg)> onReceiveRequest;

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
			SendResponse(0, msg);
		}

		inline void SendResponse(int32_t const& serial, Object_s const& msg) noexcept {
			if (auto&& gp = gatewayPeer.Lock()) {
				gp->SendTo(id, serial, msg);
			}
			else {
				assert(false);
			}
		}

		inline void SendRequest(Object_s const& msg, std::function<int(Object_s && msg)>&& cb, uint64_t const& timeoutMS) noexcept {
			std::pair<std::function<int(Object_s && msg)>, int64_t> v;
			serial = (serial + 1) & 0x7FFFFFFF;
			v.second = NowSteadyEpochMS() + (timeoutMS ? timeoutMS : ep->defaultRequestTimeoutMS);
			SendResponse(-serial, msg);
			v.first = std::move(cb);
			callbacks[serial] = std::move(v);
		}

		inline virtual void HandlePack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept {
			auto& bb = ep->recvBB;
			bb.Reset((uint8_t*)recvBuf, recvLen);

			int serial = 0;
			Object_s msg;
			if (bb.Read(serial) || bb.ReadRoot(msg)) {
				OnDisconnect();
				Dispose();
				return;
			}

			if (serial == 0) {
				OnReceivePush(std::move(msg));
			}
			else if (serial < 0) {
				OnReceiveRequest(-serial, std::move(msg));
			}
			else {
				auto&& idx = callbacks.Find(serial);
				if (idx == -1) return;
				auto a = std::move(callbacks.ValueAt(idx).first);
				callbacks.RemoveAt(idx);
				a(std::move(msg));
			}
		}

		// 每帧检查回调是否超时
		void Update() {
			for (auto&& iter = callbacks.begin(); iter != callbacks.end(); ++iter) {
				auto&& v = iter->value;
				if (v.second < ep->nowMS) {
					auto a = std::move(v.first);
					iter.Remove();
					a(nullptr);
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
		std::unordered_map<uint32_t, Ref<FromGatewayPeer>> gatewayPeers;
	};

	inline FromGatewayPeer::FromGatewayPeer() {
		simulatePeersUpdater = ep->CreateTimer(10, [this](auto t) {
			for (auto&& iter = simulatePeers.begin(); iter != simulatePeers.end();) {
				if (auto&& sp = iter->second.Lock()) {
					sp->Update();
					iter++;
				}
				else {
					simulatePeers.erase(iter++);
				}
			}
			t->SetTimeout(10);
		});
	}

	inline FromGatewayPeer::~FromGatewayPeer() {
		simulatePeersUpdater->Dispose();
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
