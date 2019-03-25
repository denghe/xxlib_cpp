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
	xx::SetConsoleUtf8();
	std::this_thread::sleep_for(100ms);
	xx::CoutN("begin");
	{
		xx::UvCoros uv;
		uv.Add([&uv](xx::Coro&& yield) {
			while (true) {
				std::vector<std::string> ips;
				int r = 0;
				if (r = uv.Resolve(yield, ips, "www.baidu.com", 500)) {
					xx::CoutN("resolve error. r = ", r);
					break;
				}
				if (!ips.size()) {
					xx::CoutN("timeout. retry.");
					continue;
				}
				for (auto&& ip : ips) {
					xx::CoutN(ip);
				}
				std::shared_ptr<HttpPeer> peer;
				if (r = uv.Dial<HttpPeer>(yield, peer, ips, 80, 2000)) {
					xx::CoutN("dial error. r = ", r);
				}
				if (!peer) {
					xx::CoutN("connect timeout. retry.");
					continue;
				}
				xx::CoutN("connected. send http header.");
				if (r = peer->Send((uint8_t*)httpHeader.data(), httpHeader.size())) {
					xx::CoutN("send error. r = ", r);
				}

				xx::CoutN("set timeoutms 3k.");
				peer->ResetTimeoutMS(3000);

				xx::Cout("wait disconnect.");
				while (!peer->Disposed()) {
					yield();
					xx::Cout(".");
				}
				//xx::CoutN("disconnected. retry.");
				break;
			};
		});
		uv.Run();
	}
	xx::CoutN("end.");
	return 0;
}
