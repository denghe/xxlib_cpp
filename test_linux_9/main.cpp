#include "xx_epoll2.hpp"
namespace EP = xx::Epoll;

int64_t counter = 0;

std::shared_ptr<xx::BBuffer> bb;
xx::BBuffer bbRead;
xx::BBuffer bbWrite;

struct KP : EP::KcpPeer {
	bool serverSide = false;
	inline virtual void OnReceive() override {
		bbRead.Reset((uint8_t*)recv.buf, recv.len);
		if (bbRead.ReadRoot(bb)) {
			Dispose();
			return;
		}
		bbWrite.Clear();
		bbWrite.WriteRoot(bb);
		if (Send(bbWrite)) {
			OnDisconnect(-3);
			Dispose();
		}
		else {
			++counter;
			recv.Clear();
			Flush();
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
			//peer->Send(".", 1);
			xx::MakeTo(bb);
			bb->Write(1, 2, 3, 4, 5, 6, 7, 8, 9, 0);
			bbWrite.Clear();
			bbWrite.WriteRoot(bb);
			peer->Send(bbWrite);
		}
		else {
			xx::CoutN("client conn fail.");
		}
	}
};

int main(int argc, char** argv) {
	xx::IgnoreSignal();
	int port = 12345;
	if (argc > 1) {
		port = atoi(argv[1]);
	}

	EP::Context ep;
	auto listener = ep.CreateUdpPeer<KL>(port);
	assert(listener);
	//auto dialer = ep.CreateDialer<D>();
	//dialer->AddAddress("127.0.0.1", port);
	//dialer->Dial(2000, EP::Protocol::Kcp);
	ep.CreateTimer(100, [](auto t) {
		xx::CoutN("counter = ", counter);
		counter = 0;
		t->SetTimeout(100);
	});
	return ep.Run(100);
}
