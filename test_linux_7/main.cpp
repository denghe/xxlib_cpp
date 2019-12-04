﻿#include "xx_epoll2.hpp"
namespace EP = xx::Epoll;

struct D;
struct P : EP::TcpPeer {
	EP::Ref<D> dialer;
	std::size_t counter = 0;
	virtual void OnReceive() override;
	virtual void OnDisconnect(int const& reason) override;
};

struct D : EP::TcpDialer {
	EP::Ref<P> peer;
	virtual EP::TcpPeer_u OnCreatePeer() override;
	virtual void OnConnect(EP::TcpPeer_r const& peer) override;
};

inline void D::OnConnect(EP::TcpPeer_r const& p_) {
	if (peer) {
		peer->Dispose();
	}
	peer = p_.As<P>();
	if (peer) {
		peer->dialer = this;
		peer->Send(xx::Buf((void*)".", 1));
	}
	else {
		int r = Dial(20);
		xx::CoutN("dial r = ", r);
	}
}

inline EP::TcpPeer_u D::OnCreatePeer() {
	return xx::TryMakeU<P>();
}

inline void P::OnReceive() {
	++counter;
	this->TcpPeer::OnReceive();
}

inline void P::OnDisconnect(int const& reason) {
	xx::CoutN("disconnected.");
	if (auto d = dialer.Lock()) {
		int r = d->Dial(20);
		xx::CoutN("dial r = ", r);
	}
}

int TestTcp(int const& threadId, int const& numTcpClients, char const* const& tarIp, int const& tarPort) {
	EP::Context ep;
	std::vector<EP::Ref<D>> ds;

	for (int i = 0; i < numTcpClients; i++) {
		auto d = ep.CreateTcpDialer<D>();
		ds.emplace_back(d);
		d->AddAddress(tarIp, tarPort);
		int r = d->Dial(20);
		xx::CoutN("dial r = ", r);
	}

	ep.CreateTimer(10, [&](auto t) {
		std::size_t tcpCounter = 0;
		for (auto&& d : ds) {
			if (auto p = d->peer.Lock()) {
				tcpCounter += p->counter;
				p->counter = 0;
			}
		}
		xx::CoutN("thread: ", threadId, ", tcpCounter: ", tcpCounter);
		t->SetTimeout(10);
		});
	return ep.Run(10);
}





struct U : EP::UdpPeer {
	using BaseType = EP::UdpPeer;
	std::size_t counter = 0;

	inline virtual void OnReceive(sockaddr* fromAddr, char const* const& buf, std::size_t const& len) override {
		++counter;
		SendTo(fromAddr, buf, len);
	}
};

int TestUdp(int const& threadId, int const& numUdpClients, char const* const& tarIp, int const& tarPort, int const& numPorts) {
	EP::Context ep;

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((uint16_t)tarPort);
	if (!inet_pton(AF_INET, tarIp, &addr.sin_addr.s_addr)) {
		throw - 1;
	}

	std::vector<EP::Ref<U>> us;
	for (int i = 0; i < numUdpClients; i++) {
		auto port = tarPort + ((threadId * numUdpClients + i) % numPorts);
		xx::CoutN("udp send tar port = ", port);
		auto u = ep.CreateUdpPeer<U>(0);
		if (!u) {
			xx::CoutN("thread: ", threadId, ", CreateUdpPeer failed.");
		}
		else {
			us.emplace_back(u);
			u->SendTo((sockaddr*)&addr, ".", 1);
		}
	}

	ep.CreateTimer(10, [&](auto t) {
		std::size_t udpCounter = 0;
		for (auto&& u : us) {
			udpCounter += u->counter;
			u->counter = 0;
		}
		xx::CoutN("thread: ", threadId, ", udpCounter: ", udpCounter);
		t->SetTimeout(10);
		});
	return ep.Run(10);
}

int main(int argc, char** argv) {
	xx::IgnoreSignal();

	int numThreads = 1;
	int numClients = 1;
	char const* tarIP = "192.168.1.132";
	int tarPort = 12345;
	int numPorts = 0;	// udp > 0

	if (argc == 6) {
		numThreads = atoi(argv[1]);
		numClients = atoi(argv[2]);
		tarIP = argv[3];
		tarPort = atoi(argv[4]);
		numPorts = atoi(argv[5]);
	}

	std::vector<std::thread> ts;
	for (int i = 0; i < numThreads; i++) {
		ts.emplace_back([i = i, &numClients, &tarIP, &tarPort, &numPorts] {
			if (numPorts) {
				TestUdp(i, numClients, tarIP, tarPort, numPorts);
			}
			else {
				TestTcp(i, numClients, tarIP, tarPort);
			}
			});
	}
	for (auto&& t : ts) {
		t.join();
	}
	xx::CoutN("end.");
}



// 下面代码展示一种 try 空指针的方式

//template<typename T>
//struct Ptr {
//	T* ptr = nullptr;
//	int lineNumber = -1;
//	T* operator->() {
//		if (!ptr) throw lineNumber;
//		return ptr;
//	}
//	void Clear(int const& lineNumber) {
//		if (ptr) {
//			delete ptr;
//			ptr = nullptr;
//			this->lineNumber = lineNumber;
//		}
//	}
//	void Reset(T* const& ptr, int const& lineNumber) {
//		Clear(lineNumber);
//		this->ptr = ptr;
//	}
//};
//
//#define PtrReset(self, ptr) self.Reset(nullptr, __LINE__);
//#define PtrClear(self) self.Clear(__LINE__);
//
//struct Foo {
//	int n = 0;
//	bool disposed = false;
//};

//int main(int argc, char** argv) {

	//Foo* f = new Foo;
	//f->n = 123;
	//Ptr<Foo> p;
	//p.ptr = f;
	//try {
	//	xx::CoutN(p->n);
	//	PtrClear(p);
	//	xx::CoutN(p->n);
	//}
	//catch (int const& lineNumber) {
	//	std::cout << lineNumber << std::endl;
	//}
	//catch (std::exception const& e) {
	//	std::cout << e.what() << std::endl;
	//}
//}