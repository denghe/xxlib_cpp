#include <xx_uv_ext.h>



/*
ʹ�÷������̳в����� Accept.  ����ִ�����к���( �������� ):
	InitGatewayListener("0.0.0.0", 22222);
*/

template<typename PeerType = xx::UvSimulatePeer>
struct UvServiceBase {
	UvServiceBase() = default;
	virtual ~UvServiceBase() {}
	UvServiceBase(UvServiceBase const&) = delete;
	UvServiceBase& operator=(UvServiceBase const&) = delete;

	xx::Uv uv;

	//cp->onDisconnect = 
	//cp->onReceivePush = 
	//cp->onReceiveRequest = 
	// ...
	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& cp) = 0;

	/***********************************************************************************************/
	// gateway
	/***********************************************************************************************/

	// ��ǰ�����id ( ������������û������� )
	uint32_t serviceId = 1;

	// gateway ר�ü�����
	std::shared_ptr<xx::UvFromGatewayListener> gatewayListener;

	// key: gatewayId
	std::unordered_map<uint32_t, std::shared_ptr<xx::UvFromGatewayPeer>> gatewayPeers;

	void InitGatewayListener(char const* const& ip, int const& port) {
		xx::MakeTo(gatewayListener, uv, ip, port);
		gatewayListener->onAccept = [this](xx::UvPeer_s peer) {
			auto&& sp = xx::As<xx::UvFromGatewayPeer>(peer);
			sp->onReceiveCommand = [this, sp](xx::BBuffer& bb)->int {
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;
				if (cmd == "gatewayId") {
					uint32_t gatewayId = 0;
					if (int r = bb.Read(gatewayId)) return r;

					// ��ע��
					if (sp->gatewayId != 0xFFFFFFFFu) return -1;
					auto&& iter = gatewayPeers.find(gatewayId);
					if (iter != gatewayPeers.end()) return -2;
					// todo: alive check ? ���ж��������, ����Ҫ������⡣ ���־����ӽ��� ������ Dispose. ���Ծ��� dialer �˷��� ping pong ������

					sp->gatewayId = gatewayId;
					gatewayPeers.emplace(gatewayId, sp);

					xx::CoutN("UvFromGatewayPeer recv cmd gatewayId: gatewayId = ", gatewayId);
					return 0;
				}

				else if (cmd == "disconnect") {
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					// ��ʱ�Ͽ� �ض�peer
					sp->DisconnectSimulatePeer(clientId);

					xx::CoutN("UvFromGatewayPeer recv cmd disconnect: clientId = ", clientId);
					return 0;
				}

				else {
					return -3;
				}
				return 0;
			};

			sp->onReceive = [this, sp](uint32_t const& id, uint8_t* const& buf, size_t const& len)->int {
				auto&& iter = sp->simulatePeers.find(id);
				if (iter == sp->simulatePeers.end() 
					|| !iter->second 
					|| iter->second->Disposed()) {
					// �� sp �� close
					sp->SendCommand_Close(id);
					return 0;
				}
				int r = iter->second->HandlePack(buf, (uint32_t)len);
				if (r) {
					sp->DisconnectSimulatePeer(id);
				}
				return 0;
			};

			sp->onDisconnect = [this, sp] {
				if (sp->gatewayId != 0xFFFFFFFFu) {
					this->gatewayPeers.erase(sp->gatewayId);
				}

				// ��ʱ�Ͽ� peers
				sp->DisconnectSimulatePeers();

				xx::CoutN("UvFromGatewayPeer disconnected. ip = ", sp->GetIP());
			};

		};
	}

};


struct Lobby : UvServiceBase<xx::UvSimulatePeer> {
	Lobby() {
		InitGatewayListener("0.0.0.0", 22222);
		InitService0Dialer();
	}

	/***********************************************************************************************/
	// service0
	/***********************************************************************************************/

	// ������ timer
	std::shared_ptr<xx::UvTimer> service0DialTimer;

	// ������
	std::shared_ptr<xx::UvToServiceDialer> service0Dialer;

	// �� service0 ������
	std::shared_ptr<xx::UvFromToServicePeer> service0Peer;


	void InitService0Dialer() {
		xx::MakeTo(service0DialTimer, uv, 500, 500, [this] {
			if (!service0Peer || service0Peer->Disposed()) {
				if (!service0Dialer->Busy()) {
					service0Dialer->Dial();
				}
			}
			});

		xx::MakeTo(service0Dialer, uv);
		service0Dialer->ip = "127.0.0.1";
		service0Dialer->port = 22222;

		service0Dialer->onConnect = [this](xx::UvPeer_s peer) {
			if (!peer) return;

			service0Peer = xx::As<xx::UvFromToServicePeer>(peer);

			service0Peer->onDisconnect = [this] {
				service0Peer.reset();
				xx::CoutN("service peer disconnect: ", service0Peer->GetIP());
			};

			service0Peer->onReceivePush = [this](xx::Object_s&& msg)->int {
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) return -1;
				std::string cmd;
				if (int r = bb->Read(cmd)) return r;
				if (cmd == "accept") {
					uint32_t gatewayId = 0;
					uint32_t clientId = 0;
					if (int r = bb->Read(gatewayId, clientId)) return r;

					// ��λ��Ŀ�� gateway peer
					auto&& iter = gatewayPeers.find(gatewayId);
					if (iter == gatewayPeers.end()) return 0;
					auto&& gp = iter->second;
					if (!gp || gp->Disposed()) return 0;

					// �������� peer ( ����Ѵ��ھͻᱻ������ )
					auto&& cp = gp->CreateSimulatePeer<xx::UvSimulatePeer>(clientId);
					AcceptSimulatePeer(cp);

					xx::CoutN("service peer recv cmd accept: gatewayId = ", gatewayId, ", clientId: ", clientId);
					return 0;
				}

				else {
					return -3;
				}
				return 0;
			};

			service0Peer->SendPush_Command_ServiceId(serviceId);

			xx::CoutN("service peer onConnect: ", service0Peer->GetIP());
		};
	}

	virtual void AcceptSimulatePeer(std::shared_ptr<xx::UvSimulatePeer>& cp) override {
		// echo
		cp->onReceivePush = [this, cp](xx::Object_s&& msg)->int {
			return cp->SendPush(msg);
		};
		cp->onDisconnect = [this, cp] {
			xx::CoutN("UvSimulatePeer disconnected. clientId = ", cp->id);
		};
	}

	// todo: listener ......
};

int main() {
	Lobby o;
	o.uv.Run();
	xx::CoutN("lobby: exit");
	return 0;
}
