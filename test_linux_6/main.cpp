#include "xx_epoll2.hpp"

struct L : xx::Epoll::TcpListener {
	inline virtual int OnAccept(std::shared_ptr<xx::Epoll::TcpPeer>& peer) override {
		// 设置 3 秒后自动断开
		return peer->SetTimeout(30);
	}
};

struct C : xx::Epoll::TcpConn {
	inline virtual void OnConnect() override {
		xx::CoutN("connected = ", connected);
	}
};

int main() {
	xx::IgnoreSignal();
	xx::Epoll::Context ep;
	ep.Delay(10, [&](xx::Epoll::Timer* const& timer) {
		xx::CoutN("111");
		ep.TcpDial<C>("127.0.0.1", 12345, 20);
		xx::CoutN("222");
		});
	auto listener = ep.TcpListen<L>(12345);
	return ep.Run(10);
}
