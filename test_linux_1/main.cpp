#include "xx_epoll.h"

namespace xx {
	struct EchoServer : Epoll<1000> {
		xx::BBuffer tmpBB;
		EchoServer() {
		}
		~EchoServer() {
			tmpBB.Reset();
		}

		inline virtual void OnAccept(EpollFDContext& ctx, int const& listenFD, int const& sockFD) {
			xx::CoutN("OnAccept: listen fd = ", listenFD, ", sock fd = ", sockFD);
		}
		virtual void OnDisconnect(EpollFDContext& ctx, int const& sockFD) {
			xx::CoutN("OnDisconnect: sock fd = ", sockFD);
		}
		virtual void OnReceive(EpollFDContext& ctx, int const& sockFD) {
			//xx::CoutN("OnReceive: sock fd =", sockFD, ", recv = ", ctx.recv);

			// echo server
			tmpBB.Reset();
			tmpBB.AddRange(ctx.recv.buf, ctx.recv.len);
			ctx.recv.Clear();
			ctx.sendQueue.Push(xx::EpollBuf(tmpBB));
		}
	};
}

int main() {
	auto&& s = std::make_unique<xx::EchoServer>();
	int r = s->Listen(12345);
	assert(!r);
	r = s->Listen(23456);
	assert(!r);
	uint64_t counter = 0;
	while (true) {
		r = s->RunOnce();
		if (r) {
			xx::Cout(r);
		}
		xx::Cout(".");
		++counter;
		if (counter % 100 == 0) {
			xx::CoutN();
		}
	}

	return 0;
}



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
