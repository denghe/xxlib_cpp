﻿#ifdef _MSC_VER
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

#include "ebml_parser.h"
#include "xx_bbuffer.h"
#include "xx_file.h"

// 整个存档文件的数据映射. 直接序列化
struct Webm {
	// 0: vp8;	1: vp9
	uint8_t codecId = 0;

	// 1: true
	uint8_t hasAlpha = 0;

	// 像素宽
	uint16_t width = 0;

	// 像素高
	uint16_t height = 0;

	// 总播放时长( 秒 )
	float duration = 0;

	// 所有帧数据长度集合( hasAlpha 则为 rgb / a 交替长度集合 )
	xx::List<uint32_t> lens;

	// 所有帧数据依次排列
	xx::BBuffer data;

	// 所有帧 rgb / a 数据块指针( 后期填充 )
	xx::List<uint8_t*> bufs;

	// 总帧数( 后期填充 )
	uint32_t count = 0;

	// todo: 可能还有别的附加信息存储. 例如 中心点坐标, 显示缩放比

	Webm() = default;
	Webm(Webm const&) = delete;
	Webm(Webm&&) = default;
	Webm& operator=(Webm const&) = delete;
	Webm& operator=(Webm&&) = default;

	inline operator bool() {
		return count != 0;
	}

	// 填充基础数据或反序列化后继续 填充 bufs, count
	inline int Init() {
		count = (uint32_t)(hasAlpha ? lens.len / 2 : lens.len);

		bufs.Resize(lens.len);
		auto baseBuf = data.buf;
		for (int i = 0; i < lens.len; ++i) {
			bufs[i] = baseBuf;
			baseBuf += lens[i];
		}
		if (baseBuf != data.buf + data.len) return -1;
		return 0;
	}

	inline void Clear() {
		codecId = 0;
		hasAlpha = 0;
		width = 0;
		height = 0;
		duration = 0;
		lens.Clear();
		data.Clear();
		bufs.Clear();
		count = 0;
	}

	inline int GetFrameBuf(uint32_t const& idx, uint8_t const*& rgbBuf, uint32_t& rgbBufLen) {
		if (hasAlpha) return -1;
		if (idx >= count) return -2;
		rgbBuf = bufs[idx];
		rgbBufLen = lens[idx];
		return 0;
	}

	inline int GetFrameBuf(uint32_t idx, uint8_t const*& rgbBuf, uint32_t& rgbBufLen, uint8_t const*& aBuf, uint32_t& aBufLen) {
		if (!hasAlpha) return -1;
		if (idx >= count) return -2;
		idx *= 2;
		rgbBuf = bufs[idx];
		aBuf = bufs[idx + 1];
		rgbBufLen = lens[idx];
		aBufLen = lens[idx + 1];
		return 0;
	}
};

namespace xx {
	// 适配 Webm 之 序列化 & 反序列化
	template<>
	struct BFuncs<::Webm, void> {
		static inline void WriteTo(BBuffer& bb, ::Webm const& in) {
			bb.Write(in.codecId, in.hasAlpha, in.width, in.height, in.duration, in.lens, in.data);
		}
		static inline int ReadFrom(BBuffer& bb, ::Webm& out) {
			if (int r = bb.Read(out.codecId, out.hasAlpha, out.width, out.height, out.duration, out.lens, out.data)) return r;
			return out.Init();
		}
	};
}

