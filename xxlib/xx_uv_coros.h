#pragma once
#include "xx_uv.h"
#include "xx_coros.h"

namespace xx {

	struct UvCoros : Uv, Coros {
		// 提供稳定在某个帧数的循环 update 机制
		std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
		std::chrono::nanoseconds totalDurations;
		UvTimer_s frameUpdater;
		UvCoros(double const& framesPerSecond = 61)
			: Uv() {
			MakeTo(frameUpdater, *this, 0, 1, [this, nanosPerFrame = std::chrono::nanoseconds(int64_t(1.0 / framesPerSecond * 1000000000))]{
				auto currTime = std::chrono::steady_clock::now();
				totalDurations += currTime - lastUpdateTime;
				lastUpdateTime = currTime;
				while (totalDurations > nanosPerFrame) {
					if (!RunOnce()) {
						frameUpdater.reset();
						return;
					};
					totalDurations -= nanosPerFrame;
				}
			});
			lastUpdateTime = std::chrono::steady_clock::now();
			totalDurations = std::chrono::nanoseconds(0);
		}

		int Resolve(Coro& yield, std::vector<std::string>& rtv, std::string const& domainName, int64_t const& timeoutMS = 0) {
			auto resolver = TryMake<UvResolver>(*this);
			if (!resolver) return -1;
			bool finished = false;
			resolver->OnFinish = [&] {
				rtv = std::move(resolver->ips);
				finished = true;
			};
			resolver->Resolve(domainName, timeoutMS);
			while (!finished) {
				if (resolver->Disposed()) return -2;
				yield();
			}
			return 0;
		}

		template<typename PeerType = UvTcpPeer>
		int Dial(Coro& yield, std::shared_ptr<PeerType>& rtv, std::vector<std::string>& ips, uint16_t const& port, int64_t const& timeoutMS = 0) {
			auto dialer = TryMake<UvTcpDialer<PeerType>>(*this);
			if (!dialer) return -1;
			bool finished = false;
			dialer->OnAccept = [&](auto& peer){
				rtv = std::move(peer);
				finished = true;
			};
			if (int r = dialer->Dial(ips, port, timeoutMS)) return r;
			while (!finished) {
				if (dialer->Disposed()) return -2;
				yield();
			}
			return 0;
		}









		//int MakeResolverTo(UvItem_s& holder, std::vector<std::string>& outResult, std::string const& domainName, int64_t const& timeoutMS = 0) {
		//	auto resolver = TryMake<UvResolver>(*this);
		//	if (!resolver) return -1;
		//	resolver->OnFinish = [resolver = &*resolver, holder = &holder, outResult = &outResult]{
		//		*outResult = std::move(resolver->ips);
		//		holder->reset();
		//	};
		//	resolver->Resolve(domainName, timeoutMS);
		//	holder = std::move(resolver);
		//	return 0;
		//}

		//template<typename PeerType = UvTcpPeer>
		//int MakeTcpDialerTo(UvItem_s& holder, std::shared_ptr<PeerType>& outResult, std::vector<std::string>& ips, uint16_t const& port, int64_t const& timeoutMS = 0) {
		//	auto dialer = TryMake<UvTcpDialer<PeerType>>(*this);
		//	if (!dialer) return -1;
		//	dialer->OnAccept = [dialer = &*dialer, holder = &holder, outResult = &outResult](auto& peer){
		//		*outResult = std::move(peer);
		//		holder->reset();
		//	};
		//	if (int r = dialer->Dial(ips, port, timeoutMS))
		//		return r;
		//	holder = std::move(dialer);
		//	return 0;
		//}

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
