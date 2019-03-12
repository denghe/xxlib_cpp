#include "xx_uv.h"
int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "need args: port\n";
		return -1;
	}
	xx::Uv uv;
	auto listener = xx::Make<xx::UvTcpListener<>>(uv, "0.0.0.0", std::atoi(argv[1]));
	listener->OnAccept = [](xx::UvTcpPeer_s& peer) {
		peer->OnReceiveRequest = [peer](int const& serial, xx::Object_s&& msg)->int {
			return peer->SendResponse(serial, msg);
		};
	};
	uv.Run();
	return 0;
}
