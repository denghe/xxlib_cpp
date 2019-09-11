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
#include <omp.h>

#include "xx_threadpool.h"

// todo: dialer, udp
/*
udp
面临的问题:
	由单个 端口 负责收发所有 client 数据, 将面临以下问题:
		从系统态 到 内存态 的读取瓶颈, pps 提升不上去
		, 多线程操作单个句柄自旋争用大无法提速
		， 多进程数据无法分流等问题

解决方案:


一个 port 做 listener, 收到 client 握手信息之后返回 id + 其他 port 值, 之后由该 port 负责处理该 client 的数据收发

设计要点：多 fd 多 port, 绕开单个 fd 的流量

*/

namespace xx {

	struct Epoll;
	struct SockContext {
		Epoll* epoll = nullptr;									// 指向总容器 for 方便
		uint32_t id = 0;										// 自增 id( 版本号 )
		int sockFD = -1;										// 原始 fd
		int listenFD = -1;										// 原始 listenFD
		List<uint8_t> recv;										// 收数据用堆积容器
		BufQueue sendQueue;										// 待发送队列

		int Send(xx::EpollBuf&& eb);
		// todo: 主动掐线

		friend Epoll;
	protected:
		inline void Clear(bool freeMemory = false);
	};

	struct SockContext_r {
		SockContext& ctx;
		uint32_t id;
		SockContext_r(SockContext& ctx)
			: ctx(ctx)
			, id(ctx.id) {
		}
		SockContext_r(SockContext_r const&) = default;
		SockContext_r& operator=(SockContext_r const&) = default;
		inline operator bool() {
			return ctx.id == id;
		}
		SockContext* operator->() {
			return &ctx;
		}
		SockContext& operator*() {
			return ctx;
		}
	};

	enum class EpollSockTypes : int {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	struct Epoll {

		// 各种配置

		// 如果一次性返回的 num events == maxNumEvents, 说明还有 events 没有被处理, 则继续 epoll_wait. 最多 maxLoopTimesPerFrame 次之后出 RunOnce
		static const int maxLoopTimesPerFrame = 48;

		// epool_wait 的返回值数量限制
		static const int maxNumEvents = 1000;

		// 支持的最大 fd 值 ( 应该大于等于 ulimit -n 的值 )
		static const int maxNumFD = 100000;

		// 支持的最大监听端口数量
		static const int maxNumListeners = 10;

		// 读缓冲区内存扩容增量
		static const int readBufReserveIncrease = 65536;

		// 每 fd 每一次可写, 写入的长度限制( 希望能实现当大量数据下发时各个 socket 公平的占用带宽 )
		static const int sendLenPerFrame = 65536;

		// writev 函数 (buf + len) 数组 参数允许的最大数组长度
		static const int maxNumIovecs = 1024;

		// 有连接进来
		virtual void OnAccept(int const& threadId, SockContext_r sctx, int const& listenIndex) = 0;

		// 有连接断开
		virtual void OnDisconnect(int const& threadId, SockContext_r sctx) = 0;

		// 有数据收到
		virtual int OnReceive(int const& threadId, SockContext_r sctx) = 0;

	protected:
		// 用于生成唯一自增 id
		std::atomic<uint32_t> id;

		// epoll fd
		int efd = -1;

		// 所有 listener 的 fd 集合
		std::array<int, maxNumListeners> listenFDs;
		int listenFDsCount = 0;

		// 用于 epoll 填充事件的容器
		std::array<epoll_event, maxNumEvents> events;

		// fd 读写上下文
		std::array<SockContext, maxNumFD> ctxs;

		// 执行线程
		std::vector<std::thread> threads;

		// 执行标志
		std::atomic<bool> running = true;

	public:
		Epoll() {
			efd = epoll_create1(0);
			if (-1 == efd) throw - 1;
		}

		Epoll(Epoll const&) = delete;
		Epoll& operator=(Epoll const&) = delete;

		virtual ~Epoll() {
			// todo: 各种 close
		}

		// numMoreThreads: 附加进程数. 0 则只有主线程 run
		inline void Run(int const& numMoreThreads = 0, int const& frameTimeoutMS = 100) {
			for (int i = 0; i < numMoreThreads; ++i) {
				threads.emplace_back([this, threadId = i + 1] {
					while (running) {
						if (RunOnce(threadId, -1)) break;
					}
					Run(threadId);
				});
			}
			while (running) {
				if (RunOnce(0, -1)) break;
			}
			for (auto&& t : threads) {
				t.join();
			}
			threads.clear();
		}

