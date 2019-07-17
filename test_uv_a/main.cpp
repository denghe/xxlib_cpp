#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

/*
转码命令行:
	./ffmpeg -f image2 -framerate 60 -i "??????(%d).png" -c:v libvpx-vp9 -pix_fmt yuva420p -b:v 1000K -speed 0 ??????.webm

转码命令行作用:
	将一连串 png 转为 码率为 1000K( 可调整可不填 ) 的 每秒 60 帧 ( 按实际情况来 ) 的带 alpha 的 webm 视频

png -> webm 期望效果:
	文件体积缩小至 1/10 左右, 运行时贴图内存节省一大半, 可用于动画对象个数不多, draw call 不敏感的情景

需求1:
	为便于运行时加载 & 解码, 需要将 转码命令行产生的 webm, 转为自己的文件格式

需求2:
	运行时逐帧解码利用 shader 将 yuva 加工为 rgba, render to texture 再利用

扩展需求:
	小图片是否可以在 2048x2048 | 4096x4096 等区间内运行时排布入内，得到类似 texture packer 的打包序列帧贴图? 从而减少 draw call?


文件格式: (ver 0.1a)
	byte					codecId;			// 0: vp8 yuv420p     1: vp8 yuva420p     2: vp9 yuv420p    3: vp9 yuva420p
	float					duration;			// 总播放时长: 秒
	int						count;				// 总帧数

	List<pair<int, int>>	frames;				// pair<rgbBuf, rgbBufSize>	int 代表 data 的 offset	// codecId: 0 / 2
	List<tuple<int * 4>>	frames;				// pair<rgbBuf, rgbBufSize, aBuf, aBufSize>			// codecId: 1 / 3

	byte[]					data;				// 跟在上面的变量数据后面
*/
#include "ebml_parser.h"

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	return 0;
}
