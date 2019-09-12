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
#include <signal.h>
#include <sys/signalfd.h>
#include <netinet/tcp.h>

#include "xx_bbuffer.h"
#include "xx_queue.h"
#include "xx_epoll_buf.h"
#include "xx_epoll_buf_queue.h"

#include <atomic>

#include "xx_threadpool.h"

// todo: dialer, udp, 超时关 fd, timer 模拟, ip 获取, 异步域名解析
/*

dialer 参考
https://stackoverflow.com/questions/51777259/how-to-code-an-epoll-based-sockets-client-in-c

步骤：创建非阻塞 socket. 映射到 ctx, 存 connect时间. 之后各种判断



udp 单个 端口 大负载 面临的问题:
	从系统态 到 内存态 的瓶颈, pps 提升不上去
	很难并行处理( 包乱序问题 )

解决方案:
	创建多 fd 多 port, 绕开单个 fd 的流量
	一个 port 做 listener, 收到 client 握手信息之后返回 id + 其他 port 值, 之后由该 port & fd 负责处理该 client 的数据收发

需要做一个 port/fd 连接管理器模拟握手
理论上讲收到的数据可以丢线程池. 按 id 来限定处理线程 确保单个逻辑连接在单个线程中处理
udp 没有连接 / 断开 的说法，都要靠自己模拟, fd 也不容易失效.


*/

namespace xx {

	/*

	// 多线程共享 listen fd 示例. 建议线程数 3. 太多会令 epoll_wait 调用频繁令导致 ksoftirq 进程 100%
	// epoll_wait 每次拿的事件个数看上去越多越好，能有效降低 ksoftirq cpu 占用

	int listenPort = std::atoi(argv[1]);
	int numThreads = std::atoi(argv[2]);

	auto&& s = std::make_unique<xx::EchoServer>();
	int r = s->Listen(listenPort);
	assert(!r);

	xx::CoutN("thread:", 0);
	auto fd = s->listenFDs[0];
	std::vector<std::thread> threads;
	for (int i = 0; i < numThreads; ++i) {
		threads.emplace_back([fd, i] {
			auto&& s = std::make_unique<xx::EchoServer>();
			int r = s->ListenFD(fd);
			assert(!r);
			s->threadId = i + 1;
			xx::CoutN("thread:", i + 1);
			s->Run();

			}).detach();
	}
	s->Run();

	*/

	namespace Epoll {

		struct Instance;
		struct Peer {
			volatile uint32_t id = 0;								// 自增 id( 版本号 )
			Instance* ep = nullptr;									// 指向总容器 for 方便
			int sockFD = -1;										// 原始 fd
			int listenFD = -1;										// 原始 listenFD
			List<uint8_t> recv;										// 收数据用堆积容器
			BufQueue sendQueue;										// 待发送队列
			void* userData = nullptr;								// 可以随便存点啥

			Peer() = default;
			Peer(Peer const&) = delete;
			Peer& operator=(Peer const&) = delete;

			int Send(xx::EpollBuf&& eb);
			// todo: 主动掐线

			friend Instance;
		protected:
			inline void Clear(bool freeMemory = false);
			inline void Init(Instance* const& ep, int const& sockFD, int const& listenFD);
		};

		struct Peer_r {
			Peer* peer = nullptr;
			uint32_t id = 0;

			Peer_r(Peer& ctx)
				: peer(&ctx)
				, id(ctx.id) {
			}

			Peer_r() = delete;
			Peer_r(Peer_r const&) = default;
			Peer_r& operator=(Peer_r const&) = default;

			inline operator bool() {
				return peer->id == id;
			}
			Peer* operator->() {
				return peer;
			}
			Peer& operator*() {
				return *peer;
			}
		};

		struct Instance {

			// 各种配置

			// 如果一次性返回的 num events == maxNumEvents, 说明还有 events 没有被处理, 则继续 epoll_wait. 最多 maxLoopTimesPerFrame 次之后出 RunOnce
			static const int maxLoopTimesPerFrame = 5;

			// epool_wait 的返回值数量限制
			static const int maxNumEvents = 8192;

			// 支持的最大 fd 值 ( 应该大于等于 ulimit -n 的值 )
			static const int maxNumFD = 65536;

			// 支持的最大监听端口数量
			static const int maxNumListeners = 10;

			// 读缓冲区内存扩容增量
			static const int readBufReserveIncrease = 65536;

			// 每 fd 每一次可写, 写入的长度限制( 希望能实现当大量数据下发时各个 socket 公平的占用带宽 )
			static const int sendLenPerFrame = 65536;

			// writev 函数 (buf + len) 数组 参数允许的最大数组长度
			static const int maxNumIovecs = 1024;

			// 有连接进来
			virtual void OnAccept(Peer_r sctx, int const& listenIndex) {};

			// 有连接断开
			virtual void OnDisconnect(Peer_r sctx) {};

			// 有数据收到
			virtual int OnReceive(Peer_r sctx) = 0;

			// 线程逻辑编号
			int threadId = 0;

			// 所有 listener 的 fd 集合
			std::array<int, maxNumListeners> listenFDs;
			int listenFDsCount = 0;

		protected:
			// 用于生成唯一自增 id
			inline static std::atomic<uint32_t> id = 0;

			// epoll fd
			int efd = -1;

			// 用于 epoll 填充事件的容器
			std::array<epoll_event, maxNumEvents> events;

			// fd 读写上下文
			std::array<Peer, maxNumFD> ctxs;

			// 执行标志
			std::atomic<bool> running = true;

		public:
			Instance() {
				efd = epoll_create1(0);
				if (-1 == efd) throw - 1;
			}

