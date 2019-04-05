inline void Player::Disconnect() noexcept {
	if (peer) {
		peer->player_w.reset();
		peer->Dispose();
		peer.reset();
	}
	recvShoots.clear();
	recvHits.clear();
}

inline int Player::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;

	// 前置填充
	for (auto&& cannon : *cannons) {
		xx::As<Cannon>(cannon)->player = this;
	}

	// 开始瀑布初始化
	return this->BaseType::InitCascade(o);
}

inline int Player::Update(int const& frameNumber) noexcept {
	auto&& cs = *this->cannons;
	if (cs.len) {
		for (size_t i = cs.len - 1; i != -1; --i) {
			if (cs[i]->Update(frameNumber)) {
				// 这样删确保顺序不变
				cs.RemoveAt(i);
			}
		}
	}

#ifndef CC_TARGET_PLATFORM
	// 处理玩家的发射请求
	while (!recvShoots.empty()) {
		auto&& o = recvShoots.front();

		// 如果收到的包比 server 当前帧还要提前, 就等到那帧再处理
		if (o->frameNumber > frameNumber) break;

		// 倒着扫找炮 id
		bool notFound = true;
		for (size_t i = cannons->len - 1; i != -1; --i) {
			auto&& c = cannons->At(i);
			// 找到了就试着射击. 失败直接踢掉
			if (c->id == o->cannonId) {
				if (!xx::As<Cannon>(c)->Shoot(o)) return -1;
				notFound = false;
				break;
			}
		}
		// 未找到炮台, 直接踢掉
		if (notFound) return -1;

		// todo: 操作成功, 就 ResetTimeoutMS
		recvShoots.pop_front();
	}

	// 处理玩家的发射请求
	while (!recvHits.empty()) {
		auto&& o = recvHits.front();

		// 倒着扫找炮 id
		bool notFound = true;
		for (size_t i = cannons->len - 1; i != -1; --i) {
			auto&& c = cannons->At(i);
			// 找到了就试着检测. 失败直接踢掉
			if (c->id == o->cannonId) {
				if (!xx::As<Cannon>(c)->Hit(o)) return -1;
				notFound = false;
				break;
			}
		}
		// 未找到炮台, 直接踢掉
		if (notFound) return -1;

		// todo: 操作成功, 就 ResetTimeoutMS
		recvHits.pop_front();
	}
#endif

	return 0;
};
