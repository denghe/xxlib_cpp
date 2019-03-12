#include "xx_uv.h"

auto msg = xx::Make<xx::BBuffer>();
struct Peer : xx::UvUdpKcpPeer {
	using BaseType = xx::UvUdpKcpPeer;
	using BaseType::BaseType;

	std::chrono::time_point<std::chrono::system_clock> last;
	inline void SendData() {
		last = std::chrono::system_clock::now();
		SendRequest(msg, [this](xx::Object_s&& msg) {
			if (!msg) {
				xx::CoutN("timeout ( 2000ms ). retry");
			}
			else {
				ResetLastReceiveMS();
				auto elapsedSec = double(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - last).count()) / 1000000000.0;
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
	auto dialer = xx::Make<xx::UvUdpKcpDialer<Peer>>(loop);
	dialer->OnConnect = [&] {
		auto& peer = dialer->peer;
		if (peer) {
			peer->OnDisconnect = [&] {
				int r = dialer->Dial(argv[1], std::atoi(argv[2]));
			};
			peer->SetReceiveTimeoutMS(3000);
			peer->SendData();
		}
	};
	int r = dialer->Dial(argv[1], std::atoi(argv[2]));
	loop.Run();
	xx::CoutN("end");
	return 0;
}
