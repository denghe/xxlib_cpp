#include <xx_uv_ext.h>



// 可能与 client 交互的其他 service 都应该连上 service0 ( 通过 serviceListener )
// 所有 gateway 都应该连上 service0 ( 通过 gatewayListener )
struct Service0 {
	xx::Uv uv;

	/***********************************************************************************************/
	// gateway
	/***********************************************************************************************/

	// 监听 gateways 接入
	std::shared_ptr<xx::UvFromGatewayListener> gatewayListener;

	// key: gatewayId
	std::unordered_map<uint32_t, std::shared_ptr<xx::UvFromGatewayPeer>> gatewayPeers;

	void InitGatewayListener() {
		xx::MakeTo(gatewayListener, uv, "0.0.0.0", 22222);
		gatewayListener->onAccept = [this](xx::UvPeer_s peer) {
			auto&& gp = xx::As<xx::UvFromGatewayPeer>(peer);
			gp->onReceiveCommand = [this, gp](xx::BBuffer& bb)->int {
				std::string cmd;
				if (int r = bb.Read(cmd)) return r;
				if (cmd == "gatewayId") {
					uint32_t gatewayId = 0;
					if (int r = bb.Read(gatewayId)) return r;

					// 已注册
					if (gp->gatewayId != 0xFFFFFFFFu) return -1;
					auto&& iter = gatewayPeers.find(gatewayId);
					if (iter != gatewayPeers.end()) return -2;
					// todo: alive check ? 非有顶下线设计, 则需要心跳检测。 发现旧连接僵死 就主动 Dispose. 可以经由 dialer 端发起 ping pong 续命包

					gp->gatewayId = gatewayId;
					gatewayPeers.emplace(gatewayId, gp);

					xx::CoutN("service to gateway peer recv cmd gatewayId: gatewayId: ", gatewayId);
					return 0;
				}

				else if (cmd == "accept") {
					uint32_t clientId = 0;
					std::string ip;
					if (int r = bb.Read(clientId, ip)) return r;

					// todo: 记录映射关系? ip check? 通知主体服务模块与 client 通信?
					// 如果其他服务尚未就绪, 发送 kick 给 gateway?
					// todo: SendPush_Command_Accept

					// for test
					gp->SendCommand_Open(clientId);

					xx::CoutN("service to gateway peer recv cmd accept: clientId: ", clientId, ", ip = ", ip);
					return 0;
				}

				else if (cmd == "disconnect") {
					uint32_t clientId = 0;
					if (int r = bb.Read(clientId)) return r;

					// todo: log?

					xx::CoutN("service to gateway peer recv cmd disconnect: clientId: ", clientId);
					return 0;
				}

				else {
					return -3;
				}
				return 0;
			};

			// for test
			gp->onReceive = [this, gp](uint32_t const& id, uint8_t* const& buf, size_t const& len)->int {
				//xx::CoutN("gp recv : id = ", id, ", buf len = ", len);
				return gp->SendDirect(buf - 8, len + 8);	// echo
			};

			gp->onDisconnect = [this, gp] {
				if (gp->gatewayId != 0xFFFFFFFFu) {
					this->gatewayPeers.erase(gp->gatewayId);
				}

				xx::CoutN("gateway peer disconnected: ip = ", gp->GetIP(), ", gatewayId = ", gp->gatewayId);
			};

			xx::CoutN("gateway peer connected: ip = ", gp->GetIP());
		};
	}



	/***********************************************************************************************/
	// service
	/***********************************************************************************************/

	// 监听 services 接入
	std::shared_ptr<xx::UvFromServiceListener> serviceListener;

	// key: serviceId
	std::unordered_map<uint32_t, std::shared_ptr<xx::UvFromToServicePeer>> servicePeers;

	void InitServiceListener() {
		xx::MakeTo(serviceListener, uv, "0.0.0.0", 33333);
		serviceListener->onAccept = [this](xx::UvPeer_s peer) {
			auto&& gp = xx::As<xx::UvFromToServicePeer>(peer);
			gp->onReceivePush = [this, gp](xx::Object_s&& msg)->int {
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) return -1;
				std::string cmd;
				if (int r = bb->Read(cmd)) return r;
				if (cmd == "serviceId") {
					uint32_t serviceId = 0;
					if (int r = bb->Read(serviceId)) return r;

					// 已注册
					if (gp->serviceId != 0xFFFFFFFFu) return -1;
					auto&& iter = servicePeers.find(serviceId);
					if (iter != servicePeers.end()) return -2;	// todo: 顶下线?

					gp->serviceId = serviceId;
					servicePeers.emplace(serviceId, gp);

					xx::CoutN("service to service peer recv cmd serviceId: serviceId: ", serviceId);
					return 0;
				}

				else {
					return -3;
				}
				return 0;
			};

			gp->onDisconnect = [this, gp] {
				if (gp->serviceId != 0xFFFFFFFFu) {
					this->gatewayPeers.erase(gp->serviceId);
				}

				xx::CoutN("service disconnected. serviceId = ", gp->serviceId + ", ip = ", gp->GetIP());
			};

			// todo: alive check ? 非有顶下线设计, 则需要心跳检测。 发现旧连接僵死 就主动 Dispose. 可以经由 dialer 端发起 ping pong 续命包
		};
	}



	/***********************************************************************************************/
	// constructor
	/***********************************************************************************************/

	Service0() {
		InitServiceListener();
		InitGatewayListener();
	}

};

int main() {
	Service0 s;
	s.uv.Run();
	return 0;
}
