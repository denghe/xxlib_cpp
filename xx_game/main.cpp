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

template<typename Peer, typename Cmd, typename ...Args>
int SendResponse(Peer& peer, int const& serial, xx::BBuffer_s& bb, Cmd const& cmd, Args const&...args) {
	if (!peer || peer->Disposed()) return -1;
	return peer->SendResponse(serial, WriteCmd(bb, cmd, args...));
}

template<typename Peer, typename ...Args>
int SendResponse_Error(Peer& peer, int const& serial, xx::BBuffer_s& bb, Args const&...args) {
	if (!peer || peer->Disposed()) return -1;
	return peer->SendResponse(serial, WriteCmd_Error(bb, args...));
}


using PeerType = xx::UvSimulatePeer;
struct GameServer : xx::UvServiceBase<PeerType, true> {

	xx::UvPeer_s service0peer;
	xx::UvDialer_s dialer;
	xx::UvTimer_s timer;

	GameServer() {
		// ��ʼ������ gateway ���ӵ� listener
		InitGatewayListener("0.0.0.0", 10002);

		// ��ʼ������ ���ӵ� service0 �� dialer
		xx::MakeTo(dialer, uv, 0);
		dialer->onConnect = [this](xx::UvPeer_s peer) {
			// û����
			if (!peer) return;

			// �����ӵ������ı���
			this->service0peer = peer;

			// ע�� peer ������
			peer->onReceiveRequest = [this, peer](int const& serial, xx::Object_s&& msg) {
				// ��ǰ����ֱ���� bb ���շ�����. �����������ͱ���
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) {
					return SendResponse_Error(peer, serial, bb, "msg is not bb");
				}

				// ��ǰ������ cmd string + args... ��Ϊ���ݸ�ʽ. �ȶ� cmd. �ٸ��ݾ��� cmd ���������
				std::string cmd;
				// �������򷵻ش���
				if (auto r = bb->Read(cmd)) {
					return SendResponse_Error(peer, serial, bb, "msg read cmd error. r = ", r);
				}

				// �������ָ��. �������� gatewayId, clientId ��֪ͨ��Ӧ�� gateway open
				if (cmd == "enter") {
					uint32_t gatewayId = 0, clientId = 0;
					// �������򷵻ش���
					if (auto r = bb->Read(gatewayId, clientId)) {
						return SendResponse_Error(peer, serial, bb, "cmd: enter read serviceId, clientId error. r = ", r);
					}

					// ��ʼ��ϵ���� open
					auto iter = gatewayPeers.find(gatewayId);

					// �Ҳ��� �� δ���� �� �ѶϿ�: ���ش�����Ϣ
					if (iter == gatewayPeers.end() || !iter->second || iter->second->Disposed()) {
						return SendResponse_Error(peer, serial, bb, "cmd: enter can't find gateway. gatewayId = ", gatewayId);
					}
					// �����ط��� open cmd
					else {
						(void)iter->second->SendCommand_Open(clientId);
					}

					// �������� peer ( ����Ѵ��ھͻᱻ������ )
					auto&& cp = iter->second->CreateSimulatePeer<PeerType>(clientId);
					AcceptSimulatePeer(cp);

					// ���سɹ�
					return SendResponse(peer, serial, bb, "success");
				}
				else {
					// ���ش���: �յ�δ�����ָ��
					return SendResponse_Error(peer, serial, bb, "recv unhandled cmd: ", cmd);
				}

				return 0;
			};

			// ���� register + serviceId
			auto&& bb = xx::Make<xx::BBuffer>();
			bb->Write("register", (uint32_t)1);
			(void)peer->SendRequest(bb, [](xx::Object_s&& msg) {
				// ��ǰ����ֱ���� bb ���շ�����. �����������ͱ���
				auto&& bb = xx::As<xx::BBuffer>(msg);
				if (!bb) {
					xx::CoutN("register recv null.");
					return -1;
				}
				
				// ��ǰ������ cmd string + args ��Ϊ���ݸ�ʽ. �ȶ� cmd
				std::string cmd;
				if (auto r = bb->Read(cmd)) {
					xx::CoutN("register read cmd error. r = ", r);
					return -2;
				}

				// �ɹ�
				if (cmd == "success") {
					xx::CoutN("register success.");
					return 0;
				}
				// �յ�������ʾ
				else if (cmd == "error") {
					std::string errText;
					if (auto r = bb->Read(errText)) {
						xx::CoutN("register read errText error. r = ", r);
						return -3;
					}
					xx::CoutN("register recv error: ", errText);
					return -4;
				}
				// �յ�����֮��İ�
				else {
					xx::CoutN("recv unhandled command: ", cmd);
					return -5;
				}
			}, 5000);
		};

		xx::MakeTo(timer, uv, 0, 500, [this] {
			if (!dialer->Busy() && (!service0peer || service0peer->Disposed())) {
				dialer->Dial("192.168.1.132", 10011);
			}
		});
	}

	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& sp) override {
		xx::CoutN("AcceptSimulatePeer");
		// ʵ�� echo Ч��
		sp->onReceiveRequest = [sp](int const& serial, xx::Object_s&& msg)->int {
			xx::CoutN("recv request msg = ", msg);
			return sp->SendResponse(serial, msg);
		};
		sp->onReceivePush = [sp](xx::Object_s&& msg)->int {
			xx::CoutN("recv push msg = ", msg);
			return sp->SendPush(msg);
		};
	}
};

int main() {
	GameServer s;
	s.uv.Run();
	return 0;
}
