#include <xx_uv_ext.h>

// 模拟了一个 0 号服务 for bug test

using PeerType = xx::UvSimulatePeer;
struct Service0 : xx::UvServiceBase<PeerType, true> {
	Service0() {
		InitGatewayListener("0.0.0.0", 10001);
	}

	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& sp) override {
		sp->onReceiveRequest = [sp](int const& serial, xx::Object_s && msg)->int {
			xx::CoutN("recv msg = ", msg);

			if (rand() % 2) {
				// 听说先 push 再 response 接受方会收不到 response? 然而并没出现这种现象
				if (int r = sp->SendPush(msg)) return r;
				return sp->SendResponse(serial, msg);
			}
			else {
				// 断线测试. 听说断线后 对方不回调? 已修复
				return -1;
			}
		};
	}
};

int main() {
	Service0 s;
	s.uv.Run();
	return 0;
}