// 从 .webm 读出数据并填充到 wm. 成功返回 0
inline int ReadWebm(Webm& wm, std::filesystem::path const& path) {
	wm.Clear();

	// 读文件
	std::unique_ptr<uint8_t[]> data;
	size_t dataLen;
	if (auto && r = xx::ReadAllBytes(path, data, dataLen)) return r;

	// 开始解析 ebml 头
	auto&& ebml = parse_ebml_file(std::move(data), dataLen/*, 1*/);
	auto&& segment = ebml.FindChildById(EbmlElementId::Segment);

	// 提取 播放总时长
	auto&& info = segment->FindChildById(EbmlElementId::Info);
	auto&& duration = info->FindChildById(EbmlElementId::Duration);
	wm.duration = (float)std::stod(duration->value());

	// 提取 编码方式
	auto&& tracks = segment->FindChildById(EbmlElementId::Tracks);
	auto&& trackEntry = tracks->FindChildById(EbmlElementId::TrackEntry);
	auto&& codecId = trackEntry->FindChildById(EbmlElementId::CodecID);
	wm.codecId = codecId->value() == "V_VP8" ? 0 : 1;

	// 提取 宽高
	auto&& video = trackEntry->FindChildById(EbmlElementId::Video);
	auto&& pixelWidth = video->FindChildById(EbmlElementId::PixelWidth);
	wm.width = std::stoi(pixelWidth->value());
	auto&& pixelHeight = video->FindChildById(EbmlElementId::PixelHeight);
	wm.height = std::stoi(pixelHeight->value());

	// 判断 是否带 alpha 通道
	auto&& _alphaMode = video->FindChildById(EbmlElementId::AlphaMode);
	wm.hasAlpha = _alphaMode->value() == "1" ? 1 : 0;

	xx::List<int> frames;
	uint32_t frameNumber = 0;

	std::list<EbmlElement>::const_iterator clusterOwner;
	if (wm.codecId == 0) {
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
		auto clusterPts = std::stoi(timecode->value());

		if (wm.hasAlpha) {
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

					wm.lens.Add((uint32_t)size);
					wm.data.AddRange(data, size);
				}
				{
					// get a data + size
					auto&& blockAdditions = blockGroup->FindChildById(EbmlElementId::BlockAdditions);
					auto&& blockMore = blockAdditions->FindChildById(EbmlElementId::BlockMore);
					auto&& blockAdditional = blockMore->FindChildById(EbmlElementId::BlockAdditional);
					auto&& data_alpha = blockAdditional->data();
					auto&& size_alpha = (uint32_t)blockAdditional->size();

					wm.lens.Add((uint32_t)size_alpha);
					wm.data.AddRange(data_alpha, size_alpha);
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

				wm.lens.Add((uint32_t)size);
				wm.data.AddRange(data, size);

				// next
				simpleBlock = cluster->FindNextChildById(++simpleBlock, EbmlElementId::BlockGroup);
				++frameNumber;
			}
		}

		cluster = clusterOwner->FindNextChildById(++cluster, EbmlElementId::Cluster);
	}

	if (auto && r = wm.Init()) return r;
	return 0;
}

// 从 .xxmv 读出数据并填充到 wm. 成功返回 0
inline int ReadXxmv(Webm& wm, std::filesystem::path const& path) {
	xx::BBuffer bb;
	xx::ReadAllBytes(path, bb);
	return bb.Read(wm);
}

inline int WriteXxmv(Webm const& wm, std::filesystem::path const& path) {
	xx::BBuffer bb;
	bb.Write(wm);
	return xx::WriteAllBytes("a.xxmv", bb);
}

#define SVPNG_LINKAGE inline
#define SVPNG_OUTPUT xx::BBuffer& bb
#define SVPNG_PUT(u) bb.Write((uint8_t)(u));
#include "svpng.inc"

#include "libyuv.h"

inline int Yuva2RgbaPng(std::filesystem::path const& fn
	, uint32_t const& w, uint32_t const& h
	, uint8_t const* const& yData, uint8_t const* const& uData, uint8_t const* const& vData, uint8_t const* const& aData
	, uint32_t const& yaStride, uint32_t const& uvStride) {

	// 数据容器
	std::vector<uint8_t> bytes;
	bytes.reserve(w * h * 4);

	// 产生像素坐标
	for (uint32_t _h = 0; _h < h; ++_h) {
		for (uint32_t _w = 0; _w < w; ++_w) {
			// 根据坐标结合具体宽高跨距算下标. uv 每个像素对应 ya 4个像素
			auto&& yaIdx = yaStride * _h + _w;
			auto&& uvIdx = uvStride * (_h / 2) + _w / 2;

			// 得到 yuv 原始数据, byte -> float
			auto&& y = yData[yaIdx] / 255.0f;
			auto&& u = uData[uvIdx] / 255.0f;
			auto&& v = vData[uvIdx] / 255.0f;

			// 进一步修正
			y = 1.1643f * (y - 0.0625f);
			u = u - 0.5f;
			v = v - 0.5f;

			// 算出 rgb( float 版 )
			auto&& r = y + 1.5958f * v;
			auto&& g = y - 0.39173f * u - 0.81290f * v;
			auto&& b = y + 2.017f * u;

			// 裁剪为 0 ~ 1
			if (r > 1.0f) r = 1.0f; else if (r < 0.0f) r = 0.0f;
			if (g > 1.0f) g = 1.0f; else if (g < 0.0f) g = 0.0f;
			if (b > 1.0f) b = 1.0f; else if (b < 0.0f) b = 0.0f;

			// 存起来
			bytes.push_back((uint8_t)(b * 255));
			bytes.push_back((uint8_t)(g * 255));
			bytes.push_back((uint8_t)(r * 255));
			bytes.push_back(aData ? aData[yaIdx] : (uint8_t)0);
		}
	}

	// 存为非压缩png
	xx::BBuffer bb;
	svpng(bb, w, h, bytes.data(), 1);
	return xx::WriteAllBytes(fn, bb);
}

