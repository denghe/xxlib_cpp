#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include "xx_uv.h"
xx::Uv uv;

#define ENABLE_CALC_SERVICE 1

struct Service;
Service* service;
#include "CatchFish.h"

int main() {
	::service = new Service();
	uv.Run();
	delete ::service;
	return 0;
}
