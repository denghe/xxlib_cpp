#include <xx_uv_ext.h>



/*
使用方法：继承并覆盖 Accept.  依次执行下列函数( 参数自拟 ):
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

	// 当前服务的id ( 可能填充自配置或者其他 )
	uint32_t serviceId = 1;

	// gateway 专用监听器
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

					// 已注册
					if (sp->gatewayId != 0xFFFFFFFFu) return -1;
					auto&& iter = gatewayPeers.find(gatewayId);
					if (iter != gatewayPeers.end()) return -2;
					// todo: alive check ? 非有顶下线设计, 则需要心跳检测。 发现旧连接僵死 就主动 Dispose. 可以经由 dialer 端发起 ping pong 续命包

					sp->gatewayId = gatewayId;
					gatewayPeers.emplace(gatewayId, sp);

					xx::CoutN("UvFromGatewayPeer recv cmd gatewayId: gatewayId = ", gatewayId);
					return 0;
				}

				else if (cmd == "disconnect") {
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					// 及时断开 特定peer
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
					// 向 sp 发 close
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

				// 及时断开 peers
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

	// 拨号用 timer
	std::shared_ptr<xx::UvTimer> service0DialTimer;

	// 拨号器
	std::shared_ptr<xx::UvToServiceDialer> service0Dialer;

	// 与 service0 的连接
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

					// 定位到目标 gateway peer
					auto&& iter = gatewayPeers.find(gatewayId);
					if (iter == gatewayPeers.end()) return 0;
					auto&& gp = iter->second;
					if (!gp || gp->Disposed()) return 0;

					// 创建虚拟 peer ( 如果已存在就会被顶下线 )
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
