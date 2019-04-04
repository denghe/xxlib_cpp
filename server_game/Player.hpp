inline void Player::Disconnect() noexcept {
	if (peer) {
		peer->player_w.reset();
		peer->Dispose();
		peer.reset();
	}
	recvShoots.Clear();
	recvHits.Clear();
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
				cs[cs.len - 1]->indexAtContainer = (int)i;
				cs.SwapRemoveAt(i);
			}
		}
	}
	return 0;
};
