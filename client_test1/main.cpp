#include "xx_uv.h"

void RunServer() {
	xx::Uv uv;
	xx::IUvListener_s listener1;
	xx::IUvListener_s listener2;

	listener1 = xx::Make<xx::UvTcpListener<>>(uv, "0.0.0.0", 11111);
	listener2 = xx::Make<xx::UvKcpListener<>>(uv, "0.0.0.0", 11111);

	std::function<void(xx::IUvPeer_s peer)> acceptFunc = [](xx::IUvPeer_s peer) {
		xx::CoutN("SERVER: ", peer->GetIP(), " connected.");
		peer->OnDisconnect([peer] {
			xx::CoutN("SERVER: ", peer->GetIP(), " disconnected.");
			});
	};
	listener1->OnAccept(std::function<void(xx::IUvPeer_s peer)>(acceptFunc));
	listener2->OnAccept(std::move(acceptFunc));

	uv.Run();
}

void RunClient() {
	xx::Uv uv;
	xx::IUvDialer_s dialer1;
	xx::IUvDialer_s dialer2;

	dialer1 = xx::Make<xx::UvTcpDialer<>>(uv);
	dialer2 = xx::Make<xx::UvKcpDialer<>>(uv);

	std::function<void(xx::IUvPeer_s peer)> acceptFunc = [](xx::IUvPeer_s peer) {
		if (!peer) {
			xx::CoutN("dial timeout.");
		}
		else {
			xx::CoutN("CLIENT: ", peer->GetIP(), " connected.");
			peer->OnDisconnect([peer] {
				xx::CoutN("CLIENT: ", peer->GetIP(), " disconnected.");
			});
		}
	};
	dialer1->OnAccept(std::function<void(xx::IUvPeer_s peer)>(acceptFunc));
	dialer2->OnAccept(std::move(acceptFunc));

	dialer1->Dial("127.0.0.1", 11111, 2000);
	dialer2->Dial("127.0.0.1", 11111, 2000);
	uv.Run();
}

int main() {
	std::thread t1(RunServer);
	t1.detach();

	std::thread t2(RunClient);
	t2.join();

	return 0;
}