			Instance(Instance const&) = delete;
			Instance& operator=(Instance const&) = delete;

			virtual ~Instance() {
				// todo: 各种 close
			}

			inline virtual void Run(int const& frameTimeoutMS = 100) {
				while (running) {
					if (RunOnce(frameTimeoutMS)) break;
				}
			}

			// 添加监听
			inline int Listen(int const& port) {
				if (listenFDsCount == maxNumListeners) return -1;
				auto&& fd = MakeListenFD(port);
				if (fd < 0) return -2;
				ScopeGuard sg([&] { close(fd); });
				if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -3;
				if (-1 == listen(fd, 2048/*SOMAXCONN*/)) return -4;
				if (-1 == Add(fd, EPOLLIN)) return -5;
				listenFDs[listenFDsCount++] = fd;
				sg.Cancel();
				return 0;
			}

			// 和其他 Instance 共享 FD
			inline int ListenFD(int const& fd) {
				if (-1 == Add(fd, EPOLLIN)) return -1;
				listenFDs[listenFDsCount++] = fd;
				return 0;
			}

			// todo: unlisten ?


			friend struct Peer;
		protected:

			inline int RunOnce(int const& frameTimeoutMS) {
				int counter = 0;
			LabBegin:

				int n = epoll_wait(efd, events.data(), maxNumEvents, frameTimeoutMS);
				if (n == -1) return errno;

				for (int i = 0; i < n; ++i) {
					// get fd
					auto fd = events[i].data.fd;
					auto ev = events[i].events;
					Peer& ctx = ctxs[fd];

					// error
					if (ev & EPOLLERR || ev & EPOLLHUP || !(ev & EPOLLIN)) goto LabClose;

					// check is listener: accept
					for (int idx = 0; idx < listenFDsCount; ++idx) {
						if (fd == listenFDs[idx]) {
							int sockFD = Accept(fd);
							if (sockFD >= 0) {
								auto&& ctx = ctxs[sockFD];
								ctx.Init(this, sockFD, fd);
								OnAccept(Peer_r(ctx), (int)idx);
							}
							goto LabContinue;
						}
					}

					// read
					if (Read(fd)) goto LabClose;
					if (OnReceive(Peer_r(ctx))) goto LabClose;
					if (Write(fd)) goto LabClose;

				LabContinue:
					continue;

					// fd cleanup
				LabClose:
					OnDisconnect(Peer_r(ctx));
					ctx.Clear(true);
					epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
					close(fd);
				}

				// limit handle times per frame
				if (n == maxNumEvents) {
					if (++counter < maxLoopTimesPerFrame) goto LabBegin;
				}

				return 0;
			}

			// return !0: error
			inline int Add(int const& fd, uint32_t const& flags) {
				epoll_event event;
				event.data.fd = fd;
				event.events = flags | EPOLLET;
				return epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
			};

			// return fd. <0: error
			inline int Accept(int const& listenFD) {
				sockaddr in_addr;									// todo: ipv6 support
				socklen_t inLen = sizeof(in_addr);
				int fd = accept(listenFD, &in_addr, &inLen);
				if (-1 == fd) return -1;
				ScopeGuard sg([&] { close(fd); });
				if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
				int on = 1;
				if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)& on, sizeof(on))) return -3;
				if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, (const char*)& on, sizeof(on))) return -4;
				//if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_FASTOPEN, (const char*)& on, sizeof(on))) return -5;
				if (-1 == Add(fd, EPOLLIN/* | EPOLLOUT*/)) return -6;
				sg.Cancel();
				return fd;
			}

			// return !0: error
			inline int Write(int const& fd) {
				Peer& ctx = ctxs[fd];
				while (ctx.sendQueue.bytes) {
					std::array<iovec, maxNumIovecs> vs;					// buf + len 数组指针
					int vsLen = 0;										// 数组长度
					size_t bufLen = sendLenPerFrame;					// 计划发送字节数

					// 填充 vs, vsLen, bufLen 并返回预期 offset
					auto&& offset = ctx.sendQueue.Fill(vs, vsLen, bufLen);

					// 返回值为 实际发出的字节数
					auto&& sentLen = writev(fd, vs.data(), vsLen);

					if (!sentLen) return -1;
					else if (sentLen == -1) {
						if (errno == EAGAIN) return 0;
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

			// return !0: error
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

			// return fd. <0: error
			inline static int MakeListenFD(int const& port) {
				char portStr[20];
				snprintf(portStr, sizeof(portStr), "%d", port);

				addrinfo hints;														// todo: ipv6 support
				memset(&hints, 0, sizeof(addrinfo));
				hints.ai_family = AF_UNSPEC;										// ipv4 / 6
				hints.ai_socktype = SOCK_STREAM;									// SOCK_STREAM / SOCK_DGRAM
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

			inline static int MakeTcpSocketFD(char const* const& ip, char const* const& port) {

			}
		};


		inline void Peer::Clear(bool freeMemory) {
			id = 0;
			sockFD = -1;
			listenFD = -1;
			recv.Clear(freeMemory);
			sendQueue.Clear(freeMemory);
		}

		inline void Peer::Init(Instance* const& ep, int const& sockFD, int const& listenFD) {
			this->id = ++ep->id;
			this->ep = ep;
			this->sockFD = sockFD;
			this->listenFD = listenFD;
			this->recv.Clear();
			this->sendQueue.Clear();
		}

		inline int Peer::Send(xx::EpollBuf&& eb) {
			assert(eb.len);
			sendQueue.Push(std::move(eb));
			return ep->Write(sockFD);
		}
	}

}
