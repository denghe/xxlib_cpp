#include <xx_uv_ext.h>

// ģ����һ�� 0 �ŷ���
// ��ûд��

struct Service0 : xx::UvServiceBase<> {
	Service0() {
		InitGatewayListener("0.0.0.0", 12346);
	}
};

int main() {
	//Service0 s;
	//s.uv.Run();
	return 0;
}
