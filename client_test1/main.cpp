//#pragma execution_character_set("utf-8")
//#include "xx_uv_stackless.h"
//#include "PKG_class.h"
//
//struct ClientPeer : xx::UvTcpPeer {
//	using xx::UvTcpPeer::UvTcpPeer;
//};
//
int main() {
//	xx::CoutN("begin");
//	{
//		xx::UvStackless uv;
//		struct Ctx1 {
//			xx::UvItem_s func;
//			std::vector<std::string> ips;
//			std::shared_ptr<ClientPeer> peer;
//		};
//		uv.Add([&uv, zs = xx::Make<Ctx1>()](int const& lineNumber) {
//			COR_BEGIN
//			LabRetry:
//				if (uv.MakeResolverTo(zs->func, zs->ips, "127.0.0.1", 500)) break;
//				while (zs->func) {
//					COR_YIELD
//				}
//				if (!zs->ips.size()) {
//					xx::CoutN("timeout. retry.");
//					goto LabRetry;
//				}
//				for (auto&& ip : zs->ips) {
//					xx::CoutN(ip);
//				}
//				if (uv.MakeTcpDialerTo<ClientPeer>(zs->func, zs->peer, zs->ips, 12345, 2000)) break;
//				while (zs->func) {
//					COR_YIELD
//				}
//				if (!zs->peer) {
//					xx::CoutN("connect timeout. retry.");
//					goto LabRetry;
//				}
//				xx::CoutN("connected.");
//				//if (zs->peer->Send((uint8_t*)httpHeader.data(), httpHeader.size())) break;
//				while (!zs->peer->Disposed()) {
//					COR_YIELD
//				}
//				xx::CoutN("disconnected. retry.");
//				goto LabRetry;
//			COR_END
//		});
//		uv.Run();
//	}
//	xx::CoutN("end.");
	return 0;
}
