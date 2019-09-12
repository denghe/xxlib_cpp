// base copy from https://github.com/matt-42/moustique
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

#include <thread>
#include <vector>
#include <boost/context/all.hpp>

namespace xx {
	enum class SockTypes : int {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	inline int MakeFD(int const& port, SockTypes const& sockType) {
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

	template <int maxEvents = 10000, int maxFD = 1000000, typename H>
	int EpollListenFD(int const& listenFD, int const& numThreads, H&& peerHandler) {
		namespace bctx = boost::context;

		if (listenFD < 0) return __LINE__;
		if (-1 == fcntl(listenFD, F_SETFL, fcntl(listenFD, F_GETFL, 0) | O_NONBLOCK)) return __LINE__;
		if (-1 == ::listen(listenFD, SOMAXCONN)) return __LINE__;

		std::vector<bctx::continuation> fibers;		// todo: autoinc peer id mapping?
		fibers.resize(maxFD);

		auto&& event_loop_fn = [listenFD, &fibers, &peerHandler] {
			int efd = epoll_create1(0);
			auto&& epoll_ctl = [efd](int fd, uint32_t flags) -> int {
				epoll_event event;
				event.data.fd = fd;
				event.events = flags;
				if (-1 == ::epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event)) return __LINE__;
				return 0;
			};
			auto&& r = epoll_ctl(listenFD, EPOLLIN | EPOLLET);
			assert(!r);

			std::vector<epoll_event> events;
			events.resize(maxEvents);
			while (true) {
				int numEvents = epoll_wait(efd, events.data(), maxEvents, -1);
				for (int i = 0; i < numEvents; ++i) {
					if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
						fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
						continue;
					}
					else if (listenFD == events[i].data.fd) {					// new connection
						while (true) {
							sockaddr in_addr;									// todo: ipv6 support
							socklen_t inLen = sizeof(in_addr);
							int fd = accept(listenFD, &in_addr, &inLen);
							if (fd == -1) break;

							//char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
							//getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);

							if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return __LINE__;

							r = epoll_ctl(fd, EPOLLIN | EPOLLOUT | EPOLLET);
							assert(!r);

							fibers[fd] = bctx::callcc([fd, &peerHandler](bctx::continuation&& c) {
								auto&& read = [fd, &c](void* const& buf, size_t const& bufLen)->size_t {
									auto&& count = ::read(fd, buf, bufLen);
									while (count <= 0) {
										if ((count < 0 && errno != EAGAIN) || count == 0) return 0;		// error
										c = c.resume();
										count = ::read(fd, buf, bufLen);
									}
									return (size_t)count;
								};

								auto&& write = [fd, &c](char const* buf, size_t const& dataLen)->int {
									auto&& count = ::write(fd, buf, dataLen);
									if (count > 0) {
										buf += count;
									}
									auto&& end = buf + dataLen;
									while (buf != end) {
										if ((count < 0 && errno != EAGAIN) || count == 0) return -1;	// error
										c = c.resume();
										count = ::write(fd, buf, end - buf);
										if (count > 0) buf += count;
									}
									return 0;
								};

								peerHandler(fd, read, write);
								close(fd);
								return std::move(c);
								});
						}
					}
					else {
						fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
					}
				}
			}
		};

		std::vector<std::thread> threads;
		for (int i = 0; i < numThreads; i++) {
			threads.push_back(std::thread([&] {
				int r = event_loop_fn();
				assert(!r);
			}));
		}
		for (auto&& t : threads) {
			t.join();
		}
		close(listenFD);
		return 0;
	}

	template <int maxEvents = 10000, int maxFD = 1000000, typename H>
	int EpollListen(int const& port, SockTypes const& sockType, int const& numThreads, H&& peerHandler) {
		return EpollListenFD<maxEvents, maxFD>(MakeFD(port, sockType), numThreads, std::forward<H>(peerHandler));
	}
}
