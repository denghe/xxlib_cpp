#include "xx_epoll2.hpp"
namespace EP = xx::Epoll;

struct KP : EP::KcpPeer {
	bool serverSide = false;
	inline virtual void OnReceive() override {
		xx::CoutN("serverSide = ", serverSide);
		if (serverSide) {
			xx::CoutN("server kcp recv = ", recv.len);
			// 忽略握手包
			if (*recv.buf == 1 && recv.len == 5) {
				recv.Clear();
				xx::CoutN("recv hand shake package.");
				Send(".", 100);
				Flush();
			}
			else {
				if (Send(recv.buf, recv.len)) {
					OnDisconnect(-3);
					Dispose();
				}
				else {
					recv.Clear();
					Flush();
				}
			}
		}
		else {
			xx::CoutN("client kcp recv = ", recv.len);
		}
	}
	inline virtual void OnDisconnect(int const& reason = 0) override {
		xx::CoutN("kcp ip: ", addr, " disconnected. reason = ", reason);
	}
};

struct KL : EP::KcpListener {
	inline virtual EP::KcpPeer_u OnCreatePeer() override {
		auto o = xx::TryMakeU<KP>();
		o->serverSide = true;
		return o;
	}
	inline virtual void OnAccept(EP::KcpPeer_r const& peer) override {
		xx::CoutN("kcp ip: ", peer->addr, " accepted.");
	}
};

struct D : EP::Dialer {
	inline virtual EP::Peer_u OnCreatePeer(bool const& isKcp) override {
		return xx::TryMakeU<KP>();
	}
	virtual void OnConnect(EP::Peer_r const& peer) override {
		if (peer) {
			xx::CoutN("client connected. addr = ", peer->addr);
		}
		else {
			xx::CoutN("client conn fail.");
		}
	}
};

int main() {
	xx::IgnoreSignal();
	EP::Context ep;
	int port = 12345;
	auto listener = ep.CreateUdpPeer<KL>(port);	
	assert(listener);
	auto dialer = ep.CreateDialer<D>();
	dialer->AddAddress("127.0.0.1", port);
	dialer->Dial(2000, EP::Protocol::Kcp);
	return ep.Run(100);
}
