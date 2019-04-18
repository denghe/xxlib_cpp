inline int Dialer::UpdateCore(int const& lineNumber) noexcept {
	COR_BEGIN

		// init
		xx::MakeTo(resolver, uv);
	resolver->OnFinish = [this] {
		ips = std::move(resolver->ips);
		finished = true;
	};
	this->OnAccept = [this](auto peer) {
		finished = true;
	};

	// 初始面板显示元素
	xx::MakeTo(panel, this);


		// begin resolve domain to iplist
		LabResolveDomain:

	// clear flag
	finished = false;

	// resolve
	resolver->Resolve(catchFish->serverIp, 2000);

	// wait resolved or timeout
	while (!finished) {
		COR_YIELD
	}

	// check resolve result. 0 mean timeout
	if (!ips.size()) {
		// todo: show error?
		goto LabResolveDomain;
	}

	// ip list -> connected peer
		LabDial :
	// clear flag
	finished = false;

	// try connect to server
	Dial(ips, catchFish->serverPort, 2000);

	// cleanup context data & displays
	Reset();

	xx::CoutTN("step 1");

	// wait connected or timeout
	while (!finished) {
		COR_YIELD
	}

	// timeout ?
	if (!peer) {
		// todo: sleep? count check? show "open network permission tips" dialog?
		goto LabDial;
	}

	xx::CoutTN("step 2");

	// send enter package
	pkgEnter->playerId = playerId;
	if (r = peer->SendPush(pkgEnter)) {
		// todo: log?
		goto LabDial;
	}

	// wait recv data
	waitMS = xx::NowSteadyEpochMS() + 10000;	// calc timeout
	while (!recvs.size()) {
		COR_YIELD
			if (xx::NowSteadyEpochMS() > waitMS) goto LabDial;
	}

	// first package handle
	if (r = HandleFirstPackage()) {
		// todo: show error?
		goto LabDial;
	}

	xx::CoutTN("step 3");

	// 记录 / 计算收到的 last frame number 用于接收超时判断( 暂定 5 秒 )
	timeoutFrameNumber = ::catchFish->scene->frameNumber + 60 * 10;

	// peer keeper
	while (!peer->Disposed()) {
		// 处理帧同步消息
		if (r = HandlePackagesOrUpdateScene()) {
			xx::CoutTN("redial when HandlePackagesOrUpdateScene() r = ", r);
			goto LabDial;
		}

		// 接收超时就重连
		if (timeoutFrameNumber < ::catchFish->scene->frameNumber) {
			xx::CoutTN("redial when recv timeout");
			++numDialTimes;
			panel->SetText_NumDialTimes(numDialTimes);
			goto LabDial;
		}

		// 处理 ping 请求的回应. 收到正常 ping 包
		if (::catchFish->scene->frameNumber % 30 == 0) {
			pkgPing->ticks = xx::NowSteadyEpochMS();
			peer->SendRequest(pkgPing, [this](xx::Object_s && msg) {
				int64_t ms = -1;
				if (auto&& pong = xx::As<PKG::Generic::Pong>(msg)) {
					ms = xx::NowSteadyEpochMS() - pong->ticks;
				}
				panel->SetText_Ping(ms);
				return 0;
			}, 2000);
			peer->Flush();
		}

		// 显示鱼的数量
		panel->SetText_NumFishs(::catchFish->scene->fishs->len);

		COR_YIELD
	}
	COR_END
}
