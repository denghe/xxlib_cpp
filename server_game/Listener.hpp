inline Service::Service() {
	// 创建游戏上下文并加载配置文件( 多个相同游戏上下文可共享同一配置文件 )
	catchFish = xx::Make<CatchFish>();
	if (int r = catchFish->Init("cfg.bin")) throw r;

	// tcp, kcp 同时监听同一端口
	listener = xx::Make<xx::UvListener>(uv, "0.0.0.0", 12345);

	// 为连接创建上下文对象并附加到连接. 同步生命周期
	listener->onAccept = [this](xx::UvPeer_s peer) {
		xx::CoutTN(peer->GetIP(), peer->IsKcp() ? " kcp" : " tcp", " accepted.");
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
			// ensure calcPeer connect to Calc server
			Dial_Calc();

			// game frame loop
			(void)catchFish->Update();
			ticksPool -= ticksPerFrame;
		}
		});
}


inline bool Service::IsAlive_CalcPeer() {
	return calcPeer && !calcPeer->Disposed();
}

inline void Service::Dial_Calc() {
	if (dialing || IsAlive_CalcPeer()) return;
	if (!calcDialer) {
		xx::TryMakeTo(calcDialer, uv);
		if (!calcDialer) {
			xx::CoutTN("make calcDialer failed.");
			return;
		}
		calcDialer->onAccept = [this](xx::UvPeer_s p) {
			dialing = false;
			if (!p) {
				xx::CoutTN("dial to Calc timeout.");
				return;
			}
			xx::CoutTN("connected: ", p->GetIP());
			p->onDisconnect = [p] {
				xx::CoutTN("disconnect: ", p->GetIP());
			};
			assert(!IsAlive_CalcPeer());
			calcPeer = std::move(p);
		};
	}
	if (int r = calcDialer->Dial("127.0.0.1", 12333, 200)) {			// todo: 读配置 / 启动参数
		xx::CoutTN("calcDialer->Dial(\"127.0.0.1\", 12333) failed. r = ", r);
	}
	else {
		dialing = true;
	}
}
