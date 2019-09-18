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
#include <optional>

#include "xx_threadpool.h"

// todo: dialer, udp, 超时关 fd, timer 模拟, ip 获取, 异步域名解析

// todo: EPOLLOUT 监视恢复. write 写入不能后靠可写监视来恢复. EPOLLIN 的 if 还是加上. 后面可能和 disposing 标志位同时判定

/*

dialer 参考
https://stackoverflow.com/questions/51777259/how-to-code-an-epoll-based-sockets-client-in-c

步骤：创建非阻塞 socket. 映射到 ctx, 存 connect时间. 之后各种判断

设计思路：
	在现有上下文中增加 target address 信息？默认为 0.
	EPOLLIN 事件时检测这个值，如果非 0, 说明是拨号连接成功的。成功后清除该 addr
	同时有超时检测请求, 考虑做到 epoll 每次 timeout 之后.
	timer 先用时间轮方式实现, 即 设定最大超时时长，基于每秒执行的 epoll_wait 次数，可得到数组长度。
	每数组元素只需要存起始上下文下标. 上下文元素中存储 prev, next 的下标，构成双向链表结构

	初期先实现 accept 产生的 peer 的超时管理



延迟掐线实现办法：
	基于 timer. 超时发生时，打 disposing 标志位，从而跳过 recv 处理？




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

namespace xx::Epoll {

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

	struct Instance;
	struct Peer {
		friend Instance;

		volatile uint64_t id = 0;								// 自增 id( 版本号 )
		int sockFD = -1;										// 原始 socket fd
		int listenFD = -1;										// 从哪个 listen socket fd accept 来的

		Instance* ep = nullptr;									// 指向总容器 for 方便
		void* userData = nullptr;								// 可以随便存点啥
		int userInt = 0;										// 可以随便存点啥

	protected:
		int timeoutIndex = -1;									// 位于 timeoutWheel 的下标. 如果该 peer 为 head 则该值非 -1 则可借助该值定位到 wheel 中的位置
		Peer* timeoutPrev = nullptr;							// 位于相同刻度时间队列中时, prev + next 组成双向链表
		Peer* timeoutNext = nullptr;							// 位于相同刻度时间队列中时, prev + next 组成双向链表

	public:
		List<uint8_t> recv;										// 收数据用堆积容器
		std::optional<BufQueue> sendQueue;						// 待发送队列

		int Send(Buf&& eb);										// 发送
		void Dispose();											// 掐线销毁
		bool Disposed();										// 返回是否已销毁

		Peer() = default;
		int SetTimeout(int const& interval);					// 设置或停止超时管理( interval 传 0 表示停止 ). 返回 0 表示设置成功. -1 表示超出了 wheel 界限. interval 指 RunOnce 次数

	protected:
		void Init(Instance* const& ep, int const& sockFD, int const& listenFD);

		Peer(Peer const&) = delete;
		Peer& operator=(Peer const&) = delete;
	};

	struct Peer_r {
		Peer* peer = nullptr;
		uint64_t id = 0;

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

		// todo: more func forward for easy use
	};

	// epoll 实体封装类
	struct Instance {
		// 各种配置
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

		// 超时时间轮长度。需要 2^n 对齐以实现快速取余. 按照典型的 60 帧逻辑，60 秒需要 3600
		static const int timeoutWheelLen = 1 << 12;

		// 线程逻辑编号
		int threadId = 0;

		// 所有 listener 的 fd 集合
		std::array<int, maxNumListeners> listenFDs;
		int listenFDsCount = 0;


	protected:
		// 用于生成唯一自增 id
		inline static std::atomic<uint64_t> id = 0;

		// epoll fd
		int efd = -1;

		// 用于 epoll 填充事件的容器
		std::array<epoll_event, maxNumEvents> events;

		// fd 读写上下文
		std::array<Peer, maxNumFD> ctxs;

		// 时间轮. 填入参与 timeout 检测的 peer 的下标( 链表头 )
		std::array<Peer*, timeoutWheelLen> timeoutWheel;

		// 时间轮游标. 指向当前链表. 每帧 +1, 按 timeoutWheelLen 长度取余, 循环使用
		int timeoutWheelCursor = 0;

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

		// 有连接进来
		inline virtual void OnAccept(Peer_r pr, int const& listenIndex) {};

		// 有连接断开
		inline virtual void OnDisconnect(Peer_r pr) {};

		// 有数据收到( 默认实现 echo 效果 )
		inline virtual int OnReceive(Peer_r pr) {
			return pr->Send(Buf(pr->recv));
		}

		// 帧逻辑可以放在这. 返回非 0 将令 Run 退出
		inline virtual int Update(int64_t frameNumber) { return 0; }

		
		// 开始运行. 期间会以固定帧率调用 Update(). waitMS 为 epoll_wait 的等待时长，需要自己算，不能太频繁调用 wait, 也不能侵占太多业务逻辑执行时长
		inline int Run(double const& frameRate = 60.3, int const& waitMS = 10) {
			assert(frameRate > 0);
			// 计算帧时间间隔
			auto ticksPerFrame = 10000000.0 / frameRate;

			// 稳定帧回调用的时间池
			double ticksPool = 0;
			int64_t frameNumber = 0;

			// 取当前时间
			auto lastTicks = xx::NowEpoch10m();

			// 开始循环
			while (running) {
				// 调用一次 epoll wait. 等待时间约为帧间隔时长.( 理论上讲应该设置的更小 )
				if (int r = Wait(waitMS)) return r;

				// 计算上个循环到现在经历的时长, 并累加到 pool
				auto currTicks = xx::NowEpoch10m();
				ticksPool += (double)(currTicks - lastTicks);
				lastTicks = currTicks;

				// 消耗累计时长, 前进 frame 并调用 Update
				while (ticksPool > ticksPerFrame) {
					HandleTimeout();
					++frameNumber;
					if (int r = Update(frameNumber)) return r;
					ticksPool -= ticksPerFrame;
				}
			}
			return 0;
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

		// todo: Unlisten ?

		// todo: Dial?


		friend struct Peer;
	protected:

		inline void HandleTimeout() {
			// 超时管理。对超时 peer 进行 Dispose 操作
			// 循环递增游标
			timeoutWheelCursor = (timeoutWheelCursor + 1) & (timeoutWheelLen - 1);
			auto p = timeoutWheel[timeoutWheelCursor];
			while (p) {
				auto np = p->timeoutNext;
				p->Dispose();
				p = np;
			};
		}

		// 进入一次 epoll wait. 可传入超时时间. 
		inline int Wait(int const& timeoutMS) {
			int n = epoll_wait(efd, events.data(), maxNumEvents, timeoutMS);
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
				ctx.Dispose();
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
			if (ctxs[fd].Disposed()) return -1;
			auto&& q = ctxs[fd].sendQueue.value();
			while (q.bytes) {
				std::array<iovec, maxNumIovecs> vs;					// buf + len 数组指针
				int vsLen = 0;										// 数组长度
				size_t bufLen = sendLenPerFrame;					// 计划发送字节数

				// 填充 vs, vsLen, bufLen 并返回预期 offset
				auto&& offset = q.Fill(vs, vsLen, bufLen);

				// 返回值为 实际发出的字节数
				auto&& sentLen = writev(fd, vs.data(), vsLen);

				if (!sentLen) return -2;
				else if (sentLen == -1) {
					if (errno == EAGAIN) return 0;
					return -3;
				}
				else if ((size_t)sentLen == bufLen) {
					q.Pop(vsLen, offset, bufLen);
				}
				else {
					q.Pop(sentLen);
					return 0;										// 理论上讲如果只写入成功一部分, 不必 retry 了。这点需要验证
				}
			}
			return 0;
		}

		// return !0: error
		inline int Read(int const& fd) {
			auto&& buf = ctxs[fd].recv;
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

		//inline static int MakeTcpSocketFD(char const* const& ip, char const* const& port) {
		//   return
		//}
	};


	inline int Peer::SetTimeout(int const& interval) {
		// 试着从 wheel 链表中移除
		if (timeoutIndex != -1) {
			if (timeoutNext != nullptr) {
				timeoutNext->timeoutPrev = timeoutPrev;
			}
			if (timeoutPrev != nullptr) {
				timeoutPrev->timeoutNext = timeoutNext;
			}
			else {
				ep->timeoutWheel[timeoutIndex] = timeoutNext;
			}
		}

		// 检查是否传入间隔时间
		if (interval) {
			// 如果设置了新的超时时间, 则放入相应的链表
			// 安全检查
			if (interval < 0 || interval > ep->timeoutWheelLen) return -1;

			// 环形定位到 wheel 元素目标链表下标
			timeoutIndex = (interval + ep->timeoutWheelCursor) & (ep->timeoutWheelLen - 1);

			// 成为链表头
			timeoutPrev = nullptr;
			timeoutNext = ep->timeoutWheel[timeoutIndex];
			ep->timeoutWheel[timeoutIndex] = this;

			// 和之前的链表头连起来( 如果有的话 )
			if (timeoutNext) {
				timeoutNext->timeoutPrev = this;
			}
		}
		else {
			// 重置到初始状态
			timeoutPrev = nullptr;
			timeoutNext = nullptr;
			timeoutIndex = -1;
		}
		return 0;
	}

	inline void Peer::Init(Instance* const& ep, int const& sockFD, int const& listenFD) {
		assert(!this->id);
		this->id = ++ep->id;
		this->ep = ep;
		this->sockFD = sockFD;
		this->listenFD = listenFD;
		this->recv.Clear();
		this->sendQueue.emplace();
	}

	inline bool Peer::Disposed() {
		return id == 0;
	}

	inline void Peer::Dispose() {
		assert(this->ep);
		if (this->id == 0) return;

		ep->OnDisconnect(Peer_r(*this));
		auto fd = sockFD;

		this->id = 0;
		this->sockFD = -1;
		this->listenFD = -1;
		this->recv.Clear(true);
		this->sendQueue.reset();
		this->SetTimeout(0);

		epoll_ctl(ep->efd, EPOLL_CTL_DEL, fd, nullptr);
		close(fd);
	}

	inline int Peer::Send(Buf&& eb) {
		assert(this->id);
		sendQueue.value().Push(std::move(eb));
		return ep->Write(sockFD);
	}
}
