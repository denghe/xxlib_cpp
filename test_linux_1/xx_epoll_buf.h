#pragma once
#include "xx_bbuffer.h"
#include "xx_queue.h"

namespace xx {
	// 引用计数放在 buf[len] 的后面. 从 BBuffer 剥离时 对齐追加一个定长 int 的空间但是不改变长度
	struct EpollBuf {

		// 指向内存块
		uint8_t* buf;

		// 内存块内有效数据长度
		size_t len;

		// 指向内存块中引用计数变量
		int* refs;

		EpollBuf(BBuffer& bb) {
			Init(bb);
		}

		inline void Reset(BBuffer& bb) {
			Dispose();
			Init(bb);
		}

		EpollBuf()
			: buf(nullptr)
			, len(0)
			, refs(nullptr)
		{
		}

		EpollBuf(EpollBuf const& o)
			: buf(o.buf)
			, len(o.len)
			, refs(o.refs)
		{
			if (refs) {
				++(*refs);
			}
		}

		~EpollBuf() {
			Dispose();
		}

		EpollBuf(EpollBuf&& o)
			: buf(o.buf)
			, len(o.len)
			, refs(o.refs)
		{
			o.buf = nullptr;
			o.len = 0;
			o.refs = nullptr;
		}

		inline EpollBuf& operator=(EpollBuf&& o) {
			std::swap(this->buf, o.buf);
			std::swap(this->len, o.len);
			std::swap(this->refs, o.refs);
			return *this;
		}

		inline EpollBuf& operator=(EpollBuf const& o) {
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

		inline void Init(BBuffer& bb) {
			// 为 refs 扩容
			bb.Reserve(bb.len + 8);

			// 直读内存信息
			buf = bb.buf;
			len = bb.len;

			// 计算出 refs 的 4 字节对齐位置
			refs = (int*)(((((size_t)buf + len - 1) / 4) + 1) * 4);

			*refs = 1;

			// 内存脱钩
			bb.Reset();
		}
	};

	// 标识内存可移动
	template<>
	struct IsTrivial<EpollBuf, void> {
		static const bool value = true;
	};
}