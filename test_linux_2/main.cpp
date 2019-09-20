#include "xx_epoll.h"

struct Server : xx::Epoll::Instance {
	Server() {
		coros.Add([this](xx::Coro& yield) {
			while (true) {
				xx::Cout(".");
				yield();
			}
		});
	}

	inline virtual void OnAccept(xx::Epoll::Peer_r pr, int const& listenIndex) override {
		assert(listenIndex >= 0);
		xx::CoutN(threadId, " OnAccept: listenIndex = ", listenIndex, ", id = ", pr->id, ", fd = ", pr->sockFD);
	}

	inline virtual void OnDisconnect(xx::Epoll::Peer_r pr) override {
		xx::CoutN(threadId, " OnDisconnect: id = ", pr->id);
	}

	virtual int OnReceive(xx::Epoll::Peer_r pr) override {
		return pr->Send(xx::Epoll::Buf(pr->recv));		// echo
	}

};

int main(int argc, char* argv[]) {
	auto&& s = std::make_unique<Server>();
	int r = s->Listen(11111);
	assert(!r);
	return s->Run(1);
}
