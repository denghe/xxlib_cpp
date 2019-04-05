inline int Scene::InitCascade(void* const& o) noexcept {
	// 将 scene 指针刷下去保存在类里面以便于使用
	return this->BaseType::InitCascade(this);
}

inline int Scene::Update(int const&) noexcept {
	// 遍历更新. 倒序扫描, 交换删除. 如果存在内部乱序删除的情况, 则需要 名单机制 或 标记机制 在更新结束之后挨个删掉
	auto&& fs = *this->fishs;
	if (fs.len) {
		for (size_t i = fs.len - 1; i != -1; --i) {
			if (fs[i]->Update(frameNumber)) {
				fs[fs.len - 1]->indexAtContainer = (int)i;
				fs.SwapRemoveAt(i);
			}
		}
	}
	auto&& ps = *this->players;
	if (ps.len) {
		for (size_t i = ps.len - 1; i != -1; --i) {
			if (ps[i].lock()->Update(frameNumber)) {
				ps.SwapRemoveAt(i);
			}
		}
	}
	// todo: foreach  items, ..... call Update

#ifndef CC_TARGET_PLATFORM
	// 存帧序号
	frameEvents->frameNumber = frameNumber;

	// 完整同步数据包( 先不创建 )
	PKG::CatchFish_Client::EnterSuccess_s enterSuccess;

	// 将本帧事件推送给已在线未断线玩家
	for (auto&& plr_w : *players) {
		auto&& plr = xx::As<Player>(plr_w.lock());
		// 只给没断线的发
		if (plr->peer) {
			// 如果是本帧内进入的玩家, 就下发完整同步
			if (frameEnters.Find(&*plr) >= 0) {
				// 如果没创建就创建之
				if (!enterSuccess) {
					xx::MakeTo(enterSuccess);
					xx::MakeTo(enterSuccess->players);
					for (auto&& plr_w : *players) {
						enterSuccess->players->Add(plr_w.lock());
					}
					enterSuccess->scene = shared_from_this();
					enterSuccess->self = plr_w;
				}
				plr->peer->SendPush(enterSuccess);
			}
			// 老玩家直接下发帧事件同步数据
			else {
				// 如果有数据就立即下发, 没有就慢发
				if (frameEvents->events->len || (frameNumber & 0xF == 0)) {
					plr->peer->SendPush(frameEvents);
				}
			}
		}
	}

	// 清除本帧内进入的玩家名册
	frameEnters.Clear();
#endif

	++frameNumber;
	return 0;
};
