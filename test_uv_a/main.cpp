#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

/*
转码命令行:
	./ffmpeg -f image2 -framerate 60 -i "??????(%d).png" -c:v libvpx-vp9 -pix_fmt yuva420p -b:v 1000K -speed 0 ??????.webm

转码命令行作用:
	将一连串 png 转为 码率为 1000K( 看情况调整或不填 ) 的 每秒 60 帧 ( 按实际情况来 ) 的带 alpha 的 webm 视频

png -> webm 期望效果:
	文件体积缩小至 1/10 左右, 运行时部分贴图内存节省一大半, 可用于动画对象个数不多, draw call 不敏感的情景

需求1:
	为便于运行时加载 & 解码, 需要将 转码命令行产生的 webm, 转为自己的文件格式

需求2:
	运行时逐帧解码利用 shader 将 yuva 加工为 rgba, render to texture 再利用

扩展需求:
	小图片是否可以在 2048x2048 | 4096x4096 等区间内运行时排布入内，得到类似 texture packer 的打包序列帧贴图? 从而减少 draw call?
*/

//#include "vpx/vpx_decoder.h"
//#include "vpx/vp8dx.h"

#include "ebml_parser.h"
#include "xx_bbuffer.h"
#include "xx_file.h"

// 整个存档文件的数据映射. 直接序列化
struct Header {
	uint8_t codecId = 0;
	uint8_t hasAlpha = 0;
	uint16_t width = 0;
	uint16_t height = 0;
	float duration = 0;
	xx::List<uint32_t> lens;
	xx::BBuffer data;

	xx::List<uint8_t*> bufs;	// 后期填充

	// 返回总的帧数
	inline size_t GetNumFrames() {
		if (hasAlpha) return lens.len / 2;
		else return lens.len;
	}

	// 填充 bufs 数组
	inline int FillBufs() {
		bufs.Resize(lens.len);
		auto baseBuf = data.buf;
		for (int i = 0; i < lens.len; ++i) {
			bufs[i] = baseBuf;
			baseBuf += lens[i];
		}
		if (baseBuf != data.buf + data.len) return -1;
		return 0;
	}

	inline std::tuple<uint8_t*, uint32_t> GetRGBData(int const& idx) {
		return std::make_tuple(bufs[idx], lens[idx]);
	}

	inline std::tuple<uint8_t*, uint32_t, uint8_t*, uint32_t> GetRGBAData(int const& idx) {
		int i = idx * 2;
		return std::make_tuple(bufs[i], lens[i], bufs[i + 1], lens[i + 1]);
	}
};

namespace xx {
	// 适配 Header 之 序列化 & 反序列化
	template<>
	struct BFuncs<Header, void> {
		static inline void WriteTo(BBuffer& bb, Header const& in) {
			bb.Write(in.codecId, in.hasAlpha, in.width, in.height, in.duration, in.lens, in.data);
		}
		static inline int ReadFrom(BBuffer& bb, Header& out) {
			int r = bb.Read(out.codecId, out.hasAlpha, out.width, out.height, out.duration, out.lens, out.data);
			if (r) return r;
			return out.FillBufs();
		}
	};
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	Header header;

	// 读文件
	std::unique_ptr<uint8_t[]> data;
	size_t dataLen;
	auto&& r = xx::ReadAllBytes("a.webm", data, dataLen);
	assert(!r);

	// 开始解析 ebml 头
	auto&& ebml = parse_ebml_file(std::move(data), dataLen/*, 1*/);
	auto&& segment = ebml.FindChildById(EbmlElementId::Segment);

	// 提取 播放总时长
	auto&& info = segment->FindChildById(EbmlElementId::Info);
	auto&& duration = info->FindChildById(EbmlElementId::Duration);
	header.duration = (float)std::stod(duration->value());

	// 提取 编码方式
	auto&& tracks = segment->FindChildById(EbmlElementId::Tracks);
	auto&& trackEntry = tracks->FindChildById(EbmlElementId::TrackEntry);
	auto&& codecId = trackEntry->FindChildById(EbmlElementId::CodecID);
	header.codecId = codecId->value() == "V_VP8" ? 0 : 1;

	// 提取 宽高
	auto&& video = trackEntry->FindChildById(EbmlElementId::Video);
	auto&& pixelWidth = video->FindChildById(EbmlElementId::PixelWidth);
	header.width = std::stoi(pixelWidth->value());
	auto&& pixelHeight = video->FindChildById(EbmlElementId::PixelHeight);
	header.height = std::stoi(pixelHeight->value());

	// 判断 是否带 alpha 通道
	auto&& _alphaMode = video->FindChildById(EbmlElementId::AlphaMode);
	header.hasAlpha = _alphaMode->value() == "1" ? 1 : 0;

	xx::List<int> frames;
	uint32_t frameNumber = 0;

	std::list<EbmlElement>::const_iterator clusterOwner;
	if (header.codecId == 0) {
		clusterOwner = segment;
	}
	else {
		auto&& tags = segment->FindChildById(EbmlElementId::Tags);
		auto&& tag = tags->FindChildById(EbmlElementId::Tag);
		clusterOwner = tag->FindChildById(EbmlElementId::Targets);
	}

