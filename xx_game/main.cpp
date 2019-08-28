#include "xx_uv.h"
#include "PKG_class.h"
int main() {
	xx::Uv uv;
	xx::UvListener ls(uv, "0.0.0.0", 20001, 2);
	ls.onAccept = [&](xx::UvPeer_s peer) {
		peer->onDisconnect = [peer] {
			xx::CoutN(peer->GetIP(), " disconnected.");
		};
		peer->onReceivePush = [peer](xx::Object_s&& msg)->int {
			peer->ResetTimeoutMS(5000);
			peer->SendPush(msg);
			xx::CoutN(msg);
			return 0;
		};
		peer->ResetTimeoutMS(5000);
		xx::CoutN(peer->GetIP(), " accepted.");
	};
	uv.Run();
	return 0;
}
