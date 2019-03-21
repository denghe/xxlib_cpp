#include "xx_uv.h"

using PeerType = xx::UvKcpPeer;
using ListenerType = xx::UvKcpListener<PeerType>;
using DialerType = xx::UvKcpDialer<PeerType>;

//using PeerType = xx::UvTcpPeer;
//using ListenerType = xx::UvTcpListener<PeerType>;
//using DialerType = xx::UvTcpDialer<PeerType>;

int main(int argc, char* argv[]) {
	xx::Uv uv;
	auto&& listener = xx::Make<ListenerType>(uv, "0.0.0.0", 12345);
	listener->OnAccept = [](std::shared_ptr<PeerType>& peer) {
		xx::CoutN("listener accept ", xx::Uv::ToIpPortString(peer->addr));
		peer->OnReceivePush = [peer](xx::Object_s&& msg) {	// hold
			return peer->SendPush(msg);	// echo
		};
	};

	auto&& dialer = xx::Make<DialerType>(uv);
	dialer->OnAccept = [](std::shared_ptr<PeerType> peer) {
		xx::CoutN("dialer ", peer ? "accept" : "timeout");
		peer->OnReceivePush = [peer](xx::Object_s&& msg) {
			xx::CoutN("dialer peer recv ", msg);
			return peer->SendPush(msg);
		};
		peer->SendPush(xx::Make<xx::BBuffer>());
	};
	auto&& r = dialer->Dial("127.0.0.1", 12345, 3000);
	xx::CoutN(r);
	uv.Run();
	return 0;
}
