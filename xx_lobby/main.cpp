#include <xx_epoll2.hpp>

namespace xx::Epoll {
	// ���� peer ( ͨ��������· peer ���� )
	struct SimulatePeer;

	// ������ǿ��. ����һЩ���ú���
	struct TcpPeerEx : TcpPeer {
		// �����¼�
		std::function<void()> onDisconnect;

		// �����û��󶨵��¼�����
		virtual void OnDisconnect(int const& reason) override {
			if (onDisconnect) {
				onDisconnect();
			}
		}

		// ��ʼ�� bb д��. �ճ� ���� ͷ��
		static void WritePackageBegin(xx::BBuffer& bb, size_t const& cap, uint32_t const& id) {
			bb.Reserve(4 + cap);
			bb.len = 4;
			bb.WriteFixed(id);
		}

		// ���� bb д�����������ݳ��� ��д ��ͷ
		static void WritePackageEnd(xx::BBuffer& bb) {
			*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);
		}
	};

	// �� gateway ��������· peer
	struct FromToGatewayPeer : TcpPeerEx {
		// �����ڲ�ָ���( 4 �ֽڳ��� + 0xFFFFFFFF + ����( ͨ��Ϊ cmd string + args... ) )
		std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceiveCommand;

		// ����һ�����ݰ�( 4 �ֽڳ��� + 4�ֽڵ�ַ + ���� )
		std::function<void(uint32_t const& id, uint8_t* const& buf, std::size_t const& len)> onReceivePackage;

		// �����ڲ�ָ���. cmd string + args...
		template<typename...Args>
		void SendCommand(Args const& ... cmdAndArgs) {
			auto&& bb = ep->sendBB;
			WritePackageBegin(bb, 64, 0xFFFFFFFFu);
			bb.Write(cmdAndArgs...);
			WritePackageEnd(bb);
			Send(bb);
		}

		// ����������( ���� header + id + data �Ľṹ )
		inline void SendTo(uint32_t const& id, int32_t const& serial, Object_s const& msg) {
			auto&& bb = ep->sendBB;
			WritePackageBegin(bb, 1024, id);
			bb.Write(serial);
			bb.WriteRoot(msg);
			WritePackageEnd(bb);
			Send(bb);
		}

		// ���ݽ����¼�: �� recv ������, ���� id ��λ�� SimulatePeer ��һ�� call �� OnReceive
		virtual void OnReceive() override {
			// ȡ��ָ�뱸��
			auto buf = (uint8_t*)this->recv.buf;
			auto end = (uint8_t*)this->recv.buf + this->recv.len;

			// ȷ����ͷ���ȳ���
			while (buf + 4 <= end) {

				// ȡ������������ �� �жϺϷ���
				auto dataLen = *(uint32_t*)buf;

				// dataLen ����Ӧ���� 4 �ֽ� id ��
				if (dataLen < 4 || dataLen > this->ep->maxPackageLength) {
					OnDisconnect(-5);
					Dispose();
					return;
				}

				// ���������������˳�
				if (buf + 4 + dataLen > end) break;

				// ������������ʼ���ô���ص�
				buf += 4;
				{
					// �����жϱ���
					Ref<Item> alive(this);

					// �ж��Ƿ�Ϊ�ڲ�ָ��
					if (*(uint32_t*)buf == 0xFFFFFFFFu) {
						onReceiveCommand(buf + 4, dataLen - 4);
					}
					else {
						onReceivePackage(*(uint32_t*)buf, buf + 4, dataLen - 4);
					}

					// �����ǰ��ʵ������ɱ���˳�
					if (!alive) return;
				}
				// ������һ�����Ŀ�ͷ
				buf += dataLen;
			}

			// �Ƴ����Ѵ��������
			this->recv.RemoveFront((char*)buf - this->recv.buf);
		}
	};

	// service �� gateway ���Ϻ����
	struct FromGatewayPeer : FromToGatewayPeer {
		using FromToGatewayPeer::FromToGatewayPeer;

		// �ڲ����ر��, �������Ӻ�ͨ���
		uint32_t gatewayId = 0xFFFFFFFFu;

		// ���ڵ�ǰ peer ��ͨ�ŵ����� peers
		std::unordered_map<uint32_t, Ref<SimulatePeer>> simulatePeers;

		// ���ڼ������� sim peers Update
		Timer_r simulatePeersUpdater;

		// ��ʼ�� timer ɶ��
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

	// service ���� gateways ר��
	struct FromGatewayListener : TcpListener {
		inline virtual TcpPeer_u OnCreatePeer() noexcept override {
			return xx::TryMakeU<FromGatewayPeer>();
		}
	};

	// ���� peer ( ͨ��������· peer ���� )
	struct SimulatePeer : ItemTimeout {
		// ָ��������( ����ʱ��ֵ )
		Ref<FromToGatewayPeer> gatewayPeer;

		// ��Ŵ� gateway ���䵽������ id( ����ʱ��ֵ )
		uint32_t id = 0xFFFFFFFFu;

		// ��� SendRequest �Ļص�
		Dict<int, std::pair<std::function<int(Object_s && msg)>, int64_t>> callbacks;

		// ������������ SendRequest ��ҵ���
		int serial = 0;

		// �Ͽ�ʱ����
		std::function<void()> onDisconnect;

		// �յ� push ��ʱ����
		std::function<void(Object_s && msg)> onReceivePush;

		// �յ� request ��ʱ����
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

		// ÿ֡���ص��Ƿ�ʱ
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

	// �� gateway ��Ǣ�ķ������
	// ʹ�÷������̳в����� AcceptSimulatePeer. ֮�� InitGatewayListener( listenIP, port )
	struct ServiceBase {
		Context ep;
		// �������� peer �¼��߼�
		virtual void AcceptSimulatePeer(Ref<SimulatePeer>& sp) = 0;

		// �����Ҫ��������˿ɸ��Ǵ˺���( ��ǰ�����ֻ�� 0 �ŷ���Ż��յ� accept ֪ͨ )
		virtual int ConnectIncomming(uint32_t const& clientId, std::string const& ip) { return 0; };

		ServiceBase() = default;
		virtual ~ServiceBase() {}
		ServiceBase(ServiceBase const&) = delete;
		ServiceBase& operator=(ServiceBase const&) = delete;

		// gateway ר�ü�����
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
//	// ��ǰ�����id ( ������������û������� )
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
//	// ������ timer
//	std::shared_ptr<xx::UvTimer> service0DialTimer;
//
//	// ������
//	std::shared_ptr<xx::UvToServiceDialer> service0Dialer;
//
//	// �� service0 ������
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
//					// ��λ��Ŀ�� gateway peer
//					auto&& iter = gatewayPeers.find(gatewayId);
//					if (iter == gatewayPeers.end()) return 0;
//					auto&& gp = iter->second;
//					if (!gp || gp->Disposed()) return 0;
//
//					// �������� peer ( ����Ѵ��ھͻᱻ������ )
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
