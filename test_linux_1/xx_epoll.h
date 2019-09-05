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
	enum class EpollSockTypes : int {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	struct EpollFDContext {
		List<uint8_t> recv;		// 收数据用堆积容器
		BufQueue sendQueue;		// 待发送队列

		inline void Clear(bool freeMemory = false) {
			recv.Clear(freeMemory);
			sendQueue.Clear(freeMemory);
		}
	};

	template<int timeoutMS = 100,int loopTimes = 100, int maxEvents = 64, int maxFD = 1000000, int readBufReserveIncrease = 65536, int sendLen = 65536, int vsCap = 1024>
	struct Epoll {

		int efd = -1;

		List<int> listenFDs;
		std::array<epoll_event, maxEvents> events;
		std::array<EpollFDContext, maxFD> ctxs;

		virtual void OnAccept(EpollFDContext& ctx, int const& listenFD, int const& sockFD) = 0;
		virtual void OnDisconnect(EpollFDContext & ctx, int const& sockFD) = 0;
		virtual void OnReceive(EpollFDContext & ctx, int const& sockFD) = 0;

		Epoll() {
			efd = epoll_create1(0);
		}
		virtual ~Epoll() {
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

		inline int Ctl(int fd, uint32_t flags) {
			epoll_event event;
			event.data.fd = fd;
			event.events = flags;
			if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event)) return -1;
			return 0;
		};

		// 用于一个 listener fd 存在于多个 epoll 多线程监听
		inline int ListenFD(int const& fd) {
			if (fd < 0) return -1;
			if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
			if (-1 == listen(fd, SOMAXCONN)) return -3;
			if( -1 == Ctl(fd, EPOLLIN | EPOLLET)) return -4;
			listenFDs.Add(fd);
			return 0;
		}

		inline int Listen(int const& port, EpollSockTypes const& sockType = EpollSockTypes::TCP) {
			auto&& fd = MakeListenFD(port, sockType);
			ScopeGuard sg([&] { close(fd); });
			if (int r = ListenFD(fd)) return r;
			sg.Cancel();
			return 0;
		}

		// todo: unlisten ?

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

		inline int RunOnce() {
			int counter = 0;
		LabBegin:
			int n = epoll_wait(efd, events.data(), maxEvents, timeoutMS);
			if (n == -1) return errno;
			for (int i = 0; i < n; ++i) {
				auto fd = events[i].data.fd;

				// disconnect
				if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
					OnDisconnect(ctxs[fd], fd);
					ctxs[fd].recv.Clear(true);
					ctxs[fd].sendQueue.Clear(true);
				}
				// accept
				else if (listenFDs.Find(fd) != (size_t)-1) {
					int sockFD = Accept(fd);
					if (sockFD < 0) break;
					ctxs[sockFD].recv.Reserve(readBufReserveIncrease);
					OnAccept(ctxs[sockFD], fd, sockFD);
				}
				// read, write
				else {
					EpollFDContext& ctx = ctxs[fd];
					if (events[i].events & EPOLLIN) {
						auto&& buf = ctx.recv;
						while (true) {
							buf.Reserve(buf.len + readBufReserveIncrease);
							auto&& len = read(fd, buf.buf + buf.len, buf.cap - buf.len);
							if (!len) goto LabForError;
							else if (len == -1) {
								if (errno == EAGAIN) break;
								goto LabForError;
							}
							else {
								ctx.recv.len += len;
								assert(ctx.recv.len <= ctx.recv.cap);
								OnReceive(ctx, fd);
							}
						}
					}
					if (events[i].events & EPOLLOUT) {
						// todo: 限制每 socket 每次发送总量
						while (ctx.sendQueue.bytes) {
							std::array<iovec, vsCap> vs;						// buf + len 数组指针
							int vsLen = 0;										// 数组长度
							size_t bufLen = sendLen;							// 计划发送字节数

							// 填充 vs, vsLen, bufLen 并返回预期 offset
							auto&& offset = ctx.sendQueue.Fill(vs, vsLen, bufLen);

							// 返回实际发出的字节数
							auto&& sentLen = writev(fd, vs.data(), vsLen);
							if (!sentLen) goto LabForError;
							else if (sentLen == -1) {
								if (errno == EAGAIN) break;
								goto LabForError;
							}
							else {
								if ((size_t)sentLen == bufLen) {
									ctx.sendQueue.Pop(vsLen, offset, bufLen);
								}
								else {
									ctx.sendQueue.Pop(sentLen);
								}
							}
						}
					}
				}
				continue;
			LabForError:
				ctxs[fd].Clear(true);
				close(fd);
				OnDisconnect(ctxs[fd], fd);
			}

			if (n == maxEvents) {
				if (++counter < loopTimes) goto LabBegin;
			}

			return 0;
		}
	};
}
