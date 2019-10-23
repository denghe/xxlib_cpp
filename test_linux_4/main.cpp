#include "xx_uv_http.h"

// todo: 超时检测机制
// todo: http dialer

int main() {
	xx::IgnoreSignal();

	xx::Uv uv;
	xx::UvHttpListener listener(uv, "0.0.0.0", 12345);
	listener.onAccept = [](xx::UvHttpPeer_s peer) {
		peer->onReceiveHttp = [peer]()->int {
			xx::CoutN(peer->ip, " recv : {\n", peer->Dump(), "\n}");
			return peer->SendHttpResponse_Text("hello world");
		};
		peer->onDisconnect = [peer] {
			xx::CoutN(peer->ip, " disconnected.");
		};
		xx::CoutN(peer->ip, " connected.");
	};
	uv.Run();
	return 0;
}
