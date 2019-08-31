#include <xx_uv_ext.h>



// ������ client ���������� service ��Ӧ������ service0 ( ͨ�� serviceListener )
// ���� gateway ��Ӧ������ service0 ( ͨ�� gatewayListener )
struct Service0 {
	xx::Uv uv;

	/***********************************************************************************************/
	// gateway
	/***********************************************************************************************/

	// ���� gateways ����
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

					// ��ע��
					if (gp->gatewayId != 0xFFFFFFFFu) return -1;
					auto&& iter = gatewayPeers.find(gatewayId);
					if (iter != gatewayPeers.end()) return -2;
					// todo: alive check ? ���ж��������, ����Ҫ������⡣ ���־����ӽ��� ������ Dispose. ���Ծ��� dialer �˷��� ping pong ������

					gp->gatewayId = gatewayId;
					gatewayPeers.emplace(gatewayId, gp);

					xx::CoutN("service to gateway peer recv cmd gatewayId: gatewayId: ", gatewayId);
					return 0;
				}

				else if (cmd == "accept") {
					uint32_t clientId = 0;
					std::string ip;
					if (int r = bb.Read(clientId, ip)) return r;

					// todo: ��¼ӳ���ϵ? ip check? ֪ͨ�������ģ���� client ͨ��?
					// �������������δ����, ���� kick �� gateway?
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

	// ���� services ����
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

					// ��ע��
					if (gp->serviceId != 0xFFFFFFFFu) return -1;
					auto&& iter = servicePeers.find(serviceId);
					if (iter != servicePeers.end()) return -2;	// todo: ������?

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

			// todo: alive check ? ���ж��������, ����Ҫ������⡣ ���־����ӽ��� ������ Dispose. ���Ծ��� dialer �˷��� ping pong ������
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
