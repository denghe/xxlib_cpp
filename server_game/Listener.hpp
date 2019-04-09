inline void Listener::Accept(std::shared_ptr<xx::UvKcpBasePeer> peer_) noexcept {
	auto&& peer = xx::As<Peer>(peer_);
	xx::CoutN(peer->GetIP(), " connected.");
	peer->catchFish = &catchFish;
	peer->OnDisconnect = [peer] {		// 用这个事件回调来持有 peer 指针
		xx::CoutN(peer->GetIP(), " disconnected.");
	};
}

inline Listener::Listener(xx::Uv& uv, std::string const& ip, int const& port)
	: BaseType(uv, ip, port) {
	if (int r = catchFish.Init("cfg.bin")) throw r;
	xx::MakeTo(looper, uv, 0, 1, [this] {
		auto currTicks = xx::NowEpoch10m();
		ticksPool += currTicks - lastTicks;
		lastTicks = currTicks;
		while (ticksPool > ticksPerFrame) {
			(void)catchFish.Update();
			ticksPool -= ticksPerFrame;
		}
	});
}
