#pragma execution_character_set("utf-8")
#include "xx_uv_coros.h"

struct HttpPeer : xx::UvTcpBasePeer {
	using xx::UvTcpBasePeer::UvTcpBasePeer;
	virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
		xx::Cout(std::string((char*)recvBuf, recvLen));
		return 0;
	}
};
std::string httpHeader = "GET / HTTP/1.1\r\n\r\n";

using namespace std::chrono;
int main() {
	std::this_thread::sleep_for(100ms);
	xx::CoutN("begin");
	{
		xx::UvCoros uv;
		uv.Add([&uv](xx::Coro&& yield) {
			while (true) {
				std::vector<std::string> ips;
				if (uv.Resolve(yield, ips, "www.baidu.com", 500)) break;
				if (!ips.size()) {
					xx::CoutN("timeout. retry.");
					continue;
				}
				for (auto&& ip : ips) {
					xx::CoutN(ip);
				}
				std::shared_ptr<HttpPeer> peer;
				if (uv.Dial<HttpPeer>(yield, peer, ips, 80, 2000)) break;
				if (!peer) {
					xx::CoutN("connect timeout. retry.");
					continue;
				}
				xx::CoutN("connected.");
				if (peer->Send((uint8_t*)httpHeader.data(), httpHeader.size())) break;
				while (!peer->Disposed()) yield();
				xx::CoutN("disconnected. retry.");
			};
		});
		uv.Run();
	}
	xx::CoutN("end.");
	return 0;
}
