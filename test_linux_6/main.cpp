#include "xx_epoll2.hpp"
namespace EP = xx::Epoll;

struct L : EP::TcpListener {
	inline virtual int OnAccept(std::shared_ptr<EP::TcpPeer>& peer) override {
		xx::CoutN("ip: ", peer->ip, " accepted.");
		return 0;
		// 设置 3 秒后自动断开
		//return peer->SetTimeout(30);
	}
};

int main() {
	xx::IgnoreSignal();
	EP::Context ep;
	auto listener = ep.TcpListen<L>(12345);
	ep.Delay(10, [&](auto t) {
		xx::Cout(".");
		t->SetTimeout(10);
	});
	return ep.Run(10);
}
