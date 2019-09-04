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
#include <errno.h>
#include <boost/context/all.hpp>

#include "xx_bbuffer.h"
#include "xx_queue.h"


namespace xx {

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

	struct EpollBuf {
		uint8_t* buf = nullptr;
		size_t len = 0;
		EpollBuf() = default;
		EpollBuf(EpollBuf const&) = delete;
		EpollBuf& operator=(EpollBuf const&) = delete;
		EpollBuf(EpollBuf&&) = default;
		EpollBuf& operator=(EpollBuf&&) = default;
		~EpollBuf() {
			if (buf) {
				::free(buf);
				buf = nullptr;
			}
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
		xx::Buffer recv;
		// todo: send queue( 从 uv 旧封装找出 BytesQueue 代码 )
	};

	template<int timeoutMS = 100, int maxEvents = 64, int maxFD = 1000000, int readBufReserveIncrease = 65536>
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
			if (-1 == ::epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event)) return __LINE__;
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
			if (listenFD < 0) return __LINE__;
			if (-1 == fcntl(listenFD, F_SETFL, fcntl(listenFD, F_GETFL, 0) | O_NONBLOCK)) return __LINE__;
			if (-1 == ::listen(listenFD, SOMAXCONN)) return __LINE__;
			return Ctl(listenFD, EPOLLIN | EPOLLET);
		}

		// return value < 0: error
		inline int Accept() {
			sockaddr in_addr;									// todo: ipv6 support
			socklen_t inLen = sizeof(in_addr);
			int fd = accept(listenFD, &in_addr, &inLen);
			if (-1 == fd) return -1;
			xx::ScopeGuard sg([&] { close(fd); });
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
				}
				else {
					auto&& fd = events[i].data.fd;
					xx::Buffer&& buf = ctxs[fd].recv;
					if (events[i].events & EPOLLIN) {
						while (true) {
							buf.Reserve(buf.len + readBufReserveIncrease);
							auto&& count = ::read(fd, buf.buf + buf.len, buf.cap - buf.len);
							if (count == -1) {
								if (errno == EAGAIN) break;
								else {
									buf.Clear(/* true */);
									goto LabEnd;
								}
							}
						}
					}
					if (events[i].events & EPOLLOUT) {
						// todo: read data from send queue & ::write(fd, ....
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
