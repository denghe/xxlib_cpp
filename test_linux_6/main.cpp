#include "xx_epoll2.hpp"
namespace EP = xx::Epoll;

std::size_t counter = 0;

struct P : EP::TcpPeer {
	inline virtual void Init() override {
		xx::CoutN("ip: ", ip, " accepted.");
	}

	inline virtual int OnReceive() override {
		++counter;
		return this->TcpPeer::OnReceive();
	}

};

struct L : EP::TcpListener {
	inline virtual std::shared_ptr<EP::TcpPeer> OnCreatePeer() override {
		return xx::Make<P>();
	}
};

int main() {
	xx::IgnoreSignal();
	EP::Context ep;
	auto listener = ep.TcpListen<L>(12345);
	ep.Delay(10, [&](auto t) {
		xx::CoutN("counter = ", counter);
		counter = 0;
		t->SetTimeout(10);
	});
	return ep.Run(10);
}
