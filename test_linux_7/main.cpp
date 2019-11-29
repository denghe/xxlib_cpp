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

int Test(int const& threadId, int const& clientCount) {
	EP::Context ep;

	std::vector<std::shared_ptr<C>> cs;
	for (int i = 0; i < clientCount; i++) {
		cs.emplace_back(ep.TcpDial<C>("192.168.1.132", 12345, 20))->id = i;
	}

	ep.Delay(10, [&](auto t) {
		std::size_t counter = 0;
		for (auto&& c : cs) {
			counter += c->counter;
			c->counter = 0;
		}
		xx::CoutN("thread: ", threadId, ", counter: ", counter);
		t->SetTimeout(10);
		});
	return ep.Run(10);
}

int main() {
	xx::IgnoreSignal();

	int numThreads = 1;
	int numClients = 1000;

	std::vector<std::thread> ts;
	for (int i = 0; i < numThreads; i++) {
		ts.emplace_back([i = i, numClients] {
			Test(i, numClients);
		});
	}
	for (auto&& t : ts) {
		t.join();
	}
	xx::CoutN("end.");
}
