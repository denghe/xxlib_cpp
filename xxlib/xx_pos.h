#pragma once
#include "xx_bbuffer.h"
#include <math.h>

namespace xx
{
	// 用来存坐标
	struct Pos
	{
		float x = 0, y = 0;

		inline bool operator==(const Pos& v) const noexcept {
			return x == v.x && y == v.y;
		}
		inline Pos& operator+=(Pos const& v) noexcept {
			x += v.x;
			y += v.y;
			return *this;
		}
		inline Pos operator+(Pos const& v) const noexcept {
			return Pos{ x + v.x, y + v.y };
		}
		inline Pos operator-(Pos const& v) const noexcept {
			return Pos{ x - v.x, y - v.y };
		}
		inline Pos operator*(float const& s) const noexcept {
			return Pos{ x * s, y * s };
		}
		inline Pos operator/(float const& s) const noexcept {
			return Pos{ x / s, y / s };
		}

#ifdef CC_TARGET_PLATFORM
		inline operator cocos2d::Vec2() const noexcept {
			return *(cocos2d::Vec2*)this;
		}
#endif
	};

	// 适配 Pos 之 ToString
	template<>
	struct SFuncs<Pos, void> {
		static inline void WriteTo(std::string& s, Pos const &in) {
			Append(s, "{ \"x\":", in.x, ", \"y\":", in.y, " }");
		}
	};

	// 适配 Pos 之 序列化 & 反序列化
	template<>
	struct BFuncs<Pos, void> {
		static inline void WriteTo(BBuffer& bb, Pos const &in) {
			bb.Reserve(bb.len + sizeof(Pos));
			memcpy(bb.buf + bb.len, &in, sizeof(Pos));
			bb.len += sizeof(Pos);
		}
		static inline int ReadFrom(BBuffer& bb, Pos &out) {
			if (bb.offset + sizeof(Pos) > bb.len) return -1;
			memcpy(&out, bb.buf + bb.offset, sizeof(Pos));
			bb.offset += sizeof(Pos);
			return 0;
		}
	};

	// 判断两线段( p0-p1, p2-p3 )是否相交, 并且往 p 填充交点
	inline bool GetSegmentIntersection(Pos const& p0, Pos const& p1, Pos const& p2, Pos const& p3, Pos* const& p = nullptr) noexcept {
		Pos s02, s10, s32;
		float s_numer, t_numer, denom, t;
		s10.x = p1.x - p0.x;
		s10.y = p1.y - p0.y;
		s32.x = p3.x - p2.x;
		s32.y = p3.y - p2.y;

		denom = s10.x * s32.y - s32.x * s10.y;
		if (denom == 0) return false; // Collinear
		bool denomPositive = denom > 0;

		s02.x = p0.x - p2.x;
		s02.y = p0.y - p2.y;
		s_numer = s10.x * s02.y - s10.y * s02.x;
		if ((s_numer < 0) == denomPositive) return false; // No collision

		t_numer = s32.x * s02.y - s32.y * s02.x;
		if ((t_numer < 0) == denomPositive) return false; // No collision

		if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive)) return false; // No collision

		t = t_numer / denom;		// Collision detected
		if (p) {
			p->x = p0.x + (t * s10.x);
			p->y = p0.y + (t * s10.y);
		}
		return true;
	}

	// 计算直线的弧度( 转为角度还要  * (180.0f / float(M_PI) )
	inline float GetAngle(Pos const& from, Pos const& to) noexcept
	{
		if (from == to) return 0.0f;
		auto&& len_y = to.y - from.y;
		auto&& len_x = to.x - from.x;
		return atan2f(len_y, len_x);
	}
	inline float GetAngle(std::pair<Pos, Pos> const& fromTo) noexcept {
		return GetAngle(fromTo.first, fromTo.second);
	}

	// 计算距离
	inline float GetDistance(Pos const& a, Pos const& b) noexcept
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		return sqrtf(dx * dx + dy * dy);
	}
	inline float GetDistance(std::pair<Pos, Pos> const& fromTo) noexcept {
		return GetDistance(fromTo.first, fromTo.second);
	}

	// 以 0,0 为中心旋转. a 为弧度( 角度 * (float(M_PI) / 180.0f) )
	inline Pos Rotate(Pos const& pos, float const& a) noexcept
	{
		auto&& sinA = sinf(a);
		auto&& cosA = cosf(a);
		return Pos{ pos.x * cosA - pos.y * sinA, pos.x * sinA + pos.y * cosA };
	}
}
