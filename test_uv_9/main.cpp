#include "xx_uv.h"
struct Peer : xx::UvTcpPeer {
	using xx::UvTcpPeer::UvTcpPeer;
	int64_t last;
	inline int SendData() {
		last = std::chrono::system_clock::now().time_since_epoch().count();
		auto msg = xx::Make<xx::BBuffer>();
		return SendRequest(msg, [this](xx::Object_s&& msg) {
			if (!msg) {
				std::cout << "timeout. retry";
			}
			else {
				auto elapsedSec = double(std::chrono::system_clock::now().time_since_epoch().count() - last) / 10000000.0;
				std::cout << elapsedSec << std::endl;
			}
			return SendData();
		}, 2000);
	}
};
int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cout << "need args: ip port\n";
		return -1;
	}
	xx::Uv uv;
	auto dialer = xx::Make<xx::UvTcpDialer<Peer>>(uv);
	dialer->OnConnect = [&dialer] {
		dialer->peer->SendData();
	};
	dialer->Dial(argv[1], std::atoi(argv[2]));
	uv.Run();
	std::cout << "end.";
	return 0;
}
