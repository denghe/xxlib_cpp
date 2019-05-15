#ifdef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Player::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;

	// 填充玩家对应的位置的坐标
	this->pos = scene->cfg->sitPositons->At((int)this->sit);

	// 初始面板显示元素
	DrawInit();

	// 前置填充
	for (auto&& cannon : *cannons) {
		cannon->player = this;
	}

	// 开始瀑布初始化
	return InitCascadeCore(o);
}

inline void PKG::CatchFish::Player::DrawInit() noexcept {
	// 根据玩家位置计算金币显示 y 偏移
	float offsetY = (this->sit == PKG::CatchFish::Sits::LeftBottom
		|| this->sit == PKG::CatchFish::Sits::RightBottom) ? 40 : -40;

	labelCoin = cocos2d::Label::createWithSystemFont("", "", 32);
	labelCoin->setPosition(pos + xx::Pos{ 0, offsetY });
	labelCoin->setLocalZOrder(500);			// 确保覆盖在所有炮台之上
	cc_fishNode->addChild(labelCoin);
	DrawUpdate_Coin();						// 更新显示内容
}
inline void PKG::CatchFish::Player::DrawUpdate_Coin() noexcept {
	assert(!::catchFish->disposed);
	if (!labelCoin) return;
	if (coin == lastCoin) return;
	labelCoin->setString(std::to_string(coin));
	lastCoin = coin;
}
#else
inline void PKG::CatchFish::Player::MakeRefundEvent(int64_t const& coin, bool isPersonal) noexcept {
	auto&& refund = xx::Make<PKG::CatchFish::Events::Refund>();
	refund->playerId = id;
	refund->coin = coin;
	refund->isPersonal = isPersonal;
	scene->frameEvents->events->Add(std::move(refund));
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
	DrawUpdate_Coin();

#else
	// 简单包堆积检测
	if (recvs.size() > 200) return Kick("recvs.size() > 200");

	// 是否产生有效操作
	bool success = false;

	// 处理玩家的请求
	while (!recvs.empty()) {
		auto&& msg = recvs.front();
		auto&& typeId = msg->GetTypeId();
		switch (typeId) {
		case xx::TypeId_v<PKG::Client_CatchFish::Bet>: {
			auto&& o = xx::As<PKG::Client_CatchFish::Bet>(msg);

			// 倒着扫找炮 id
			size_t i = cannons->len - 1;
			for (; i != -1; --i) {
				auto&& c = cannons->At(i);
				// 找到了就试着改倍率. 失败直接踢掉
				if (c->id == o->cannonId) {
					if (int r = c->SetCoin(o)) return Kick("Bet failed. r = ", r," pkg = ", o);
					break;
				}
			}
			// 未找到炮台, 直接踢掉
			if (i == -1) return Kick("can't find cannon id = ", o->cannonId, "pkg = ", o);

			//  操作成功, 需要重置超时时间
			success = true;
			break;
		}
		case xx::TypeId_v<PKG::Client_CatchFish::Fire>: {
			auto&& o = xx::As<PKG::Client_CatchFish::Fire>(msg);

			// 如果收到的包比 server 当前帧还要提前, 就等到那帧再处理
			if (o->frameNumber > frameNumber) break;

			// 倒着扫找炮 id
			size_t i = cannons->len - 1;
			for (; i != -1; --i) {
				auto&& c = cannons->At(i);
				// 找到了就试着射击. 失败直接踢掉
				if (c->id == o->cannonId) {
					if (int r = c->Fire(o)) return Kick("Fire failed. r = ", r, "pkg = ", o);
					break;
				}
			}
			// 未找到炮台, 直接踢掉
			if (i == -1) return Kick("can't find cannon id = ", o->cannonId, "pkg = ", o);

			//  操作成功, 需要重置超时时间
			success = true;
			break;
		}
		case xx::TypeId_v<PKG::Client_CatchFish::Hit>: {
			auto&& o = xx::As<PKG::Client_CatchFish::Hit>(msg);

			// 倒着扫找炮 id
			size_t i = cannons->len - 1;
			for (; i != -1; --i) {
				auto&& c = cannons->At(i);
				// 找到了就试着检测. 失败直接踢掉
				if (c->id == o->cannonId) {
					if (int r = c->Hit(o)) return Kick("Hit failed. r = ", r, "pkg = ", o);
					break;
				}
			}
			// 未找到炮台, 直接踢掉
			if (i == -1) return Kick("can't find cannon id = ", o->cannonId, "pkg = ", o);

			//  操作成功, 需要重置超时时间
			success = true;
			break;
		}
		default:
			assert(false);
		}

		// 清掉当前指令
		recvs.pop_front();
	}

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
	recvs.clear();
	if (peer) {
		peer->Dispose(1);
		peer.reset();
	}
	return 0;
}

#endif
