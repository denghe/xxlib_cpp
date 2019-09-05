#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <errno.h>
#include <boost/context/all.hpp>

#include "xx_bbuffer.h"
#include "xx_queue.h"

#include "xx_uv.h"


namespace xx {

	xx::Uv uv;
	xx::UvDialer dialer(uv);

	struct Coros;
	struct Coro {
		boost::context::continuation c1;
		boost::context::continuation c2;
		size_t idx = -1;
		Coros& coros;

		void operator()() {
			c2 = c2.resume();
		}

		Coro(Coros& coros, boost::context::continuation&& c);
		~Coro();
		Coro(Coro&&) = delete;
		Coro& operator=(Coro&&) = delete;
		Coro(Coro&) = delete;
		Coro& operator=(Coro&) = delete;
	};

	struct Coros {
		List<Coro*> cs;

		void Add(std::function<void(Coro& yield)>&& job, bool runImmediately = false) {

			auto&& c1 = boost::context::callcc([this, job = std::move(job)](boost::context::continuation&& c2) {
				Coro co(*const_cast<Coros*>(this), std::move(c2));
				co.c2 = co.c2.resume();
				job(co);
				return std::move(co.c2);
			});

			auto&& c = cs[cs.len - 1]->c1;
			c = std::move(c1);

			if (runImmediately) {
				c = c.resume();
			}
		}

		void RunOnce() {
			for (int i = (int)cs.len - 1; i >= 0; --i) {
				auto&& co = cs[i];
				co->c1 = co->c1.resume();
			}
		}

		void Run() {
			while (cs.len) {
				RunOnce();
			}
		}
	};

	inline Coro::Coro(Coros& coros, boost::context::continuation&& c)
		: c2(std::move(c))
		, coros(coros)
	{
		idx = coros.cs.len;
		coros.cs.Add(this);
	}

	inline Coro::~Coro() {
		coros.cs[coros.cs.len - 1]->idx = idx;
		coros.cs.SwapRemoveAt(idx);
	}





	enum class EpollMessageTypes {
		Unknown,
		Accept,
		Disconnect,
		Read
	};

	// 引用计数放在 buf[len] 的后面. 从 BBuffer 剥离时 对齐追加一个定长 int 的空间但是不改变长度
	struct EpollBuf {

		// 指向内存块
		uint8_t* buf;

		// 内存块内有效数据长度
		size_t len;

		// 指向内存块中引用计数变量
		int* refs;

		EpollBuf()
			: buf(nullptr)
			, len(0)
			, refs(nullptr)
		{
		}

		EpollBuf(BBuffer& bb) {
			// 为 refs 扩容
			bb.Reserve(bb.len + 8);

			// 直读内存信息
			buf = bb.buf;
			len = bb.len;

			// 计算出 refs 的 4 字节对齐位置
			refs = (int*)(((((size_t)buf + len - 1) / 4) + 1) * 4);

			// 内存脱钩
			bb.Reset();
		}

