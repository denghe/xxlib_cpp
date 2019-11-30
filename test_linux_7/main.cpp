#include "xx_epoll2.hpp"
namespace EP = xx::Epoll;

struct C : EP::TcpConn {
	using BaseType = EP::TcpConn;

	int id = -1;
	std::size_t counter = 0;

	inline virtual void OnConnect() override {
		//xx::CoutN("connected = ", connected);
		if (connected) {
			int r = Send(xx::Buf((void*)".", 1));
			//xx::CoutN("Send r = ", r);
		}
	}
	inline virtual int OnReceive() override {
		//xx::Cout("recv len = ", recv.len);
		++counter;
		return this->BaseType::OnReceive();
	}
};

int TestTcp(int const& threadId, int const& numTcpClients, char const* const& tarIp, int const& tarPort) {
	EP::Context ep;

	std::vector<std::shared_ptr<C>> cs;
	for (int i = 0; i < numTcpClients; i++) {
		cs.emplace_back(ep.TcpDial<C>(tarIp, tarPort, 20))->id = i;
	}

	ep.Delay(10, [&](auto t) {
		std::size_t tcpCounter = 0;
		for (auto&& c : cs) {
			tcpCounter += c->counter;
			c->counter = 0;
		}
		xx::CoutN("thread: ", threadId, ", tcpCounter: ", tcpCounter);
		t->SetTimeout(10);
		});
	return ep.Run(10);
}

struct U : EP::UdpPeer {
	using BaseType = EP::UdpPeer;

	std::size_t counter = 0;

	sockaddr_in addr;
	U(char const* const& tarIP, int const& tarPort) {
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons((uint16_t)tarPort);
		if (!inet_pton(AF_INET, tarIP, &addr.sin_addr.s_addr)) {
			throw - 1;
		}
	}

	inline virtual void Init() override {
		SendTo((sockaddr*)&addr, sizeof(addr), ".", 1);
	}

	inline virtual int OnReceive(sockaddr* fromAddr, socklen_t const& fromAddrLen, char const* const& buf, std::size_t const& len) override {
		++counter;
		SendTo(fromAddr, fromAddrLen, buf, len);
		return 0;
	}
};

int TestUdp(int const& threadId, int const& numUdpClients, char const* const& tarIp, int const& tarPort) {
	EP::Context ep;

	std::vector<std::shared_ptr<U>> us;
	for (int i = 0; i < numUdpClients; i++) {
		auto port = tarPort + ((threadId * numUdpClients + i) % 5);
		xx::CoutN("udp send tar port = ", port);
		us.emplace_back(ep.UdpBind<U>(0, tarIp, port));
	}

	ep.Delay(10, [&](auto t) {
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

	int numThreads = 6;
	int numClients = 12;
	char const* tarIP = "192.168.1.132";
	int tarPort = 10000;
	if (argc == 5) {
		numThreads = atoi(argv[1]);
		numClients = atoi(argv[2]);
		tarIP = argv[3];
		tarPort = atoi(argv[4]);
	}

	std::vector<std::thread> ts;
	for (int i = 0; i < numThreads; i++) {
		ts.emplace_back([i = i, &numClients, &tarIP, &tarPort] {
			TestUdp(i, numClients, tarIP, tarPort);
			});
	}
	for (auto&& t : ts) {
		t.join();
	}
	xx::CoutN("end.");
}
