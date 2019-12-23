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

	// 模拟帧循环用定时器
	std::shared_ptr<xx::UvTimer> timer;

	// 协程行号
	int lineNumber = 0;

	// 链路 peer. 连接到 gateway
	std::shared_ptr<xx::UvToGatewayDialer<xx::UvFrameSimulatePeer>> gatewayDialer;

	// 下面是内部服务开放之后产生的 peer 的存放点
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

		// 开始拨号
		r = gatewayDialer->Dial("192.168.1.132", 9999, 2000);
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

		// 如果连接失败就再次拨号
		if (gatewayDialer->PeerAlive() < 0)
			goto LabDial;

		// 等待创建 simulate peer 
		ticks = 50;
		while (--ticks) {
			COR_YIELD;
			if (service0Peer) goto LabStep1;
		}
		goto LabDial;

	LabStep1:
		xx::CoutN("step1");

		// 试着通过 service0Peer 发包 

		// 发 enter 包. serviceId 传 1
		finished = false;
		service0Peer->SendRequest(WriteCmd(bb, "enter", (uint32_t)1), [this](xx::Object_s&& msg) {
			bb = xx::As<xx::BBuffer>(msg);
			finished = true;
			return 0;
		}, 5000);
		xx::CoutN("client send enter to ", 0, ", bb = ", bb);

		// 等待 enter 结果。如果断线就重拨
		while (gatewayDialer->PeerAlive() >= 0) {
			if (finished) break;
			COR_YIELD;
		}
		
		// 断线或超时
		if (!bb) {
			goto LabDial;
		}

		xx::CoutN("enter result = ", bb);
		if (auto r = bb->Read(cmd)) {
			xx::CoutN("read cmd error. r = ", r);
			goto LabDial;
		}

		// 成功: 
		if (cmd == "success") {
			// todo: 如果返回值携带 serviceId 那就还要进一步比较 accept 的 gamePeer id 是否等于 serviceId
			goto LabWaitGamePeer;
		}
		// 出错: 打印错误明细
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
		// 等待 game service open。如果断线就重拨
		while (gatewayDialer->PeerAlive() >= 0) {
			if (gamePeer) goto LabGamePeerLogic;
			COR_YIELD;
			// todo: timeout check
		}
		goto LabDial;

	LabGamePeerLogic:
		xx::CoutN("game peer opened");
		gamePeer->SendPush(WriteCmd(bb, "test"));

		// 保持连接 & 断线检查
		while (gatewayDialer->PeerAlive() >= 0) {
			if (gamePeer->recvs.size()) {
				for (auto&& recv : gamePeer->recvs) {
					xx::CoutN("recv push from game peer: ", recv.first, " ", recv.second);
				}
				gamePeer->recvs.clear();
			}
			COR_YIELD;
			// todo
		}

	//xx::Cout(".");

		// todo: 检查 game peer. 等到后用来发包


		//// 等回包			
		//// 如果超过 3 秒没收到回应 就断线重播
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

		//	// 如果收到东西, 判断是否符合预期
		//	if (service0Peer->recvs.size()) {
		//		if (service0Peer->recvs.size() != 1) {
		//			xx::CoutN("recvs nums is wrong. dump: {");
		//			for (auto&& recv : service0Peer->recvs) {
		//				xx::CoutN("addr = ", recv.first, ", pkg = ", recv.second);
		//			}
		//			xx::CoutN("} redial");
		//			goto LabDial;
		//		}

		//		// 校验是否为 addr = 0, 内容 bbuffer [ 123 ]
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

		//		// 继续下一个测试
		//		break;
		//	}
		//}

		//++count;
		//if (count % 100 == 0) {
		//	xx::CoutN(count);
		//}
		//goto LabStep1;

		//// 重拨
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
