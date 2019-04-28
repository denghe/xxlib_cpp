#include "xx_uv.h"
#include "xx_uv_lua.h"

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
		peer->OnReceivePush([peer](xx::Object_s && msg) {
			xx::CoutN("SERVER: ", peer->GetIP(), " OnReceivePush ", msg);
			return peer->SendPush(msg);	// echo
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
			xx::CoutN("CLIENT: connected.");
			peer->OnDisconnect([peer] {
				xx::CoutN("CLIENT: disconnected.");
			});
			peer->OnReceivePush([peer](xx::Object_s && msg) {
				xx::CoutN("CLIENT: OnReceivePush ", msg);
				return -1;
			});
			auto&& bb = xx::Make<xx::BBuffer>();
			bb->Write(1u, 2u, 3u, 4u, 5u);
			peer->SendPush(bb);
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
