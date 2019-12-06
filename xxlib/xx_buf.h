#pragma once
#include "xx_bbuffer.h"
#include "xx_queue.h"

namespace xx {
	// 引用计数放在 buf[len] 的后面. 从 BBuffer 剥离时 对齐追加一个定长 int 的空间但是不改变长度
	struct Buf {

		// 指向内存块
		uint8_t* buf;

		// 内存块内有效数据长度
		size_t len;

		// 指向内存块中引用计数变量
		int* refs;

		// 从 bb 直接移走内存
		template<typename BB>
		Buf(BB& bb) {
			Init<BB>(bb);
		}

		// 重设数据
		inline void Reset(BBuffer& bb) {
			Dispose();
			Init(bb);
		}

		Buf()
			: buf(nullptr)
			, len(0)
			, refs(nullptr)
		{
		}

		// 增加引用计数
		Buf(Buf const& o)
			: buf(o.buf)
			, len(o.len)
			, refs(o.refs)
		{
			if (refs) {
				++(*refs);
			}
		}

		// 复制内存
		Buf(void const* const& buf, size_t const& len) {
			Init(malloc(len + 8), len);
			memcpy(this->buf, buf, len);
		}

		// unsafe: 直接使用传入的内存( len 后面还需要 refs 的存储空间 )
		Buf(size_t const& len, void* const& buf) {
			Init(buf, len);
		}

		~Buf() {
			Dispose();
		}

		Buf(Buf&& o)
			: buf(o.buf)
			, len(o.len)
			, refs(o.refs)
		{
			o.buf = nullptr;
			o.len = 0;
			o.refs = nullptr;
		}

		inline Buf& operator=(Buf&& o) {
			std::swap(this->buf, o.buf);
			std::swap(this->len, o.len);
			std::swap(this->refs, o.refs);
			return *this;
		}

		inline Buf& operator=(Buf const& o) {
			if (this == &o) return *this;
			Dispose();
			this->buf = o.buf;
			this->len = o.len;
			this->refs = o.refs;
			++(*refs);
			return *this;
		}

	protected:
		inline void Dispose() {
			if (buf) {
				if (-- * refs == 0) {
					::free(buf);
				}
				buf = nullptr;
			}
		}

		template<typename BB>
		inline void Init(BB& bb) {
			// 为 refs 扩容
			bb.Reserve(bb.len + 8);

			// 复制 buf 信息并填充 refs
			Init(bb.buf, bb.len);

			// 内存脱钩
			bb.Reset();
		}

		// buf 需要有额外的存 refs 的内存空间
		inline void Init(void* const& buf, size_t const& len) {
			// 直读内存信息
			this->buf = (uint8_t*)buf;
			this->len = len;

			// 计算出 refs 的 4 字节对齐位置
			refs = (int*)(((((size_t)buf + len - 1) / 4) + 1) * 4);

			// 初始引用计数为 1
			*refs = 1;
		}
	};
}
namespace xx {
	// 标识内存可移动
	template<>
	struct IsTrivial<Buf, void> {
		static const bool value = true;
	};
}
