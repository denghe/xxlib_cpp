#include <xx_uv_ext.h>

// ģ����һ�� game server. ���� service0. �� gateway ��

template<typename CMD, typename ...Args>
xx::BBuffer_s& WriteCmd(xx::BBuffer_s& bb, CMD const& cmd, Args const&...args) {
	if (!bb) {
		xx::MakeTo(bb);
	}
	else {
		bb->Clear();
	}
	bb->Write(cmd, args...);
	return bb;
}

template<typename ...Args>
xx::BBuffer_s& WriteCmd_Error(xx::BBuffer_s& bb, Args const&...args) {
	std::string s;
	xx::Append(s, args...);
	return WriteCmd(bb, "error", s);
}

using PeerType = xx::UvSimulatePeer;
struct GameServer : xx::UvServiceBase<PeerType, true> {

	xx::UvPeer_s service0peer;
	xx::UvDialer_s dialer;
	xx::UvTimer_s timer;

	GameServer() {
		InitGatewayListener("0.0.0.0", 10002);

		xx::MakeTo(dialer, uv, 0);
		dialer->onConnect = [this](xx::UvPeer_s peer) {
			// û����
			if (!peer) return;

			// �����ӵ������ı���
			this->service0peer = peer;

			// ע�� peer ������
			peer->onReceiveRequest = [this, peer](int const& serial, xx::Object_s&& msg) {
				// ��ǰ����ֱ���� bb ���շ�����. �����������Ͳ���
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) {
					xx::MakeTo(bb);
					bb->Write("error", "service0peer.onReceiveRequest recv error: msg is not bb");
					peer->SendResponse(serial, bb);
					return 0;
				}

				// ��ǰ������ cmd string + args ��Ϊ���ݸ�ʽ. �ȶ� cmd
				std::string cmd;
				if (auto r = bb->Read(cmd)) {
					bb->Clear();
					bb->Write("error", "game_service_peer.SendRequest callback error: msg read cmd error");
					peer->SendResponse(serial, bb);
					return 0;
				}

				// �������ָ��. �������� gatewayId, clientId ��֪ͨ��Ӧ�� gateway open
				if (cmd == "enter") {
					uint32_t gatewayId = 0, clientId = 0;
					if (auto r = bb->Read(gatewayId, clientId)) return 0;

					// ��ϵ���� open
					auto iter = gatewayPeers.find(gatewayId);
					if (iter == gatewayPeers.end()) {
						// todo
					}
					// todo


					xx::CoutN("enter success.");
					return 0;
				}
				// todo
				else {
					xx::CoutN("recv unhandled command: ", cmd);
					return -2;
				}

				return 0;
			};

			// ���� register + serviceId
			auto&& bb = xx::Make<xx::BBuffer>();
			bb->Write(std::string("register"), (uint32_t)1);
			(void)peer->SendRequest(bb, [](xx::Object_s&& msg) {
				// ��ǰ����ֱ���� bb ���շ�����. �����������Ͳ���
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) return -1;
				
				// ��ǰ������ cmd string + args ��Ϊ���ݸ�ʽ. �ȶ� cmd
				std::string cmd;
				if (auto r = bb->Read(cmd)) return r;

				if (cmd == "success") {
					xx::CoutN("register success.");
					return 0;
				}
				else {
					xx::CoutN("recv unhandled command: ", cmd);
					return -2;
				}
			}, 5000);
		};

		xx::MakeTo(timer, uv, 0, 500, [this] {
			if (!dialer->Busy() && !service0peer || service0peer->Disposed()) {
				dialer->Dial("192.168.1.132", 10011);
			}
		});
	}

	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& sp) override {
		//sp->onReceiveRequest = [sp](int const& serial, xx::Object_s&& msg)->int {
		//	xx::CoutN("recv request msg = ", msg);
		//	return sp->SendResponse(serial, msg);
		//};
		//sp->onReceivePush = [sp](xx::Object_s&& msg)->int {
		//	xx::CoutN("recv push msg = ", msg);
		//	return sp->SendPush(msg);
		//};
	}
};

int main() {
	GameServer s;
	s.uv.Run();
	return 0;
}
