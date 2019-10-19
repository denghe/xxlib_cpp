#include <xx_uv_ext.h>

// 模拟了一个 0 号服务
// 还没写完

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
