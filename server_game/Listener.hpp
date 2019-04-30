inline Service::Service() {
	// 创建游戏上下文并加载配置文件( 多个相同游戏上下文可共享同一配置文件 )
	catchFish = xx::Make<CatchFish>();
	if (int r = catchFish->Init("cfg.bin")) throw r;

	// tcp, kcp 同时监听同一端口
	listener = xx::Make<xx::UvListener>(uv, "0.0.0.0", 12345);

	// 为连接创建上下文对象并附加到连接. 同步生命周期
	listener->onAccept = [this](xx::UvPeer_s peer) {
		peer->onDisconnect = [peer] {		// hold memory
			xx::CoutTN(peer->GetIP(), " disconnected.");
		};
	};

	listener->onCreatePeer = [this](xx::Uv & uv) {
		auto&& p = xx::TryMake<Peer>(uv);
		p->service = this;
		p->catchFish = &*catchFish;
		return p;
	};

	xx::MakeTo(looper, uv, 0, 1, [this] {
		auto currTicks = xx::NowEpoch10m();
		ticksPool += currTicks - lastTicks;
		lastTicks = currTicks;
		while (ticksPool > ticksPerFrame) {
			// game frame loop
			(void)catchFish->Update();
			ticksPool -= ticksPerFrame;
		}
	});
}
