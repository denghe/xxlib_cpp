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

	enum class EpollMessageTypes {
		Unknown,
		Accept,
		Disconnect,
		Read
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
		
		// todo: 支持传入 ListenFD? 多个 FD?

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
