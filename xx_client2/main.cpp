#include <xx_uv_ext.h>

template<typename Cmd, typename ...Args>
xx::BBuffer_s& WriteCmd(xx::BBuffer_s& bb, Cmd const& cmd, Args const&...args) {
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

struct Client {
	xx::Uv uv;

	// ģ��֡ѭ���ö�ʱ��
	std::shared_ptr<xx::UvTimer> timer;

	// Э���к�
	int lineNumber = 0;

	// ��· peer. ���ӵ� gateway
	std::shared_ptr<xx::UvToGatewayDialer<xx::UvFrameSimulatePeer>> gatewayDialer;

	// �������ڲ����񿪷�֮������� peer �Ĵ�ŵ�
	std::shared_ptr<xx::UvFrameSimulatePeer> service0Peer;
	std::shared_ptr<xx::UvFrameSimulatePeer> gamePeer;

	int count = 0;
	int r = 0;
	int ticks = 0;
	bool finished = false;
	xx::BBuffer_s bb = xx::Make<xx::BBuffer>();
	std::string cmd;

	Client() {

		xx::MakeTo(timer, uv, 0, 100, [this] {
			if (this->lineNumber >= 0) {
				this->lineNumber = Update();
			}
			});
		assert(timer);

		xx::MakeTo(gatewayDialer, uv, 1);	// 0: tcp   1: kcp   2: auto
		assert(gatewayDialer);

		gatewayDialer->onAcceptSimulatePeer = [this](std::shared_ptr<xx::UvFrameSimulatePeer>& p) {
			xx::CoutN("onAcceptSimulatePeer: p->id = ", p->id);
			switch (p->id) {
			case 0:
				service0Peer = p;
				break;
			default:
				gamePeer = p;
			}
		};
	}

	void Cleanup() {
		gatewayDialer->Cancel();
		gatewayDialer->peer.reset();
		service0Peer.reset();
		gamePeer.reset();
		cmd.clear();
	}

	int Update() {
		COR_BEGIN;
	LabDial:
		Cleanup();

		// ��ʼ����
		r = gatewayDialer->Dial("192.168.1.132", 9999, 2000);
		xx::CoutN("client gatewayDialer dial........");

		// û��ʼ��������ʧ�ܣ�����û�����ջ��
		if (r) {
			// ��� xx ֡�ٴβ���
			ticks = 50;
			while (--ticks) {
				COR_YIELD;
			}
			goto LabDial;
		}

		// �ȴ����Ų������
		while (gatewayDialer->Busy()) {
			COR_YIELD;
		}

		// �������ʧ�ܾ��ٴβ���
		if (gatewayDialer->PeerAlive() < 0)
			goto LabDial;

		// �ȴ����� simulate peer 
		ticks = 50;
		while (--ticks) {
			COR_YIELD;
			if (service0Peer) goto LabStep1;
		}
		goto LabDial;

	LabStep1:
		xx::CoutN("step1");

		// ����ͨ�� service0Peer ���� 

		// �� enter ��. serviceId �� 1
		finished = false;
		service0Peer->SendRequest(WriteCmd(bb, "enter", (uint32_t)1), [this](xx::Object_s&& msg) {
			bb = xx::As<xx::BBuffer>(msg);
			finished = true;
			return 0;
		}, 5000);
		xx::CoutN("client send enter to ", 0, ", bb = ", bb);

		// �ȴ� enter �����������߾��ز�
		while (gatewayDialer->PeerAlive() >= 0) {
			if (finished) break;
			COR_YIELD;
		}
		
		// ���߻�ʱ
		if (!bb) {
			goto LabDial;
		}

		xx::CoutN("enter result = ", bb);
		if (auto r = bb->Read(cmd)) {
			xx::CoutN("read cmd error. r = ", r);
			goto LabDial;
		}

		// �ɹ�: 
		if (cmd == "success") {
			// todo: �������ֵЯ�� serviceId �Ǿͻ�Ҫ��һ���Ƚ� accept �� gamePeer id �Ƿ���� serviceId
			goto LabWaitGamePeer;
		}
		// ����: ��ӡ������ϸ
		else if (cmd == "error") {
			std::string errText;
			if (auto r = bb->Read(errText)) {
				xx::CoutN("enter read errText error. r = ", r);
			}
			else {
				xx::CoutN("enter recv error: ", errText);
			}
		}
		else {
			xx::CoutN("recv unhandled cmd: ", cmd);
		}
		goto LabDial;

	LabWaitGamePeer:
		// �ȴ� game service open��������߾��ز�
		while (gatewayDialer->PeerAlive() >= 0) {
			if (gamePeer) goto LabGamePeerLogic;
			COR_YIELD;
			// todo: timeout check
		}
		goto LabDial;

	LabGamePeerLogic:
		xx::CoutN("game peer opened");

		// todo: send pkg to service 1

		// �������� & ���߼��
		while (gatewayDialer->PeerAlive() >= 0) {
			COR_YIELD;
			// todo
		}

	//xx::Cout(".");

		// todo: ��� game peer. �ȵ�����������


		//// �Ȼذ�			
		//// ������� 3 ��û�յ���Ӧ �Ͷ����ز�
		//ticks = 30;
		//while (true) {
		//	COR_YIELD;
		//	if (service0Peer->Disposed()) {
		//		xx::CoutN("service0Peer disconnected. redial");
		//		goto LabDial;
		//	}
		//	if (!--ticks) {
		//		xx::CoutN("timeout. redial");
		//		goto LabDial;
		//	}
		//	//xx::Cout(".");

		//	// ����յ�����, �ж��Ƿ����Ԥ��
		//	if (service0Peer->recvs.size()) {
		//		if (service0Peer->recvs.size() != 1) {
		//			xx::CoutN("recvs nums is wrong. dump: {");
		//			for (auto&& recv : service0Peer->recvs) {
		//				xx::CoutN("addr = ", recv.first, ", pkg = ", recv.second);
		//			}
		//			xx::CoutN("} redial");
		//			goto LabDial;
		//		}

		//		// У���Ƿ�Ϊ addr = 0, ���� bbuffer [ 123 ]
		//		auto&& recv = service0Peer->recvs[0];
		//		bool isOK = false;
		//		do {
		//			if (recv.first != 0 || !recv.second) break;
		//			auto&& recvBB = xx::As<xx::BBuffer>(recv.second);
		//			if (!recvBB) break;
		//			if (recvBB->len != 1) break;
		//			if (recvBB->At(0) != 123) break;
		//			isOK = true;
		//		} while (false);
		//		if (!isOK) {
		//			xx::CoutN("recv is wrong. addr = ", recv.first, ", pkg = ", recv.second, ". redial");
		//			goto LabDial;
		//		}
		//		service0Peer->recvs.clear();

		//		// ������һ������
		//		break;
		//	}
		//}

		//++count;
		//if (count % 100 == 0) {
		//	xx::CoutN(count);
		//}
		//goto LabStep1;

		//// �ز�
		//xx::CoutN("client send test finished. redial");
		//goto LabDial;

		COR_END
	}
};

int main() {
	Client client;
	client.uv.Run();
	xx::CoutN("client exit");
}
