#include <xx_epoll2.hpp>

namespace xx::Epoll {
	// ���� peer ( ͨ��������· peer ���� )
	struct SimulatePeer;

	// ��ǿ��. ֧�� 4�ֽ� ���� ��ͷ���. ֧�ַ��� Object ����
	struct TcpPeerEx : TcpPeer {
		// �����¼�
		std::function<void()> onDisconnect;

		// �������߼����Ǳ�д
		virtual void OnReceivePackage(uint8_t* const& buf, uint32_t const& len) = 0;

		// �����ڲ�ָ���. cmd string + args...
		template<typename...Args>
		void SendCommand(Args const& ... cmdAndArgs) {
			auto&& bb = ep->sendBB;
			bb.Reserve(64);
			bb.len = 4;	// �ճ� ���� ͷ��
			bb.WriteFixed(0xFFFFFFFFu);
			bb.Write(cmdAndArgs...);
			*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);	// ������ݳ��ȵ���ͷ
			this->TcpPeer::Send(bb);
		}

		// ����������( ���� header + id + data �Ľṹ. ��� id �� 0 �򲻰��� id ����. �����ڷ���֮��ͨ�� )
		inline void Send(int32_t const& serial, Object_s const& msg, uint32_t const& id = 0) {
			auto&& bb = ep->sendBB;
			bb.Reserve(1024);
			bb.len = 4;	// �ճ� ���� ͷ��
			if (id) {
				bb.WriteFixed(id);
			}
			bb.Write(serial);
			bb.WriteRoot(msg);
			*(uint32_t*)bb.buf = (uint32_t)(bb.len - 4);	// ������ݳ��ȵ���ͷ
			this->TcpPeer::Send(bb);
		}

		// ���ݽ����¼�: �� recv ������, ���� id ��λ�� SimulatePeer ��һ�� call �� OnReceive
		virtual void OnReceive() override {
			// ȡ��ָ�뱸��
			auto buf = (uint8_t*)this->recv.buf;
			auto end = (uint8_t*)this->recv.buf + this->recv.len;

			// �����жϱ���
			Ref<Item> alive(this);

			// ȷ����ͷ���ȳ���
			while (buf + 4 <= end) {
				// ȡ������������ �� �жϺϷ���
				auto dataLen = *(uint32_t*)buf;

				// ���ȱ����ж�
				if (dataLen > this->ep->maxPackageLength) {
					OnDisconnect(-5);
					// �����ǰ��ʵ������ɱ���˳�
					if (!alive) return;
					Dispose();
					return;
				}

				// ���������������˳�
				if (buf + 4 + dataLen > end) break;

				// ������������ʼ���ô���ص�
				buf += 4;
				{
					// �ַ�
					OnReceivePackage(buf, dataLen);

					// �����ǰ��ʵ������ɱ���˳�
					if (!alive) return;
				}
				// ������һ�����Ŀ�ͷ
				buf += dataLen;
			}

			// �Ƴ����Ѵ��������
			this->recv.RemoveFront((char*)buf - this->recv.buf);
		}

		// �����û��󶨵��¼�����
		virtual void OnDisconnect(int const& reason) override {
			if (onDisconnect) {
				onDisconnect();
			}
		}
	};

	// �� gateway ��������· peer
	struct FromToGatewayPeer : TcpPeerEx {
		// �����ڲ�ָ���( 4 �ֽڳ��� + 0xFFFFFFFF + ����( ͨ��Ϊ cmd string + args... ) )
		std::function<void(uint8_t* const& buf, std::size_t const& len)> onReceiveCommand;

		// ����һ�����ݰ�( 4 �ֽڳ��� + 4�ֽڵ�ַ + ���� )
		std::function<void(uint32_t const& id, uint8_t* const& buf, std::size_t const& len)> onReceivePackage;

		// ָ����������( �������Ӻ�ͨ��� )
		std::unordered_map<uint32_t, FromToGatewayPeer*>* gatewayPeers = nullptr;

		// �ڲ����ر��, �������Ӻ�ͨ���
		uint32_t gatewayId = 0xFFFFFFFFu;

		// ���ڵ�ǰ peer ��ͨ�ŵ����� peers. key Ϊ id
		// ���� sp ����ʱ��������Ƴ�, ��ָ��һ����Ч
		std::unordered_map<uint32_t, SimulatePeer*> simulatePeers;

		// ���ڼ������� sim peers Update
		Timer_r simulatePeersUpdater;

		// ��ʼ�� timer
		FromToGatewayPeer();

		// ���� timer
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

		// ���ṹ: header + id + data. Ͷ�ݵ��˵Ĳ���Ϊ id + data
		virtual void OnReceivePackage(uint8_t* const& buf, uint32_t const& len) override {
			// dataLen ����Ӧ���� 4 �ֽ� id ��
			if (len < 4) {
				Dispose();
				return;
			}
			// �ж��Ƿ�Ϊ�ڲ�ָ��
			if (*(uint32_t*)buf == 0xFFFFFFFFu) {
				onReceiveCommand(buf + 4, len - 4);
			}
			else {
				onReceivePackage(*(uint32_t*)buf, buf + 4, len - 4);
			}
		}
	};

	// service ���� gateways ר��
	struct FromGatewayListener : TcpListener {
		inline virtual TcpPeer_u OnCreatePeer() noexcept override {
			return xx::TryMakeU<FromToGatewayPeer>();
		}
	};

	// ���� peer ( ͨ��������· peer ���� )
	struct SimulatePeer : ItemTimeout {
		// ָ��������. ���� gp ����ʱ�� Dispose ���� SimulatePeer, �ʸ�ָ��һ����Ч( ����ʱ��ֵ )
		FromToGatewayPeer* gatewayPeer = nullptr;

		// ��Ŵ� gateway ���䵽������ id( ����ʱ��ֵ )
		uint32_t id = 0xFFFFFFFFu;

		// ��� SendRequest �Ļص�
		typedef std::function<int(Object_s && msg)> RequestCB;
		std::unordered_map<int, std::pair<RequestCB, int64_t>> callbacks;

		// ������������ SendRequest ��ҵ���
		int serial = 0;

		// �Ͽ�ʱ����
		std::function<void()> onDisconnect;

		// �յ� push ��ʱ����
		std::function<void(Object_s && msg)> onReceivePush;

		// �յ� request ��ʱ����
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
			gatewayPeer->Send(-serial, msg, id);	// serial ȡ��ֵ����
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

		// ÿ֡���ص��Ƿ�ʱ
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
		// todo: remove from gatewayPeers when FromToGatewayPeer Dispose
		std::unordered_map<uint32_t, FromToGatewayPeer*> gatewayPeers;
	};

	inline FromToGatewayPeer::FromToGatewayPeer() {
		simulatePeersUpdater = ep->CreateTimer(10, [this](auto t) {
			for (auto&& iter = simulatePeers.begin(); iter != simulatePeers.end();) {
				// �������������´� simulatePeers erase �������� iter ʧЧ. ������ next
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
