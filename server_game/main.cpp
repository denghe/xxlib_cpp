#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include "xx_uv.h"
xx::Uv uv;

#include "CatchFish.h"

int main() {
	Service service;
	uv.Run();
	return 0;
}
