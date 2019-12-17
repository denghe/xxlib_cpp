#include <xx_uv_ext.h>

// ģ����һ�� game server. ���� service0. �� gateway ��

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

			// �����ӵ�������
			this->service0peer = peer;

			// ���� register + serviceId
			auto&& bb = xx::Make<xx::BBuffer>();
			bb->Write(std::string("register"), (uint32_t)1);
			(void)peer->SendRequest(bb, [](xx::Object_s&& msg) {
				// ��ǰ����ֱ���� bb ���շ�����. �����������Ͳ���
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) return -1;
				


				return 0;
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