	auto&& cluster = clusterOwner->FindChildById(EbmlElementId::Cluster);
	while (cluster != clusterOwner->children().cend()) {
		auto timecode = cluster->FindChildById(EbmlElementId::Timecode);
		assert(timecode != cluster->children().end());
		auto clusterPts = std::stoi(timecode->value());

		if (header.hasAlpha) {
			auto&& blockGroup = cluster->FindChildById(EbmlElementId::BlockGroup);
			while (blockGroup != cluster->children().cend()) {
				{
					// get yuv data + size
					auto&& block = blockGroup->FindChildById(EbmlElementId::Block);
					auto&& data = block->data();
					auto&& size = block->size();

					// fix yuv data + size
					size_t track_number_size_length;
					(void)get_ebml_element_size(data, size, track_number_size_length);
					data = data + track_number_size_length + 3;
					size = size - track_number_size_length - 3;

					header.lens.Add((uint32_t)size);
					header.data.AddRange(data, size);
				}
				{
					// get a data + size
					auto&& blockAdditions = blockGroup->FindChildById(EbmlElementId::BlockAdditions);
					auto&& blockMore = blockAdditions->FindChildById(EbmlElementId::BlockMore);
					auto&& blockAdditional = blockMore->FindChildById(EbmlElementId::BlockAdditional);
					auto&& data_alpha = blockAdditional->data();
					auto&& size_alpha = (uint32_t)blockAdditional->size();

					header.lens.Add((uint32_t)size_alpha);
					header.data.AddRange(data_alpha, size_alpha);
				}

				// next
				blockGroup = cluster->FindNextChildById(++blockGroup, EbmlElementId::BlockGroup);
				++frameNumber;
			}
		}
		else {
			auto&& simpleBlock = cluster->FindChildById(EbmlElementId::SimpleBlock);
			while (simpleBlock != cluster->children().cend()) {
				auto&& data = simpleBlock->data();
				auto&& size = simpleBlock->size();

				// fix yuv data + size
				size_t track_number_size_length;
				(void)get_ebml_element_size(data, size, track_number_size_length);
				data = data + track_number_size_length + 3;
				size = size - track_number_size_length - 3;

				header.lens.Add((uint32_t)size);
				header.data.AddRange(data, size);

				// next
				simpleBlock = cluster->FindNextChildById(++simpleBlock, EbmlElementId::BlockGroup);
				++frameNumber;
			}
		}

		cluster = clusterOwner->FindNextChildById(++cluster, EbmlElementId::Cluster);
	}


	r = header.FillBufs();
	assert(!r);

	xx::BBuffer bb;
	bb.Write(header);

	Header h2;
	r = bb.Read(h2);
	assert(!r);

	r = xx::WriteAllBytes("a.xxmv", bb);
	assert(!r);

	return 0;
}





//
//inline void YUVA2RGBA(std::string const& fn, uint32_t w, uint32_t h, uint8_t* yData, uint8_t* uData, uint8_t* vData, uint8_t* aData, int yaStride, int uvStride) {
//	// 数据容器
//	std::vector<uint8_t> bytes;
//	bytes.reserve(w * h * 4);
//
//	// 产生像素坐标
//	for (uint32_t _h = 0; _h < h; ++_h) {
//		for (uint32_t _w = 0; _w < w; ++_w) {
//			// 根据坐标结合具体宽高跨距算下标. uv 每个像素对应 ya 4个像素
//			auto&& yaIdx = yaStride * _h + _w;
//			auto&& uvIdx = uvStride * (_h / 2) + _w / 2;
//
//			// 得到 yuv 原始数据, byte -> float
//			auto&& y = yData[yaIdx] / 255.0f;
//			auto&& u = uData[uvIdx] / 255.0f;
//			auto&& v = vData[uvIdx] / 255.0f;
//
//			// 进一步修正
//			y = 1.1643f * (y - 0.0625f);
//			u = u - 0.5f;
//			v = v - 0.5f;
//
//			// 算出 rgb( float 版 )
//			auto&& r = y + 1.5958f * v;
//			auto&& g = y - 0.39173f * u - 0.81290f * v;
//			auto&& b = y + 2.017f * u;
//
//			// 裁剪为 0 ~ 1
//			if (r > 1.0f) r = 1.0f; else if (r < 0.0f) r = 0.0f;
//			if (g > 1.0f) g = 1.0f; else if (g < 0.0f) g = 0.0f;
//			if (b > 1.0f) b = 1.0f; else if (b < 0.0f) b = 0.0f;
//
//			// 存起来
//			bytes.push_back((uint8_t)(b * 255));
//			bytes.push_back((uint8_t)(g * 255));
//			bytes.push_back((uint8_t)(r * 255));
//			bytes.push_back(aData ? aData[yaIdx] : (uint8_t)0);
//		}
//	}
//
//	FILE* fp = fopen(fn.c_str(), "wb");
//	svpng(fp, w, h, bytes.data(), 1);
//	fclose(fp);
//}
