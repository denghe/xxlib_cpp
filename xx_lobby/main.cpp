#include <xx_uv_ext.h>

// 这个服务还没写完

struct Lobby : xx::UvServiceBase<xx::UvSimulatePeer> {

	// 当前服务的id ( 可能填充自配置或者其他 )
	uint32_t serviceId = 1;

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
