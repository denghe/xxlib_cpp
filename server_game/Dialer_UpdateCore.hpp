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
LabDial:
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
	if (token.size()) {
		xx::MakeTo(pkgEnter->token, token);
	}
	else {
		pkgEnter->token.reset();
	}
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
	timeoutFrameNumber = ::catchFish->scene->frameNumber + 60 * 5;

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
				// 防止 catchFish 析构造成的 msg 为空
				if (!msg && !::catchFish->disposed) {
					panel->SetText_Ping(-1);
				}
				else {
					if (auto && pong = xx::As<PKG::Generic::Pong>(msg)) {
						panel->SetText_Ping(xx::NowSteadyEpochMS() - pong->ticks);
					}
				}
				return 0;
				}, 2000);
			peer->Flush();
		}

		// 显示鱼的数量
		panel->SetText_NumFishs(::catchFish->scene->fishs->len);

		COR_YIELD
	}

	xx::CoutTN("redial by user. redial after 2 secs");
	waitMS = xx::NowSteadyEpochMS() + 2000;	// 等 2 秒再重连
	while (xx::NowSteadyEpochMS() < waitMS) {
		COR_YIELD
	}
	goto LabDial;
	COR_END
}
