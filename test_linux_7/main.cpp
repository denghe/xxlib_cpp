#include "xx_epoll2.hpp"
namespace EP = xx::Epoll;

struct D;
struct P : EP::TcpPeer {
	EP::Ref<D> dialer;
	size_t counter = 0;
	virtual void OnReceive() override;
	virtual void OnDisconnect(int const& reason) override;
};
struct K : EP::KcpPeer {
	EP::Ref<D> dialer;
	size_t counter = 0;
	virtual void OnReceive() override;
	virtual void OnDisconnect(int const& reason) override;
};

struct D : EP::Dialer {
	EP::Ref<P> peer;
	EP::Protocol protocol = EP::Protocol::Both;
	int Dial();
	virtual EP::Peer_u OnCreatePeer(bool const& isKcp) override;
	virtual void OnConnect(EP::Peer_r const& peer) override;
protected:
	using D::Dialer::Dial;
};



inline void D::OnConnect(EP::Peer_r const& p_) {
	if (auto p = p_.Lock()) {
		auto&& init = [&](auto peer) {
			peer->dialer = this;
			peer->Send(xx::Buf((void*)"..........", 10));
		};
		if(auto peer = dynamic_cast<P*>(p)) {
			init(peer);
		}
		else if (auto peer = dynamic_cast<K*>(p)) {
			init(peer);
		}
		else {
			throw - 1;
		}
	}
	else {
		int r = Dial();
		xx::CoutN("dial r = ", r);
	}
}
inline int D::Dial() {
	return this->Dialer::Dial(20, protocol);
}
inline EP::Peer_u D::OnCreatePeer(bool const& isKcp) {
	if (isKcp) {
		return xx::TryMakeU<K>();
	}
	else {
		return xx::TryMakeU<P>();
	}
}



inline void P::OnReceive() {
	++counter;
	this->TcpPeer::OnReceive();
}

inline void P::OnDisconnect(int const& reason) {
	xx::CoutN("disconnected.");
	if (auto d = dialer.Lock()) {
		int r = d->Dial();
		xx::CoutN("dial r = ", r);
	}
}

inline void K::OnReceive() {
	xx::CoutN("K recv len = ", recv.len);
	++counter;
	this->KcpPeer::OnReceive();
}

inline void K::OnDisconnect(int const& reason) {
	xx::CoutN("disconnected.");
	if (auto d = dialer.Lock()) {
		int r = d->Dial();
		xx::CoutN("dial r = ", r);
	}
}



int Test1(int const& threadId, int const& numTcpClients, char const* const& tarIp, int const& tarPort, int const& tcpKcp) {
	EP::Context ep;
	std::vector<EP::Ref<D>> ds;

	for (int i = 0; i < numTcpClients; i++) {
		auto d = ep.CreateDialer<D>();
		ds.emplace_back(d);
		d->AddAddress(tarIp, tarPort);
		d->protocol = (EP::Protocol)tcpKcp;
		int r = d->Dial();
		xx::CoutN("dial r = ", r);
	}

	ep.CreateTimer(10, [&](auto t) {
		size_t tcpCounter = 0;
		for (auto&& d : ds) {
			if (auto p = d->peer.Lock()) {
				tcpCounter += p->counter;
				p->counter = 0;
			}
		}
		xx::CoutN("thread: ", threadId, ", counter: ", tcpCounter);
		t->SetTimeout(10);
		});
	return ep.Run(10);
}





//struct U : EP::UdpPeer {
//	using BaseType = EP::UdpPeer;
//	size_t counter = 0;
//	bool received = false;
//	inline virtual void OnReceive() override {
//		++counter;
//		this->UdpPeer::OnReceive();
//	}
//};
//
//int TestUdp(int const& threadId, int const& numUdpClients, char const* const& tarIp, int const& tarPort, int const& numPorts) {
//	EP::Context ep;
//
//	sockaddr_in6 addr;
//	memset(&addr, 0, sizeof(addr));
//	auto a4 = (sockaddr_in*)&addr;
//	a4->sin_family = AF_INET;
//	a4->sin_port = htons((uint16_t)tarPort);
//	if (!inet_pton(AF_INET, tarIp, &a4->sin_addr.s_addr)) {
//		throw - 1;
//	}
//
//	std::vector<EP::Ref<U>> us;
//	for (int i = 0; i < numUdpClients; i++) {
//		auto port = tarPort + ((threadId * numUdpClients + i) % numPorts);
//		xx::CoutN("udp send tar port = ", port);
//		auto u = ep.CreateUdpPeer<U>(0);
//		if (!u) {
//			xx::CoutN("thread: ", threadId, ", CreateUdpPeer failed.");
//		}
//		else {
//			us.emplace_back(u);
//		}
//	}
//
//	ep.CreateTimer(100, [&](auto t) {
//		size_t udpCounter = 0;
//		for (auto&& u : us) {
//			if (auto o = u.Lock()) {
//				if (!o->counter) {
//					o->addr = addr;
//					o->Send(".", 1);
//				}
//				else {
//					udpCounter += o->counter;
//					o->counter = 0;
//				}
//			}
//		}
//		xx::CoutN("thread: ", threadId, ", udpCounter: ", udpCounter);
//		t->SetTimeout(100);
//		});
//	return ep.Run(100);
//}

int main(int argc, char** argv) {
	xx::IgnoreSignal();

	int numThreads = 1;
	int numClients = 1;
	char const* tarIP = "192.168.1.236";
	int tarPort = 12345;
	//int numPorts = 1;	// udp > 0
	int tcpKcp = 1;	// 0:tcp  1:kcp  2:both

	if (argc == 6) {
		numThreads = atoi(argv[1]);
		numClients = atoi(argv[2]);
		tarIP = argv[3];
		tarPort = atoi(argv[4]);
		//numPorts = atoi(argv[5]);
		tcpKcp = atoi(argv[5]);
	}

	std::vector<std::thread> ts;
	for (int i = 0; i < numThreads; i++) {
		ts.emplace_back([i = i, &numClients, &tarIP, &tarPort, &tcpKcp] {
			//TestUdp(i, numClients, tarIP, tarPort, numPorts);
			Test1(i, numClients, tarIP, tarPort, tcpKcp);
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