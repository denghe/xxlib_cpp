#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include "xx_uv.h"
xx::Uv uv;

// todo: 对于 server 收到的所有来自客户端的 float double 值类型，做非数字, 极值, 除 0 前置检测。避免出现逻辑数学计算异常

struct Service;
Service* service;
#include "CatchFish.h"

int main() {
	::service = new Service();
	uv.Run();
	delete ::service;
	return 0;
}
