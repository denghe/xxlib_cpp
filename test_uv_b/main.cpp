#include "xx_uv.h"

auto msg = xx::Make<xx::BBuffer>();
struct Peer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
	using BaseType::BaseType;

	int64_t last = xx::NowSteadyEpochMS();
	inline void SendData() {
		last = xx::NowSteadyEpochMS();
		SendRequest(msg, [this](xx::Object_s&& msg) {
			if (!msg) {
				xx::CoutN("timeout ( 2000ms ). retry");
			}
			else {
				ResetTimeoutMS(3000);
				auto elapsedSec = (xx::NowSteadyEpochMS() - last);
				std::cout << elapsedSec << std::endl;
			}
			if (!Disposed()) {
				SendData();
			}
			return 0;
		}, 2000);
		Flush();
	}
};

int main(int argc, char* argv[]) {
	if (argc < 3) {
		xx::CoutN("need args: ip port");
		return -1;
	}
	xx::Uv loop;
	auto dialer = xx::Make<xx::UvKcpDialer<Peer>>(loop);
	dialer->OnAccept = [&](auto& peer) {
		if (peer) {
			xx::CoutN("dialer accepted. ", peer->guid);
			peer->OnDisconnect = [&] {
				xx::CoutN("disconnected. redial...");
				int r = dialer->Dial(argv[1], std::atoi(argv[2]));
			};
			peer->ResetTimeoutMS(3000);
			peer->SendData();
		}
		else {
			xx::CoutN("dialer timeout");
		}
	};
	xx::CoutN("dial...");
	int r = dialer->Dial(argv[1], std::atoi(argv[2]));
	loop.Run();
	xx::CoutN("end");
	return 0;
}
