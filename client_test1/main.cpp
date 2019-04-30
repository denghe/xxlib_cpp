#include "xx_uv.h"
#include "xx_uv_lua.h"

void RunServer() {
	xx::Uv uv;
	auto&& listener = xx::Make<xx::UvListener>(uv, "0.0.0.0", 11111);
	listener->onAccept = [](xx::UvPeer_s peer) {
		xx::CoutN("SERVER: ", peer->GetIP(), " connected with ", (peer->IsKcp() ? "KCP" : "TCP"));
		peer->onDisconnect = [peer] {
			xx::CoutN("SERVER: ", peer->GetIP(), " disconnected.");
		};
		peer->onReceivePush = [peer](xx::Object_s && msg) {
			xx::CoutN("SERVER: ", peer->GetIP(), " OnReceivePush ", msg);
			return peer->SendPush(msg);	// echo
		};
	};
	uv.Run();
}

void RunClient() {
	xx::Uv uv;
	auto&& dialer = xx::Make<xx::UvDialer>(uv);
	dialer->onAccept = [](xx::UvPeer_s peer) {
		if (!peer) {
			xx::CoutN("dial timeout.");
		}
		else {
			xx::CoutN("CLIENT: connected.");
			peer->onDisconnect = [peer] {
				xx::CoutN("CLIENT: disconnected.");
			};
			peer->onReceivePush = [peer](xx::Object_s && msg) {
				xx::CoutN("CLIENT: OnReceivePush ", msg);
				return -1;
			};
			auto&& bb = xx::Make<xx::BBuffer>();
			bb->Write(1u, 2u, 3u, 4u, 5u);
			peer->SendPush(bb);
		}
	};
	dialer->Dial("127.0.0.1", 11111, 2000);
	uv.Run();
}

int main() {
	std::thread t1(RunServer);
	t1.detach();

	std::thread t2(RunClient);
	t2.join();

	return 0;
}