		EpollBuf(EpollBuf const& o)
			: buf(o.buf)
			, len(o.len)
			, refs(o.refs)
		{
			++(*refs);
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

		EpollBuf& operator=(EpollBuf&& o) {
			std::swap(this->buf, o.buf);
			std::swap(this->len, o.len);
			std::swap(this->refs, o.refs);
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
	};

	// 标识内存可移动
	template<>
	struct IsTrivial<EpollBuf, void> {
		static const bool value = true;
	};

	// buf 队列。提供按字节数 pop 的功能
	struct BufQueue : protected Queue<std::shared_ptr<EpollBuf>> {
		typedef Queue<std::shared_ptr<EpollBuf>> BaseType;
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

		// todo: helpers ( 方便使用，比如从 BBuffer 剥离 buf + cap + len )

		void Clear() {
			this->BaseType::Clear();
			bytes = 0;
			offset = 0;
		}

		// 压入 buf + len
		void Push(std::shared_ptr<EpollBuf>&& eb) {
			assert(eb && eb->len);
			bytes += eb->len;
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
				auto&& siz = Top()->len - offset;
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
			if (vsLen == 1 && Top()->len > this->offset + bufLen) {
				this->offset = offset + bufLen;
			}
			else {
				PopMulti(vsLen);
				this->offset = offset;
			}
		}

		// 填充指定字节数到 buf vec for writev 一次性发送多段
		// 回填 vs 长度, 回填 实际 bufLen
		// 返回 pop 后的预期 offset for PopUnsafe
		template<size_t vsCap>
		size_t Fill(std::array<iovec, vsCap>& vs, int& vsLen, size_t& bufLen) {
			assert(bufLen);
			assert(bytes);
			if (bufLen > bytes) {
				bufLen = bytes;
			}
			auto&& cap = std::min(vsCap, this->BaseType::Count());
			auto o = &*At(0);
			auto&& siz = o->len - offset;
			vs[0].iov_base = o->buf + offset;
			vs[0].iov_len = siz;
			if (siz > bufLen) {
				vsLen = 1;
				return offset + bufLen;
			}
			auto&& len = bufLen - siz;
			for (int i = 0; i < vsCap; ++i) {
				o = &*At(i);
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

	struct EpollMessage {
		EpollMessageTypes type = EpollMessageTypes::Unknown;
		int fd = 0;
		EpollBuf buf;

		EpollMessage() = default;
		EpollMessage(EpollMessage const&) = delete;
		EpollMessage& operator=(EpollMessage const&) = delete;
		EpollMessage(EpollMessage&&) = default;
		EpollMessage& operator=(EpollMessage&&) = default;
	};

	enum class EpollSockTypes : int {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	struct EpollFDContext {
		// 收数据用堆积容器
		List<uint8_t> recv;

		// 待发送队列
		BufQueue sendQueue;
	};

	template<int timeoutMS = 100, int maxEvents = 64, int maxFD = 1000000, int readBufReserveIncrease = 65536, int sendLen = 65536, int vsCap = 1024>
	struct Epoll {

		int efd = -1;
		int listenFD = -1;
		std::array<epoll_event, maxEvents> events;
		std::array<EpollFDContext, maxFD> ctxs;
		std::vector<EpollMessage> msgs;

		Epoll() {
			efd = epoll_create1(0);
		}

		inline int Ctl(int fd, uint32_t flags) {
			epoll_event event;
			event.data.fd = fd;
			event.events = flags;
			if (-1 == ::epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event)) return -1;
			return 0;
		};

		inline int MakeFD(int const& port, EpollSockTypes const& sockType) {
			char portStr[20];
			snprintf(portStr, sizeof(portStr), "%d", port);

			addrinfo hints;														// todo: ipv6 support
			memset(&hints, 0, sizeof(addrinfo));
			hints.ai_family = AF_UNSPEC;										// ipv4 / 6
			hints.ai_socktype = (int)sockType;									// SOCK_STREAM / SOCK_DGRAM
			hints.ai_flags = AI_PASSIVE;										// all interfaces

			addrinfo* ai_, * ai;
			if (getaddrinfo(nullptr, portStr, &hints, &ai_)) return -1;

			int fd;
			for (ai = ai_; ai != nullptr; ai = ai->ai_next) {
				fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
				if (fd == -1) continue;

				int enable = 1;
				if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
					close(fd);
					continue;
				}
				if (!bind(fd, ai->ai_addr, ai->ai_addrlen)) break;				// success

				close(fd);
			}
			freeaddrinfo(ai_);

			return ai ? fd : -2;
		}

		inline int Listen(int const& port, EpollSockTypes const& sockType) {
			listenFD = MakeFD(port, sockType);
			if (listenFD < 0) return -1;
			if (-1 == fcntl(listenFD, F_SETFL, fcntl(listenFD, F_GETFL, 0) | O_NONBLOCK)) return -2;
			if (-1 == ::listen(listenFD, SOMAXCONN)) return -3;
			return Ctl(listenFD, EPOLLIN | EPOLLET);
		}

		// return value < 0: error
		inline int Accept() {
			sockaddr in_addr;									// todo: ipv6 support
			socklen_t inLen = sizeof(in_addr);
			int fd = accept(listenFD, &in_addr, &inLen);
			if (-1 == fd) return -1;
			ScopeGuard sg([&] { close(fd); });
			if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
			if (-1 == Ctl(fd, EPOLLIN | EPOLLOUT | EPOLLET)) return -3;
			sg.Cancel();
			return fd;
		}

		inline void RunOnce() {
			int numEvents = epoll_wait(efd, events.data(), maxEvents, timeoutMS);
			for (int i = 0; i < numEvents; ++i) {
				if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
					auto&& m = msgs.emplace_back();
					m.fd = events[i].data.fd;
					m.type = EpollMessageTypes::Disconnect;
				}
				else if (listenFD == events[i].data.fd) {				// new connection
					int fd = Accept();
					if (fd < 0) break;
					auto&& m = msgs.emplace_back();
					m.fd = events[i].data.fd;
					m.type = EpollMessageTypes::Accept;
					ctxs[fd].recv.Reserve(readBufReserveIncrease);
				}
				else {
					auto&& fd = events[i].data.fd;
					EpollFDContext& ctx = ctxs[fd];
					if (events[i].events & EPOLLIN) {
						auto&& buf = ctx.recv;
						while (true) {
							buf.Reserve(buf.len + readBufReserveIncrease);
							auto&& count = ::read(fd, buf.buf + buf.len, buf.cap - buf.len);
							if (count == -1) {
								if (errno == EAGAIN) break;
								else {
									buf.Clear(true);
									goto LabEnd;
								}
							}
						}
					}
					if (events[i].events & EPOLLOUT) {
						if (ctx.sendQueue.bytes) {
							std::array<iovec, vsCap> vs;						// buf + len 数组指针
							int vsLen = 0;										// 数组长度
							size_t bufLen = sendLen;							// 计划发送字节数

							// 填充 vs, vsLen, bufLen 并返回预期 offset
							auto&& offset = ctx.sendQueue.Fill(vs, vsLen, bufLen);

							// 返回实际发出的字节数
							auto&& sentLen = ::writev(fd, vs.data(), vsLen);
							if (sentLen == -1) {
								if (errno == EAGAIN) break;
								else {
									ctx.sendQueue.Clear();
									goto LabEnd;
								}
							}

							if (sentLen == bufLen) {
								ctx.sendQueue.Pop(vsLen, offset, bufLen);
							}
							else {
								ctx.sendQueue.Pop(sentLen);
							}
						}
					}
				}
			LabEnd:;
			}
		}
	};
}

int main() {

	return 0;
}




//int main() {
//	xx::Coros cs;
//	cs.Add([&](xx::Coro& yield) {
//		for (size_t i = 0; i < 10; i++) {
//			xx::CoutN(i);
//			yield();
//		}
//		});
//	cs.Run();
//	xx::CoutN("end");
//	std::cin.get();
//	return 0;
//}

//
//int main() {
//	auto&& c = boost::context::callcc([](boost::context::continuation&& c) {
//	    for (size_t i = 0; i < 10; i++) {
//			xx::CoutN(i);
//			c = c.resume();
//		}
//		return std::move(c);
//		});
//	while (c) {
//		xx::CoutN("."); c = c.resume();
//	}
//	std::cin.get();
//	return 0;
//}

//#include "xx_epoll_context.h"
//
//int main() {
//
//	// echo server sample
//	xx::EpollListen(1234, xx::SockTypes::TCP, 2, [](int fd, auto read, auto write) {
//			printf("peer accepted. fd = %i\n", fd);
//			char buf[1024];
//			while (size_t received = read(buf, sizeof(buf))) {
//				if (write(buf, received)) break;
//			}
//			printf("peer disconnected: fd = %i\n", fd);
//		}
//	);
//	return 0;
//}
