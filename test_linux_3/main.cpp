#include "xx_uv.h"
struct Foo {
	int lineNumber = 0;
	xx::Uv uv;
	xx::UvDialer_s dialer;
	xx::UvPeer_s peer;
	xx::BBuffer_s bb;
	bool callbacked = false;
	int r = 0;
	int count = 0;
	int64_t beginTime = 0;

	Foo() {
		xx::MakeTo(dialer, uv);
		dialer->onAccept = [&](xx::UvPeer_s p) {
			if (p) {
				peer = std::move(p);
				peer->onReceivePush = [&](xx::Object_s&& msg)->int {
					//xx::CoutN("recv push ", msg);
					return 0;
				};
			}
		};
		xx::MakeTo(bb);
		beginTime = xx::NowSteadyEpochMS();
	}

	void Reset() {
		peer.reset();
		callbacked = false;
	}

	int Update() {
		COR_BEGIN

			LabDial :
		COR_YIELD
			xx::CoutN("dial...");

		Reset();
		dialer->Dial("127.0.0.1", 12345);
		while (dialer->Busy()) {
			COR_YIELD
		}
		if (!peer || peer->Disposed()) goto LabDial;

		callbacked = false;
		r = peer->SendRequest(bb, [this](xx::Object_s&& msg)->int {
			xx::CoutN("recv request ", msg);
			if (!msg) {
				throw - 1;
			}
			callbacked = true;
			return 0;
			}, 0);
		assert(!r);

		while (!callbacked) {
			COR_YIELD
		}
		xx::CoutN("end...");

		if (++count > 10000) {
			xx::CoutN(xx::NowSteadyEpochMS() - beginTime);
			return 0;
		}
		goto LabDial;

		COR_END
	}
};

int main() {
	xx::IgnoreSignal();
	Foo f;
	while (true) {
		f.uv.Run(UV_RUN_NOWAIT);
		f.lineNumber = f.Update();
		if (!f.lineNumber) break;
		//usleep(100000);
	}
	return 0;
}










































//#include "xx_epoll.h"
//
//int main() {
//	int pipeFDs[2];
//	auto&& r = pipe(pipeFDs);
//	assert(r >= 0);
//
//	std::thread t([&] {
//		while (true) {
//			auto&& ticks = xx::NowEpoch10m();
//			auto&& r = write(pipeFDs[1], &ticks, sizeof(ticks));
//			assert(r == sizeof(ticks));
//			usleep(10000);
//		}
//	});
//
//	auto&& efd = epoll_create1(0);
//	if (-1 == efd) throw - 1;
//
//	epoll_event event;
//	event.data.fd = pipeFDs[0];
//	event.events = EPOLLIN;
//	r = epoll_ctl(efd, EPOLL_CTL_ADD, pipeFDs[0], &event);
//	assert(r == 0);
//
//	std::array<epoll_event, 512> events;
//	while (true) {
//		int n = epoll_wait(efd, events.data(), events.size(), -1);
//		if (n == -1) return errno;
//		for (int i = 0; i < n; ++i) {
//			auto fd = events[i].data.fd;
//			auto ev = events[i].events;
//
//			// error
//			if (ev & EPOLLERR || ev & EPOLLHUP) {
//				epoll_ctl(efd, EPOLL_CTL_DEL, fd, nullptr);
//				continue;
//			}
//
//			// read
//			if (ev & EPOLLIN) {
//				if (fd == pipeFDs[0]) {
//					int64_t ticks;
//					auto&& len = read(fd, &ticks, sizeof(ticks));
//					if (len <= 0) return -1;
//					xx::CoutN(xx::NowEpoch10m() - ticks);
//				}
//			}
//		}
//	}
//
//	//sleep(3);
//	//write(fd[1], ...)
//	//read(fd[0], readbuf, sizeof(readbuf));
//
//	return 0;
//}
