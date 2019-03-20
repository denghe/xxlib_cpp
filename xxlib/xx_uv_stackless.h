#pragma once
#include "xx_uv.h"

namespace xx {

	struct UvStackless : Uv, Stackless {
		std::chrono::time_point<std::chrono::steady_clock> corsLastTime;
		std::chrono::nanoseconds corsDurationPool;
		UvTimer_s corsTimer;
		UvStackless(double const& framesPerSecond = 61)
			: Uv() {
			corsTimer = MakeTo(corsTimer, *this, 0, 1, [this, nanosPerFrame = std::chrono::nanoseconds(int64_t(1.0 / framesPerSecond * 1000000000))]{
				auto currTime = std::chrono::steady_clock::now();
				corsDurationPool += currTime - corsLastTime;
				corsLastTime = currTime;
				while (corsDurationPool > nanosPerFrame) {
					if (!RunOnce()) {
						corsTimer.reset();
						return;
					};
					corsDurationPool -= nanosPerFrame;
				}
			});
			corsLastTime = std::chrono::steady_clock::now();
			corsDurationPool = std::chrono::nanoseconds(0);
		}


		int MakeResolverTo(UvItem_s& holder, std::vector<std::string>& outResult, std::string const& domainName, int64_t const& timeoutMS = 0) {
			auto resolver = TryMake<UvResolver>(*this);
			if (!resolver) return -1;
			resolver->OnFinish = [resolver = &*resolver, holder = &holder, outResult = &outResult]{
				*outResult = std::move(resolver->ips);
				holder->reset();
			};
			resolver->Resolve(domainName, timeoutMS);
			holder = std::move(resolver);
			return 0;
		}

		template<typename PeerType = UvTcpPeer>
		int MakeTcpDialerTo(UvItem_s& holder, std::shared_ptr<PeerType>& outResult, std::vector<std::string>& ips, uint16_t const& port, int64_t const& timeoutMS = 0) {
			auto dialer = TryMake<UvTcpDialer<PeerType>>(*this);
			if (!dialer) return -1;
			dialer->OnAccept = [dialer = &*dialer, holder = &holder, outResult = &outResult](auto& peer){
				*outResult = std::move(peer);
				holder->reset();
			};
			if (int r = dialer->Dial(ips, port, timeoutMS))
				return r;
			holder = std::move(dialer);
			return 0;
		}

		//template<typename PeerType = UvUdpKcpPeer>
		//int MakeUdpDialerTo(UvItem_s& holder, std::shared_ptr<PeerType>& outResult, std::string ip, uint16_t const& port) {
		//	auto dialer = TryMake<UvUdpKcpDialer<PeerType>>(*this);
		//	if (!dialer) return -1;
		//	dialer->OnConnect = [dialer = &*dialer, holder = &holder, outResult = &outResult]{
		//		*outResult = std::move(dialer->peer);
		//		holder->reset();
		//	};
		//	if (int r = dialer->Dial(ip, port, timeoutMS))
		//		return r;
		//	holder = std::move(dialer);
		//	return 0;
		//}
	};
}