// 这个 windows x64 下大约快 20+ 倍
inline int Yuva2RgbaPng2(std::filesystem::path const& fn
	, uint32_t const& w, uint32_t const& h
	, uint8_t const* const& yData, uint8_t const* const& uData, uint8_t const* const& vData, uint8_t const* const& aData
	, uint32_t const& yaStride, uint32_t const& uvStride) {

	std::vector<uint8_t> bytes;
	bytes.resize(w * h * 4);

	if(auto&& r = libyuv::I420AlphaToARGB(yData, yaStride, uData, uvStride, vData, uvStride, aData, yaStride, bytes.data(), w * 4, w, h, 0)) return r;

	xx::BBuffer bb;
	svpng(bb, w, h, bytes.data(), 1);
	return xx::WriteAllBytes(fn, bb);
}

#include "vpx_decoder.h"
#include "vp8dx.h"

inline int Xxmv2Pngs(Webm& wm) {
	vpx_codec_ctx_t ctx;
	vpx_codec_dec_cfg_t cfg{ 1, wm.width, wm.height };
	auto&& iface = wm.codecId ? vpx_codec_vp9_dx() : vpx_codec_vp8_dx();
	if (auto && r = vpx_codec_dec_init(&ctx, iface, &cfg, 0)) return r;	// because VPX_CODEC_OK == 0

	uint8_t const *rgbBuf = nullptr, *aBuf = nullptr;
	uint32_t rgbBufLen = 0, aBufLen = 0;

	if (wm.hasAlpha) {
		vpx_codec_ctx_t ctxAlpha;
		if (auto && r = vpx_codec_dec_init(&ctxAlpha, iface, &cfg, 0)) return r;

		for (uint32_t i = 0; i < wm.count; ++i) {
			if (auto && r = wm.GetFrameBuf(i, rgbBuf, rgbBufLen, aBuf, aBufLen)) return r;

			if (auto && r = vpx_codec_decode(&ctx, rgbBuf, rgbBufLen, nullptr, 0)) return r;
			if (auto && r = vpx_codec_decode(&ctxAlpha, aBuf, aBufLen, nullptr, 0)) return r;

			vpx_codec_iter_t iterator = nullptr;
			auto&& imgRGB = vpx_codec_get_frame(&ctx, &iterator);
			if (!imgRGB || imgRGB->fmt != VPX_IMG_FMT_I420) return -1;
			if (imgRGB->stride[1] != imgRGB->stride[2]) return -2;

			iterator = nullptr;
			auto&& imgA = vpx_codec_get_frame(&ctxAlpha, &iterator);
			if (!imgA || imgA->fmt != VPX_IMG_FMT_I420) return -3;
			if (imgA->stride[0] != imgRGB->stride[0]) return -4;

			auto&& fn = std::string("a") + std::to_string(i) + ".png";
			if (auto && r = Yuva2RgbaPng2(fn, wm.width, wm.height
				, imgRGB->planes[0], imgRGB->planes[1], imgRGB->planes[2], imgA->planes[0]
				, imgRGB->stride[0], imgRGB->stride[1])) return r;
		}
	}
	else {
		for (uint32_t i = 0; i < wm.count; ++i) {
			if (auto && r = wm.GetFrameBuf(i, rgbBuf, rgbBufLen)) return r;

			if (auto && r = vpx_codec_decode(&ctx, rgbBuf, rgbBufLen, nullptr, 0)) return r;

			vpx_codec_iter_t iterator = nullptr;
			auto&& imgRGB = vpx_codec_get_frame(&ctx, &iterator);
			if (!imgRGB || imgRGB->fmt != VPX_IMG_FMT_I420) return -1;
			if (imgRGB->stride[1] != imgRGB->stride[2]) return -2;

			auto&& fn = std::string("a") + std::to_string(i) + ".png";
			if (auto && r = Yuva2RgbaPng2(fn, wm.width, wm.height
				, imgRGB->planes[0], imgRGB->planes[1], imgRGB->planes[2], nullptr
				, imgRGB->stride[0], imgRGB->stride[1])) return r;

		}
	}

	return 0;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	Webm wm;
	if (auto && r = ReadWebm(wm, "a.webm")) return r;
	if (auto && r = WriteXxmv(wm, "a.xxmv")) return r;

	wm.Clear();
	if (auto && r = ReadXxmv(wm, "a.xxmv")) return r;

	if (auto && r = Xxmv2Pngs(wm)) return r;

	xx::CoutN("done");
	return 0;
}
