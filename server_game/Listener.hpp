inline Service::Service() {
	// 创建游戏上下文并加载配置文件( 多个相同游戏上下文可共享同一配置文件 )
	catchFish = xx::Make<CatchFish>();
	if (int r = catchFish->Init("cfg.bin")) throw r;

	// tcp, kcp 同时监听同一端口
	tcpListener = xx::Make<xx::UvTcpListener<>>(uv, "0.0.0.0", 12345);
	kcpListener = xx::Make<xx::UvKcpListener<>>(uv, "0.0.0.0", 12345);

	// 为连接创建上下文对象并附加到连接. 同步生命周期
	tcpListener->OnAccept([this](xx::IUvPeer_s peer) { new PeerContext(peer, this); });
	kcpListener->OnAccept([this](xx::IUvPeer_s peer) { new PeerContext(peer, this); });

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
