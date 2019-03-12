#pragma execution_character_set("utf-8")
#include "xx_uv_stackless.h"

struct HttpPeer : xx::UvTcpBasePeer {
	using xx::UvTcpBasePeer::UvTcpBasePeer;
	virtual int Unpack(uint8_t* const& recvBuf, uint32_t const& recvLen) noexcept override {
		xx::Cout(std::string((char*)recvBuf, recvLen));
		return 0;
	}
};
std::string httpHeader = "GET / HTTP/1.1\r\n\r\n";

struct SleepFunc {
	int leftTicks = 0;
	inline void Init(int const& ticks) {
		leftTicks = ticks;
	}
	inline operator bool() noexcept {
		xx::CoutN(leftTicks);
		return --leftTicks > 0;
	}
};

struct Sleeper {
	SleepFunc Sleep;

	int lineNumber = 0;
	inline int Update() {
		COR_BEGIN
			while (Sleep) {
				COR_YIELD
			}
		COR_END
	}
	inline operator bool() noexcept {
		return lineNumber = Update();
	}
};
using namespace std::chrono;
int main() {
	std::this_thread::sleep_for(100ms);
	xx::CoutN("begin");
	{
		xx::UvStackless uv;
		struct Ctx1 {
			xx::UvItem_s func;
			std::vector<std::string> ips;
			std::shared_ptr<HttpPeer> peer;
			Sleeper Sleeper;
		};
		uv.Add([&uv, zs = xx::Make<Ctx1>()](int const& lineNumber) {
			COR_BEGIN
				zs->Sleeper.Sleep.Init(120);
				while (zs->Sleeper) {
					COR_YIELD
				}
			LabRetry:
				if (uv.MakeResolverTo(zs->func, zs->ips, "www.baidu.com", 500)) break;
				while (zs->func) {
					COR_YIELD
				}
				if (!zs->ips.size()) {
					xx::CoutN("timeout. retry.");
					goto LabRetry;
				}
				for (decltype(auto) ip : zs->ips) {
					xx::CoutN(ip);
				}
				if (uv.MakeTcpDialerTo<HttpPeer>(zs->func, zs->peer, zs->ips, 80, 2000)) break;
				while (zs->func) {
					COR_YIELD
				}
				if (!zs->peer) {
					xx::CoutN("connect timeout. retry.");
					goto LabRetry;
				}
				xx::CoutN("connected.");
				if (zs->peer->Send((uint8_t*)httpHeader.data(), httpHeader.size())) break;
				while (!zs->peer->Disposed()) {
					COR_YIELD
				}
				xx::CoutN("disconnected. retry.");
				goto LabRetry;
			COR_END
		});
		uv.Run();
	}
	xx::CoutN("end.");
	return 0;
}



// callback mode
//#pragma execution_character_set("utf-8")
//#include "xx_uv.h"
//int main() {
//	xx::CoutN("begin");
//	{
//		xx::Uv uv;
//		auto resolver = xx::Make<xx::UvResolver>(uv);
//		resolver->OnFinish = [&] {
//			if (resolver->ips.size()) {
//				for (decltype(auto) ip : resolver->ips) {
//					xx::CoutN(ip);
//				}
//			}
//			else {
//				xx::CoutN("timeout. retry.");
//				resolver->Resolve("www.baidu.com", 500);
//			}
//		};
//		resolver->Resolve("www.jljcxovpzivakfnewnf.com", 500);
//		uv.Run();
//		xx::CoutN("end.");
//	}
//	return 0;
//}
