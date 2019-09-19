#include "xx_epoll.h"
//#include "xx_coros_boost.h"


/*

协程如果只有 yield() 可用，是比较粗糙的。会在每帧无脑被唤醒，进而执行自己的状态检查，虽然也能完成需求，但是会空耗 cpu, 且处理及时度降低很多.

考虑参考 golang 的 select 部分, yield 变为条件式.

协程事件类型分类：
1. 网络: 连，断，收 ( 理论上讲还可以有个 发送队列已清空 )
	根据 fd 进一步路由到
2. 帧步进



*/

// todo: dialer, udp, 超时关 fd, timer 模拟, ip 获取, 异步域名解析

// todo: EPOLLOUT 监视恢复. write 写入不能后靠可写监视来恢复. EPOLLIN 的 if 还是加上. 后面可能和 disposing 标志位同时判定

// todo: Dispatch

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

namespace {
	using namespace xx::Epoll;
	struct EchoServer : Instance {
		inline virtual void OnAccept(Peer_r pr, int const& listenIndex) override {
			xx::CoutN(threadId, " OnAccept: listenIndex = ", listenIndex, ", id = ", pr->id, ", fd = ", pr->sockFD);
			pr->SetTimeout(100);
		}

		inline virtual void OnDisconnect(Peer_r pr) override {
			xx::CoutN(threadId, " OnDisconnect: id = ", pr->id);
		}

		//virtual int OnReceive(Peer_r pr) override {
		//	return pr->Send(Buf(pr->recv));		// echo
		//}

		inline virtual int Update(int64_t frameNumber) override {
			xx::Cout(".");
			//Sleep((double)rand() / RAND_MAX * 100 + 50);		// 模拟长时间运行
			return 0;
		}
	};
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		xx::CoutN("need args: listenPort numThreads");
		return -1;
	}
	int listenPort = std::atoi(argv[1]);
	int numThreads = std::atoi(argv[2]);

	auto&& s = std::make_unique<EchoServer>();
	int r = s->Listen(listenPort);
	assert(!r);

	xx::CoutN("thread:", 0);
	std::vector<std::thread> threads;
	auto fd = s->listenFDs[0];
	for (int i = 0; i < numThreads; ++i) {
		threads.emplace_back([fd, i] {
			auto&& s = std::make_unique<EchoServer>();
			int r = s->ListenFD(fd);
			assert(!r);
			s->threadId = i + 1;
			xx::CoutN("thread:", i + 1);
			s->Run(10);
			}).detach();
	}

	s->Run(10);

	return 0;
}





//struct sigaction sa;
//sa.sa_handler = SIG_IGN;//设定接受到指定信号后的动作为忽略
//sa.sa_flags = 0;
//if (sigemptyset(&sa.sa_mask) == -1 || //初始化信号集为空
//	sigaction(SIGPIPE, &sa, 0) == -1) { //屏蔽SIGPIPE信号
//	perror("failed to ignore SIGPIPE; sigaction");
//	exit(EXIT_FAILURE);
//}

//signal(SIGPIPE, SIG_IGN);

//sigset_t signal_mask;
//sigemptyset(&signal_mask);
//sigaddset(&signal_mask, SIGPIPE);
//int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
//if (rc != 0) {
//	printf("block sigpipe error\n");
//}

//rc = sigprocmask(SIG_BLOCK, &signal_mask, NULL);
//if (rc != 0) {
//	printf("block sigpipe error\n");
//}



//uint64_t counter = 0;
//if (r) {
//	xx::Cout(r);
//}
//xx::Cout(".");
//++counter;
//if (counter % 100 == 0) {
//	xx::CoutN();
//}

//std::vector<EpollMessage> msgs;
					//auto&& m = msgs.emplace_back();
				//m.fd = events[i].data.fd;
				//m.type = EpollMessageTypes::Accept;
					//auto&& m = msgs.emplace_back();
				//m.fd = events[i].data.fd;
				//m.type = EpollMessageTypes::Disconnect;


//enum class EpollMessageTypes {
//	Unknown,
//	Accept,
//	Disconnect,
//	Read
//};

//struct EpollMessage {
//	EpollMessageTypes type = EpollMessageTypes::Unknown;
//	int fd = 0;
//	EpollBuf buf;

//	EpollMessage() = default;
//	EpollMessage(EpollMessage const&) = delete;
//	EpollMessage& operator=(EpollMessage const&) = delete;
//	EpollMessage(EpollMessage&&) = default;
//	EpollMessage& operator=(EpollMessage&&) = default;
//};

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
//	xx::EpollListen(12345, xx::SockTypes::TCP, 4, [](int fd, auto read, auto write) {
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
