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

#include "xx_bbuffer.h"
#include "xx_queue.h"
#include "xx_coros_boost.h"
#include "xx_epoll_buf.h"
#include "xx_epoll_buf_queue.h"

namespace xx {

	struct SockContext {
		uint32_t id = 0;			// 自增 id
		int sockFD = -1;			// 原始 fd
		int listenFD = -1;			// 原始 listenFD
		List<uint8_t> recv;			// 收数据用堆积容器
		BufQueue sendQueue;			// 待发送队列
		bool writing = false;		// 是否正在发送( 是：不管 sendQueue 空不空，都不能 write, 只能塞 sendQueue )

		inline void Clear(bool freeMemory = false) {
			id = 0;
			sockFD = -1;
			listenFD = -1;
			recv.Clear(freeMemory);
			sendQueue.Clear(freeMemory);
			writing = false;
		}
	};

	enum class EpollSockTypes : int {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	// todo: udp + kcp ?

	template<int frameTimeoutMS = 100, int maxLoopTimesPerFrame = 48, int maxNumEvents = 1000, int maxNumFD = 100000, int maxNumListeners = 10, int readBufReserveIncrease = 65536, int sendLenPerFrame = 65536, int vsCap = 1024>
	struct Epoll {

		// 用户事件
		virtual void OnAccept(SockContext& sctx, int const& listenIndex) = 0;
		virtual void OnDisconnect(SockContext& sctx) = 0;
		virtual int OnReceive(SockContext& sctx) = 0;

		// 判断 epoll 是否创建成功
		inline operator bool() {
			return efd != -1;
		}

		Epoll() {
			efd = epoll_create1(0);
		}

		virtual ~Epoll() {
			// todo: 各种 close
		}

		// 和其他 Epoll 共享 FD
		inline int ListenFD(int const& fd) {
			if (-1 == Ctl(fd, EPOLLIN | EPOLLET)) return -1;
			listenFDs[listenFDsCount++] = fd;
			return 0;
		}

		inline int Listen(int const& port, EpollSockTypes const& sockType = EpollSockTypes::TCP) {
			auto&& fd = MakeListenFD(port, sockType);
			if (fd < 0) return -1;
			ScopeGuard sg([&] { close(fd); });
			if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
			if (-1 == listen(fd, SOMAXCONN)) return -3;
			if (-1 == Ctl(fd, EPOLLIN | EPOLLET)) return -4;
			listenFDs[listenFDsCount++] = fd;
			sg.Cancel();
			return 0;
		}

		// todo: unlisten ?

		inline void Send(SockContext& ctx, xx::EpollBuf&& eb) {
			ctx.sendQueue.Push(std::move(eb));
		}

		inline int RunOnce() {
			int counter = 0;
		LabBegin:

			// todo: epoll_pwait ?
			int n = epoll_wait(efd, events.data(), maxNumEvents, frameTimeoutMS);
			if (n == -1) return errno;
			for (int i = 0; i < n; ++i) {
				// get fd
				auto fd = events[i].data.fd;
				SockContext& ctx = ctxs[fd];

				// error
				if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) goto LabForError;

				// accept
				for (int idx = 0; idx < listenFDsCount; ++idx) {
					if (fd == listenFDs[idx]) {
						int sockFD = Accept(fd);
						if (sockFD >= 0) {
							OnAccept(ctxs[sockFD], (int)idx);
						}
						goto LabContinue;
					}
				}
				// read
				if (events[i].events & EPOLLIN) {
					if (Read(fd)) goto LabForError;
					if (OnReceive(ctx)) goto LabForError;
				}
				// write
				if (events[i].events & EPOLLOUT) {
					ctx.writing = false;
				}
				if (!ctx.writing && ctx.sendQueue.bytes) {
					if (Write(fd)) goto LabForError;
				}

			LabContinue:
				continue;

				// fd cleanup
			LabForError:
				OnDisconnect(ctx);
				ctx.Clear(true);
				close(fd);
			}

			// limit handle times per frame
			if (n == maxNumEvents) {
				if (++counter < maxLoopTimesPerFrame) goto LabBegin;
			}

			return 0;
		}

	protected:

		inline int Ctl(int fd, uint32_t flags) {
			epoll_event event;
			event.data.fd = fd;
			event.events = flags;
			if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event)) return -1;
			return 0;
		};

		// return value < 0: error
		inline int Accept(int const& listenFD) {
			sockaddr in_addr;									// todo: ipv6 support
			socklen_t inLen = sizeof(in_addr);
			int fd = accept(listenFD, &in_addr, &inLen);
			if (-1 == fd) return -1;
			ScopeGuard sg([&] { close(fd); });
			if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
			if (-1 == Ctl(fd, EPOLLIN | EPOLLOUT | EPOLLET)) return -3;
			auto&& ctx = ctxs[fd];
			ctx.id = ++id;
			ctx.sockFD = fd;
			ctx.listenFD = listenFD;
			sg.Cancel();
			return fd;
		}

		// 向 sockFD 发送 len 字节数据. 
		inline int Write(int const& fd, int const& len = sendLenPerFrame) {
			SockContext& ctx = ctxs[fd];
			while (ctx.sendQueue.bytes) {
				std::array<iovec, vsCap> vs;						// buf + len 数组指针
				int vsLen = 0;										// 数组长度
				size_t bufLen = len;								// 计划发送字节数

				// 填充 vs, vsLen, bufLen 并返回预期 offset
				auto&& offset = ctx.sendQueue.Fill(vs, vsLen, bufLen);

				// 返回值为 实际发出的字节数
				auto&& sentLen = writev(fd, vs.data(), vsLen);

				// todo: 限制每 socket 每次发送总量? 限速?

				if (!sentLen) return -1;
				else if (sentLen == -1) {
					if (errno == EAGAIN) {
						ctx.writing = true;
						return 0;
					}
					return -2;
				}
				else if ((size_t)sentLen == bufLen) {
					ctx.sendQueue.Pop(vsLen, offset, bufLen);
				}
				else {
					ctx.sendQueue.Pop(sentLen);
					return 0;										// 理论上讲如果只写入成功一部分, 不必 retry 了。这点需要验证
				}
			}
			return 0;
		}

		inline int Read(int const& fd) {
			auto&& ctx = ctxs[fd];
			auto&& buf = ctx.recv;
			while (true) {
				buf.Reserve(buf.len + readBufReserveIncrease);
				auto&& len = read(fd, buf.buf + buf.len, buf.cap - buf.len);
				if (!len) return -1;
				else if (len == -1) return errno == EAGAIN ? 0 : -2;
				else {
					buf.len += len;
					if (buf.len <= buf.cap) return 0;				// 理论上讲如果连 buf 都没读满, 不必 retry 了。这点需要验证
				}
			}
			return 0;
		}

		inline static int MakeListenFD(int const& port, EpollSockTypes const& sockType = EpollSockTypes::TCP) {
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

		// 用于生成唯一自增 id
		inline static std::atomic<uint32_t> id;

		// 当前 epoll 的 fd
		int efd = -1;

	public:
		// 所有 listener 的 fd 集合
		std::array<int, maxNumListeners> listenFDs;
		int listenFDsCount = 0;
	protected:

		// 用于 epoll 填充事件的容器
		std::array<epoll_event, maxNumEvents> events;

		// fd 读写上下文
		inline static std::array<SockContext, maxNumFD> ctxs;

	};
}