		// 添加监听
		inline int Listen(int const& port, EpollSockTypes const& sockType = EpollSockTypes::TCP) {
			if (listenFDsCount == maxNumListeners) return -1;
			auto&& fd = MakeListenFD(port, sockType);
			if (fd < 0) return -2;
			ScopeGuard sg([&] { close(fd); });
			if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -3;
			if (-1 == listen(fd, SOMAXCONN)) return -4;
			if (-1 == Add(fd, EPOLLIN)) return -5;
			listenFDs[listenFDsCount++] = fd;
			sg.Cancel();
			return 0;
		}

		// todo: unlisten ?


		friend struct SockContext;
	protected:

		// threadId 可用于 RunOnce 之后的逻辑判断, 0 即主线程
		inline virtual int RunOnce(int const& threadId, int const& frameTimeoutMS) {
			int counter = 0;
		LabBegin:

			int n = epoll_wait(efd, events.data(), maxNumEvents, frameTimeoutMS);
			if (n == -1) return errno;

			{
				xx::ThreadPool tp;
				//#pragma omp parallel for num_threads(5)
				for (int i = 0; i < n; ++i) {

					tp.Add([&, i] {

						// get fd
						auto fd = events[i].data.fd;
						auto ev = events[i].events;
						SockContext& ctx = ctxs[fd];

						// error
						if (ev & EPOLLERR || ev & EPOLLHUP || !(ev & EPOLLIN)) goto LabClose;

						// check is listener: accept
						for (int idx = 0; idx < listenFDsCount; ++idx) {
							if (fd == listenFDs[idx]) {
								int sockFD = Accept(fd);
								if (sockFD >= 0) {
									OnAccept(threadId, SockContext_r(ctxs[sockFD]), (int)idx);
								}
								goto LabContinue;
							}
						}

						// read
						if (Read(fd)) goto LabClose;
						if (OnReceive(threadId, SockContext_r(ctx))) goto LabClose;
						if (Write(fd)) goto LabClose;

					LabContinue:
						//Mod(fd, &events[i]);
						//continue;
						return;

						// fd cleanup
					LabClose:
						OnDisconnect(threadId, SockContext_r(ctx));
						ctx.Clear(true);
						close(fd);
						});
				}

			}

			// limit handle times per frame
			if (n == maxNumEvents) {
				if (++counter < maxLoopTimesPerFrame) goto LabBegin;
			}

			return 0;
		}

		inline int Add(int const& fd, uint32_t const& flags) {
			epoll_event event;
			event.data.fd = fd;
			event.events = flags | EPOLLET;
			auto r = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event);
			assert(r == 0);
			return r;
		};

		//inline int Mod(int const& fd, epoll_event* const& e) {
		//	auto r = epoll_ctl(efd, EPOLL_CTL_MOD, fd, e);
		//	assert(r == 0);
		//	return r;
		//}

		// return value < 0: error
		inline int Accept(int const& listenFD) {
			sockaddr in_addr;									// todo: ipv6 support
			socklen_t inLen = sizeof(in_addr);
			int fd = accept(listenFD, &in_addr, &inLen);
			if (-1 == fd) return -1;
			ScopeGuard sg([&] { close(fd); });
			if (-1 == fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK)) return -2;
			int on = 1;
			//if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)& on, sizeof(on))) return -3;
			//if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, (const char*)& on, sizeof(on))) return -4;
			//if (-1 == setsockopt(fd, IPPROTO_TCP, TCP_FASTOPEN, (const char*)& on, sizeof(on))) return -5;
			if (-1 == Add(fd, EPOLLIN/* | EPOLLOUT*/)) return -6;
			auto&& ctx = ctxs[fd];
			ctx.epoll = this;
			ctx.id = ++id;
			ctx.sockFD = fd;
			ctx.listenFD = listenFD;
			sg.Cancel();
			return fd;
		}

		// 向 sockFD 发送 sendLenPerFrame 字节数据. 
		inline int Write(int const& fd) {
			SockContext& ctx = ctxs[fd];
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
	};


	inline void SockContext::Clear(bool freeMemory) {
		id = 0;
		sockFD = -1;
		listenFD = -1;
		recv.Clear(freeMemory);
		sendQueue.Clear(freeMemory);
	}

	inline int SockContext::Send(xx::EpollBuf&& eb) {
		assert(eb.len);
		sendQueue.Push(std::move(eb));
		return epoll->Write(sockFD);
	}
}
