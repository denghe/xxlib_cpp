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

		inline void Clear(bool freeMemory = false) {
			id = 0;
			sockFD = -1;
			listenFD = -1;
			recv.Clear(freeMemory);
			sendQueue.Clear(freeMemory);
		}
	};

	enum class EpollSockTypes : int {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	template<int timeoutMS = 100, int loopTimes = 100, int maxEvents = 64, int maxFD = 1000000, int maxNumListeners = 100, int readBufReserveIncrease = 65536, int sendLen = 65536, int vsCap = 1024>
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
			assert(-1 != efd);		// todo: assert? operator bool?
		}

		virtual ~Epoll() {
			// todo: 各种 close
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

		inline int SendTo(SockContext& ctx, xx::EpollBuf&& eb) {
			auto bytes = ctx.sendQueue.bytes;
			ctx.sendQueue.Push(std::move(eb));
			return !bytes ? Write(ctx.sockFD) : 0;
		}

		inline int RunOnce() {
			int counter = 0;
		LabBegin:
			int n = epoll_wait(efd, events.data(), maxEvents, timeoutMS);
			if (n == -1) return errno;
			for (int i = 0; i < n; ++i) {
				// get fd
				auto fd = events[i].data.fd;

				// error
				if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
					OnDisconnect(ctxs[fd]);
					ctxs[fd].Clear(true);
					continue;
				}

				// check fd is listener
				int idx = -1;
				for (int j = 0; j < listenFDsCount; ++j) {
					if (fd == listenFDs[j]) {
						idx = j;
						break;
					}
				}

				// accept
				if (idx != -1) {
					int sockFD = Accept(fd);
					if (sockFD < 0) break;
					ctxs[sockFD].id = ++id;
					ctxs[sockFD].sockFD = sockFD;
					ctxs[sockFD].listenFD = fd;
					ctxs[sockFD].recv.Reserve(readBufReserveIncrease);
					OnAccept(ctxs[sockFD], (int)idx);
					continue;
				}
				// read, write
				else {
					if (events[i].events & EPOLLIN) {
						if (Read(fd)) goto LabForError;
						if (OnReceive(ctxs[fd])) goto LabForError;
					}
					if (events[i].events & EPOLLOUT) {
						if (Write(fd)) goto LabForError;
					}
					continue;
				}

				// fd cleanup
			LabForError:
				close(fd);
				OnDisconnect(ctxs[fd]);
				ctxs[fd].Clear(true);
			}

			// limit handle times per frame
			if (n == maxEvents) {
				if (++counter < loopTimes) goto LabBegin;
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
			sg.Cancel();
			return fd;
		}

		// 向 sockFD 发送 len 字节数据. 
		inline int Write(int const& fd, int const& len = sendLen) {
			SockContext& ctx = ctxs[fd];
			while (ctx.sendQueue.bytes) {
				std::array<iovec, vsCap> vs;						// buf + len 数组指针
				int vsLen = 0;										// 数组长度
				size_t bufLen = len;								// 计划发送字节数

				// todo: 限制每 socket 每次发送总量

				// 填充 vs, vsLen, bufLen 并返回预期 offset
				auto&& offset = ctx.sendQueue.Fill(vs, vsLen, bufLen);

				// 返回实际发出的字节数
				auto&& sentLen = writev(fd, vs.data(), vsLen);
				if (!sentLen) return -1;
				else if (sentLen == -1) return errno == EAGAIN ? 0 : -2;
				else {
					if ((size_t)sentLen == bufLen) {
						ctx.sendQueue.Pop(vsLen, offset, bufLen);
					}
					else {
						ctx.sendQueue.Pop(sentLen);
					}
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
					assert(buf.len <= buf.cap);
				}
			}
			return 0;
		}



		inline static std::atomic<uint32_t> id;		// 用于生成自增 id

		// 当前 epoll 的 fd
		int efd = -1;

	public:
		// 所有 listener 的 fd 集合
		std::array<int, maxNumListeners> listenFDs;
		int listenFDsCount = 0;
	protected:

		// 用于 epoll 填充事件的容器
		std::array<epoll_event, maxEvents> events;

		// fd 读写上下文
		std::array<SockContext, maxFD> ctxs;

	};
}
