inline void Listener::Accept(std::shared_ptr<xx::UvKcpBasePeer> peer_) noexcept {
	auto&& peer = xx::As<Peer>(peer_);
	xx::CoutTN(peer->GetIP(), " connected.");
	peer->catchFish = &catchFish;

	// 用这个事件回调来持有 peer 指针
	peer->OnDisconnect = [peer] {
		xx::CoutTN(peer->GetIP(), " disconnected.");
	};

	// 启用超时检测. xx ms 没收到包就 Disconnect. 收到 Ping 会再次重置
	peer->ResetTimeoutMS(10000);
}

inline Listener::Listener(xx::Uv& uv, std::string const& ip, int const& port)
	: BaseType(uv, ip, port) {
	if (int r = catchFish.Init("cfg.bin")) throw r;
	xx::MakeTo(looper, uv, 0, 1, [this] {
		auto currTicks = xx::NowEpoch10m();
		ticksPool += currTicks - lastTicks;
		lastTicks = currTicks;
		while (ticksPool > ticksPerFrame) {
			// game frame loop
			(void)catchFish.Update();
			ticksPool -= ticksPerFrame;
		}
	});
}
