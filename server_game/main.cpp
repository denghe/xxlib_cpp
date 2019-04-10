#pragma execution_character_set("utf-8")
#define _CRT_SECURE_NO_WARNINGS
#include "xx_uv.h"
xx::Uv uv;

inline int ReadFile(const char* fn, xx::BBuffer& bb)
{
	FILE* fp = fopen(fn, "rb");
	if (nullptr == fp) return -1;
	fseek(fp, 0L, SEEK_END);	// 定位到文件末尾
	int flen = ftell(fp);		// 得到文件大小
	if (flen <= 0)
	{
		fclose(fp);
		return -1;
	}
	bb.Clear();
	bb.Reserve(flen);			// 申请内存空间
	fseek(fp, 0L, SEEK_SET);	// 定位到文件开头
	if (flen != fread(bb.buf, flen, 1, fp)) return -2; // 一次性读取全部文件内容
	bb.len = flen;
	fclose(fp);
	return 0;
}

#include "CatchFish.h"

int main() {
	auto&& listener = xx::Make<Listener>(uv, "0.0.0.0", 12345);
	uv.Run();
	return 0;
}
