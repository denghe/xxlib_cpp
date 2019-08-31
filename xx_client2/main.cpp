#include <xx_uv_ext.h>

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
	std::shared_ptr<xx::UvFrameSimulatePeer> lobbyPeer;
	std::shared_ptr<xx::UvFrameSimulatePeer> gamePeer;

	int count = 0;
	int r = 0;
	int ticks = 0;
	xx::BBuffer_s bb = xx::Make<xx::BBuffer>();

	Client() {

		xx::MakeTo(timer, uv, 0, 1, [this] {
			if (this->lineNumber >= 0) {
				this->lineNumber = Update();
			}
			});
		assert(timer);

		xx::MakeTo(gatewayDialer, uv, 0);	// 0: tcp   1: kcp   2: auto
		assert(gatewayDialer);

		gatewayDialer->onAcceptSimulatePeer = [this](std::shared_ptr<xx::UvFrameSimulatePeer>& p) {
			switch (p->id) {
			case 0:
				service0Peer = p;
				break;
			case 1:
				lobbyPeer = p;
				break;
			case 2:
				gamePeer = p;
				break;
			default:
				assert(false);
			}
		};
	}

	void Cleanup() {
		gatewayDialer->Cancel();
		gatewayDialer->peer.reset();
		service0Peer.reset();
		lobbyPeer.reset();
		gamePeer.reset();
	}

	int Update() {
		COR_BEGIN;
	LabDial:
		Cleanup();

		// ��ʼ����
		r = gatewayDialer->Dial("192.168.1.52", 20000, 500);		// 500 ms ��ʱ
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

		// ���δ����ʧ�ܾ��ٴβ���
		if (!gatewayDialer->PeerAlive()) goto LabDial;

		// �ȴ����� simulate peer 
		ticks = 50;
		while (--ticks) {
			COR_YIELD;
			if (service0Peer) goto LabStep1;
		}
		goto LabDial;

	LabStep1:
		//xx::CoutN("step1");

		// ����ͨ�� service0Peer ���� 

		// ������ addr = 0, bbuffer [ 123 ]
		bb->Clear();
		bb->Write((uint8_t)123);
		service0Peer->SendPush(bb);
		//xx::CoutN("client send to addr = ", 0, ", bb = ", bb);

		// �Ȼذ�			
		// ������� 3 ��û�յ���Ӧ �Ͷ����ز�
		ticks = 30;
		while (true) {
			COR_YIELD;
			if (service0Peer->Disposed()) {
				xx::CoutN("service0Peer disconnected. redial");
				goto LabDial;
			}
			if (!--ticks) {
				xx::CoutN("timeout. redial");
				goto LabDial;
			}
			//xx::Cout(".");

			// ����յ�����, �ж��Ƿ����Ԥ��
			if (service0Peer->recvs.size()) {
				if (service0Peer->recvs.size() != 1) {
					xx::CoutN("recvs nums is wrong. dump: {");
					for (auto&& recv : service0Peer->recvs) {
						xx::CoutN("addr = ", recv.first, ", pkg = ", recv.second);
					}
					xx::CoutN("} redial");
					goto LabDial;
				}

				// У���Ƿ�Ϊ addr = 0, ���� bbuffer [ 123 ]
				auto&& recv = service0Peer->recvs[0];
				bool isOK = false;
				do {
					if (recv.first != 0 || !recv.second) break;
					auto&& recvBB = xx::As<xx::BBuffer>(recv.second);
					if (!recvBB) break;
					if (recvBB->len != 1) break;
					if (recvBB->At(0) != 123) break;
					isOK = true;
				} while (false);
				if (!isOK) {
					xx::CoutN("recv is wrong. addr = ", recv.first, ", pkg = ", recv.second, ". redial");
					goto LabDial;
				}
				service0Peer->recvs.clear();

				// ������һ������
				break;
			}
		}

		++count;
		if (count % 100 == 0) {
			xx::CoutN(count);
		}
		goto LabStep1;

		// �ز�
		xx::CoutN("client send test finished. redial");
		goto LabDial;

		COR_END
	}
};

int main() {
	Client client;
	client.uv.Run();
	xx::CoutN("client exit");
}
