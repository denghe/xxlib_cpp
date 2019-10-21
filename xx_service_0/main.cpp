#include <xx_uv_ext.h>

// ģ����һ�� 0 �ŷ��� for bug test

using PeerType = xx::UvSimulatePeer;
struct Service0 : xx::UvServiceBase<PeerType, true> {
	Service0() {
		InitGatewayListener("0.0.0.0", 12346);
	}

	virtual void AcceptSimulatePeer(std::shared_ptr<PeerType>& sp) override {
		sp->onReceiveRequest = [sp](int const& serial, xx::Object_s && msg)->int {
			xx::CoutN("recv msg = ", msg);

			if (rand() % 2) {
				// ��˵�� push �� response ���ܷ����ղ��� response? Ȼ����û������������
				if (int r = sp->SendPush(msg)) return r;
				return sp->SendResponse(serial, msg);
			}
			else {
				// ���߲���. ��˵���ߺ� �Է����ص�? ���޸�
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
