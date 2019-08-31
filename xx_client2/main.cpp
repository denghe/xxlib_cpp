#include <xx_uv_ext.h>

struct Client {
	xx::Uv uv;

	// 模拟帧循环用定时器
	std::shared_ptr<xx::UvTimer> timer;

	// 协程行号
	int lineNumber = 0;

	// 链路 peer. 连接到 gateway
	std::shared_ptr<xx::UvToGatewayDialer<xx::UvFrameSimulatePeer>> gatewayDialer;

	// 下面是内部服务开放之后产生的 peer 的存放点
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

		// 开始拨号
		r = gatewayDialer->Dial("192.168.1.52", 20000, 500);		// 500 ms 超时
		xx::CoutN("client gatewayDialer dial........");

		// 没开始拨号立刻失败（可能没网络堆栈）
		if (r) {
			// 间隔 xx 帧再次拨号
			ticks = 50;
			while (--ticks) {
				COR_YIELD;
			}
			goto LabDial;
		}

		// 等待拨号产生结果
		while (gatewayDialer->Busy()) {
			COR_YIELD;
		}

		// 如果未连接失败就再次拨号
		if (!gatewayDialer->PeerAlive()) goto LabDial;

		// 等待创建 simulate peer 
		ticks = 50;
		while (--ticks) {
			COR_YIELD;
			if (service0Peer) goto LabStep1;
		}
		goto LabDial;

	LabStep1:
		//xx::CoutN("step1");

		// 试着通过 service0Peer 发包 

		// 发包给 addr = 0, bbuffer [ 123 ]
		bb->Clear();
		bb->Write((uint8_t)123);
		service0Peer->SendPush(bb);
		//xx::CoutN("client send to addr = ", 0, ", bb = ", bb);

		// 等回包			
		// 如果超过 3 秒没收到回应 就断线重播
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

			// 如果收到东西, 判断是否符合预期
			if (service0Peer->recvs.size()) {
				if (service0Peer->recvs.size() != 1) {
					xx::CoutN("recvs nums is wrong. dump: {");
					for (auto&& recv : service0Peer->recvs) {
						xx::CoutN("addr = ", recv.first, ", pkg = ", recv.second);
					}
					xx::CoutN("} redial");
					goto LabDial;
				}

				// 校验是否为 addr = 0, 内容 bbuffer [ 123 ]
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

				// 继续下一个测试
				break;
			}
		}

		++count;
		if (count % 100 == 0) {
			xx::CoutN(count);
		}
		goto LabStep1;

		// 重拨
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
