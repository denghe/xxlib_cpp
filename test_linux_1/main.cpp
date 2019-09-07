#include "xx_epoll.h"
#include <signal.h>

namespace xx {
	struct EchoServer : Epoll<1000> {
		int threadId = 0;

		inline virtual void OnAccept(SockContext& sctx, int const& listenIndex) {
			xx::CoutN(threadId, " OnAccept: listenIndex = ", listenIndex, ", id = ", sctx.id);
		}

		virtual void OnDisconnect(SockContext& sctx) {
			xx::CoutN(threadId, "OnDisconnect: id = ", sctx.id);
		}

		// echo server
		xx::BBuffer tmpBB;
		virtual int OnReceive(SockContext& sctx) {
			tmpBB.AddRange(sctx.recv);
			sctx.recv.Clear();
			return SendTo(sctx, xx::EpollBuf(tmpBB));
		}
	};
}

int main() {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;//设定接受到指定信号后的动作为忽略
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask) == -1 || //初始化信号集为空
		sigaction(SIGPIPE, &sa, 0) == -1) { //屏蔽SIGPIPE信号
		perror("failed to ignore SIGPIPE; sigaction");
		exit(EXIT_FAILURE);
	}

	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		printf("block sigpipe error\n");
	}

	auto&& s = std::make_unique<xx::EchoServer>();
	int r = s->Listen(12345);
	assert(!r);
	auto fd = s->listenFDs[0];

	std::vector<std::thread> threads;
	for (int i = 0; i < 4; ++i) {
		threads.emplace_back([fd, i] {
			auto&& s = std::make_unique<xx::EchoServer>();
			int r = s->ListenFD(fd);
			assert(!r);
			s->threadId = i + 1;
			xx::CoutN("thread:", i);
			while (true) {
				s->RunOnce();
			}

		}).detach();
	}
	while (true) {
		r = s->RunOnce();
	}
	return 0;
}


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
