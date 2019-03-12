#include "xx_uv.h"

int main(int argc, char* argv[]) {
	return 0;
}









//#include "xx_uv.h"
//
//struct KcpPeer {
//	ikcpcb *kcp;
//	std::chrono::time_point<std::chrono::steady_clock> lastTime;
//	std::function<int(const char *buf, int len)> OnOutput;
//	std::function<int(const char *buf, int len)> OnReceive;
//
//	KcpPeer(xx::Guid const& conv) {
//		kcp = ikcp_create(conv, this);
//		if (!kcp) throw - 1;
//		ikcp_nodelay(kcp, 1, 10, 2, 1);
//		ikcp_wndsize(kcp, 128, 128);
//		kcp->rx_minrto = 10;
//		ikcp_setoutput(kcp, [](const char *buf, int len, ikcpcb *kcp, void *user)->int {
//			return ((KcpPeer*)user)->OnOutput(buf, len);
//		});
//		lastTime = std::chrono::steady_clock::now();
//	}
//	~KcpPeer() {
//		ikcp_release(kcp);
//	}
//
//	int Input(const char *buf, int len) {
//		if (int r = ikcp_input(kcp, buf, len)) return r;
//		return 0;
//	}
//
//	int Send(const char *buf, int len) {
//		return ikcp_send(kcp, buf, len);
//	}
//
//	void Update(int ms) {
//		//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastTime).count();
//		ikcp_update(kcp, ms);// (int)ms);
//		do {
//			char buf[512];
//			int recvLen = ikcp_recv(kcp, buf, 512);
//			if (recvLen <= 0) break;
//			OnReceive(buf, recvLen);
//		} while (true);
//	}
//};
//
//int main(int argc, char* argv[]) {
//	//xx::UvLoop loop;
//
//	xx::Guid conv;
//	auto server = xx::TryMake<KcpPeer>(conv);
//	auto client = xx::TryMake<KcpPeer>(conv);
//
//	server->OnOutput = [&](const char *buf, int len)->int {
//		return client->Input(buf, len);
//	};
//	client->OnOutput = [&](const char *buf, int len)->int {
//		return server->Input(buf, len);
//	};
//	server->OnReceive = [&](const char *buf, int len)->int {
//		return server->Send(buf, len);	// echo
//	};
//	client->OnReceive = [&](const char *buf, int len)->int {
//		std::cout << len;
//		return 0;
//	};
//
//	for (int i = 0; i < 1000; i++)
//	{
//		client->Send("a", 100);
//		server->Update(i * 10);
//		client->Update(i * 10);
//	}
//
//	//auto timer = xx::TryMake<xx::UvTimer>(loop, 0, 10);
//	//timer->OnFire = [&server, timer] {	// hold memory
//	//	server->Update();
//	//};
//	//loop.Run();
//	return 0;
//}
