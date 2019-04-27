#ifdef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Player::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;

	// 填充玩家对应的位置的坐标
	this->pos = scene->cfg->sitPositons->At((int)this->sit);

	// 初始面板显示元素
	xx::MakeTo(panel, this);

	// 前置填充
	for (auto&& cannon : *cannons) {
		xx::As<Cannon>(cannon)->player = this;
	}

	// 开始瀑布初始化
	return InitCascadeCore(o);
}
#endif

inline int PKG::CatchFish::Player::Update(int const& frameNumber) noexcept {
#ifndef CC_TARGET_PLATFORM
	if (timeoutFrameNumber < scene->frameNumber) return -1;
#endif

	auto&& cs = *this->cannons;
	if (cs.len) {
		for (size_t i = cs.len - 1; i != -1; --i) {
			if (cs[i]->Update(frameNumber)) {
				// 这样删确保顺序不变
				cs.RemoveAt(i);
			}
		}
	}

#ifdef CC_TARGET_PLATFORM
	// 更新金币显示
	panel->SetText_Coin(this->coin);

#else
	// 简单包堆积检测
	if (recvFires.size() > 200) return Kick("recvFires.size() > 200");
	if (recvHits.size() > 200) return Kick("recvHits.size() > 200");

	// 是否产生有效操作
	bool success = false;

	// 处理玩家的发射请求
	while (!recvFires.empty()) {
		auto&& o = recvFires.front();

		// 如果收到的包比 server 当前帧还要提前, 就等到那帧再处理
		if (o->frameNumber > frameNumber) break;

		// 倒着扫找炮 id
		size_t i = cannons->len - 1;
		for (; i != -1; --i) {
			auto&& c = cannons->At(i);
			// 找到了就试着射击. 失败直接踢掉
			if (c->id == o->cannonId) {
				if (int r = xx::As<Cannon>(c)->Fire(o)) return Kick("Fire failed. r = ", r, "pkg = ", o);
				break;
			}
		}
		// 未找到炮台, 直接踢掉
		if (i == -1) return Kick("can't find cannon id = ", o->cannonId, "pkg = ", o);

		//  操作成功, 需要重置超时时间
		success = true;

		// 清掉当前指令
		recvFires.pop_front();
	}

	// 处理玩家的发射请求
	while (!recvHits.empty()) {
		auto&& o = recvHits.front();

		// 倒着扫找炮 id
		size_t i = cannons->len - 1;
		for (; i != -1; --i) {
			auto&& c = cannons->At(i);
			// 找到了就试着检测. 失败直接踢掉
			if (c->id == o->cannonId) {
				if (int r = xx::As<Cannon>(c)->Hit(o)) return Kick("Hit failed. r = ", r, "pkg = ", o);
				break;
			}
		}
		// 未找到炮台, 直接踢掉
		if (i == -1) return Kick("can't find cannon id = ", o->cannonId, "pkg = ", o);

		//  操作成功, 需要重置超时时间
		success = true;

		// 清掉当前指令
		recvHits.pop_front();
	}

	// ... more cmds handle here

	if (success) {
		ResetTimeoutFrameNumber();
	}

#endif

	return 0;
};


#ifndef CC_TARGET_PLATFORM

inline void PKG::CatchFish::Player::ResetTimeoutFrameNumber() noexcept {
	if (peer && !peer->Disposed()) {
		peer->ResetTimeoutMS(10000);
	}
	timeoutFrameNumber = scene->frameNumber + 60 * 20;		// todo: 从配置拿时长
}

template<typename ...Args>
inline int PKG::CatchFish::Player::Kick(Args const& ... reason) noexcept {
	xx::CoutTN("Kick player id = ", id, ", reason = ", reason...);
	recvFires.clear();
	recvHits.clear();
	if (peer) {
		peer->Dispose();
		peer.reset();
	}
	return 0;
}

#endif
