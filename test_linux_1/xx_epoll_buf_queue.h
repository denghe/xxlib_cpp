﻿#pragma once
#include "xx_epoll_buf.h"
namespace xx::Epoll {
	// todo: helpers ( 方便使用，比如从 BBuffer 剥离 buf + cap + len )
	// buf 队列。提供按字节数 pop 的功能
	struct BufQueue : protected Queue<Buf> {
		typedef Queue<Buf> BaseType;
		size_t bytes = 0;											// 剩余字节数 = sum( bufs.len ) - offset, pop & push 时会变化
		size_t offset = 0;											// 队列头部包已 pop 字节数

		BufQueue(BufQueue const& o) = delete;
		BufQueue& operator=(BufQueue const& o) = delete;

		explicit BufQueue(size_t const& capacity = 8)
			: BaseType(capacity) {
		}

		BufQueue(BufQueue&& o)
			: BaseType((BaseType&&)o) {
		}

		~BufQueue() {
			Clear();
		}

		void Clear(bool renewBuf = false) {
			this->BaseType::Clear();
			bytes = 0;
			offset = 0;
			if (renewBuf) {
				free(buf);
				auto bufByteLen = Round2n(8 * sizeof(Buf));
				buf = (Buf*)malloc((size_t)bufByteLen);
				assert(buf);
				cap = size_t(bufByteLen / sizeof(Buf));
			}
		}

		void Push(Buf&& eb) {
			assert(eb.len);
			bytes += eb.len;
			this->BaseType::Push(std::move(eb));
		}

		// 弹出指定字节数 for writev 返回值 < bufLen 的情况
		void Pop(size_t bufLen) {
			if (!bufLen) return;
			if (bufLen >= bytes) {
				Clear();
				return;
			}
			bytes -= bufLen;
			while (bufLen) {
				auto&& siz = Top().len - offset;
				if (siz > bufLen) {
					offset += bufLen;
					return;
				}
				else {
					bufLen -= siz;
					offset = 0;
					this->BaseType::Pop();
				}
			}
		}

		// 弹出指定个数 buf 并直接设定 offset ( 完整发送的情况 )
		void Pop(int const& vsLen, size_t const& offset, size_t const& bufLen) {
			bytes -= bufLen;
			if (vsLen == 1 && Top().len > this->offset + bufLen) {
				this->offset = offset + bufLen;
			}
			else {
				PopMulti(vsLen);
				this->offset = offset;
			}
		}

		// 填充指定字节数到 buf vec for writev 一次性发送多段
		// 回填 vs 长度, 回填 实际 bufLen, 返回 pop 后的预期 offset for PopUnsafe
		template<size_t vsCap>
		size_t Fill(std::array<iovec, vsCap>& vs, int& vsLen, size_t& bufLen) {
			assert(bufLen);
			assert(bytes);
			if (bufLen > bytes) {
				bufLen = bytes;
			}
			auto&& cap = (int)std::min(vsCap, this->BaseType::Count());
			auto o = &At(0);
			auto&& siz = o->len - offset;
			vs[0].iov_base = o->buf + offset;
			vs[0].iov_len = siz;
			if (siz > bufLen) {
				vsLen = 1;
				return offset + bufLen;
			}
			else if (siz == bufLen) {
				vsLen = 1;
				return 0;
			}
			auto&& len = bufLen - siz;
			for (int i = 1; i < cap; ++i) {
				o = &At(i);
				vs[i].iov_base = o->buf;
				if (o->len > len) {
					vs[i].iov_len = len;
					vsLen = i + 1;
					return len;
				}
				vs[i].iov_len = o->len;
				len -= o->len;
				if (!len) break;
			}
			vsLen = vsCap;
			bufLen -= len;
			return 0;
		}
	};
}
