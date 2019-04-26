#ifdef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Scene::InitCascade(void* const& o) noexcept {
	// 将 scene 指针刷下去保存在类里面以便于使用
	// 备份，不做 InitCascade
	auto borns = std::move(this->borns);
	// 下刷
	int r = InitCascadeCore(this);
	// 还原
	this->borns = std::move(borns);
	return r;
}
#endif

inline int PKG::CatchFish::Scene::Update() noexcept {
	++frameNumber;

	// 遍历更新. 倒序扫描, 交换删除. 如果存在内部乱序删除的情况, 则需要 名单机制 或 标记机制 在更新结束之后挨个删掉
	auto&& fs = *fishs;
	if (fs.len) {
		for (size_t i = fs.len - 1; i != -1; --i) {
			assert(fs[i]->indexAtContainer == (int)i);
			if (fs[i]->Update(frameNumber)) {
				fs[fs.len - 1]->indexAtContainer = (int)i;
				fs.SwapRemoveAt(i);
			}
		}
	}

	// 倒序遍历 items. Update 返回非 0 则杀掉
	auto&& is = *items;
	if (is.len) {
		for (size_t i = is.len - 1; i != -1; --i) {
			if (int r = is[i]->Update(frameNumber)) {
				is.SwapRemoveAt(i);
			}
		}
	}

	// 如果关卡持续时间到达就循环切换到下一关
	assert(stage);
	assert(stage->ticks <= stage->cfg_endTicks);
	if (stage->ticks == stage->cfg_endTicks) {
		int stageId = stage->cfg_id == cfg->stages->len - 1 ? 0 : stage->cfg_id + 1;
		cfg->stageBufs[stageId].offset = 0;
		int r = cfg->stageBufs[stageId].ReadRoot(stage);
		assert(!r);
		stage->InitCascade(this);
	}

	// 取出关卡 ticks 备用
	auto&& ticks = ++stage->ticks;

	// 倒序遍历发射器. 判断生效时间. Update 返回非 0 则杀掉
	auto&& es = *stage->elements;
	if (es.len) {
		for (size_t i = es.len - 1; i != -1; --i) {
			auto&& e = es[i];
			if (e->cfg_beginTicks <= ticks) {
				if (int r = e->Update(ticks)) {
					es.SwapRemoveAt(i);
				}
			}
		}
	}

#ifndef CC_TARGET_PLATFORM
	// 倒序遍历监视器. 判断生效时间. Update 返回非 0 则杀掉
	auto&& ms = *stage->monitors;
	if (ms.len) {
		for (size_t i = ms.len - 1; i != -1; --i) {
			auto&& m = ms[i];
			if (m->cfg_beginTicks <= ticks) {
				if (int r = m->Update(ticks)) {
					ms.SwapRemoveAt(i);
				}
			}
		}
	}
#endif

	// 倒序遍历预约生成鱼. 时间到达就将鱼移到 fishs 并初始化相关数据
	auto&& bs = *borns;
	if (bs.len) {
		for (size_t i = bs.len - 1; i != -1; --i) {
			auto&& b = bs[i];
			assert(b->beginFrameNumber >= frameNumber);
			if (b->beginFrameNumber == frameNumber) {
				b->fish->indexAtContainer = (int)fs.len;
				fs.Add(b->fish);
#ifdef CC_TARGET_PLATFORM
				b->fish->InitCascade(this);
#endif
				bs.SwapRemoveAt(i);
			}
		}
	}

	// 倒序遍历玩家，Update 返回非 0 则杀掉
	auto&& ps = *players;
	if (ps.len) {
		for (size_t i = ps.len - 1; i != -1; --i) {
			auto w = ps[i];								// 后面 assert 要用. 中途可能被删掉. 不存 && 引用
			int r = 0;
			{
				auto&& p = xx::As<Player>(w.lock());
				if (r = p->Update(frameNumber)) {
					catchFish->Cleanup(p);
				}
			}
			assert(!r || r && !w.lock());
		}
	}


#ifndef CC_TARGET_PLATFORM
	// 存帧序号
	frameEvents->frameNumber = frameNumber;

	// 将本帧事件推送给已在线未断线玩家
	for (auto&& plr_w : *players) {
		auto&& plr = xx::As<Player>(plr_w.lock());
		// 只给没断线的发
		if (plr->peer && !plr->peer->Disposed()) {
			// 如果是本帧内进入的玩家, 就下发完整同步
			if (frameEnters.Find(&*plr) != -1) {
				auto&& enterSuccess = xx::Make<PKG::CatchFish_Client::EnterSuccess>();
				xx::MakeTo(enterSuccess->players);
				for (auto&& plr_w : *players) {
					enterSuccess->players->Add(plr_w.lock());
				}
				enterSuccess->scene = catchFish->scene;		// todo: 如果是多个服务器实例则需要用 key 定位
				enterSuccess->self = plr_w;
				xx::MakeTo(enterSuccess->token, plr->token);

				plr->peer->SendPush(enterSuccess);
				plr->peer->Flush();
			}
			// 老玩家直接下发帧事件同步数据
			else {
				// 如果有数据就立即下发, 没有就慢发
				if (frameEvents->events->len || plr->events->len || !(frameNumber & 0xF)) {
					frameEvents->persionalEvents = plr->events;
					plr->peer->SendPush(frameEvents);
					plr->peer->Flush();
				}
			}
		}
		plr->events->Clear();			// 清除发送过的数据
	}
	frameEvents->events->Clear();		// 清除发送过的数据
	frameEvents->persionalEvents.reset();
	frameEnters.Clear();				// 清除发送过的数据
#endif

	return 0;
};


inline PKG::CatchFish::WayFish_s PKG::CatchFish::Scene::MakeRandomFish(int const& fishId, int64_t const& coin, float const& scaleFrom, float const& scaleTo) noexcept {
	auto&& fishCfg = cfg->fishs->At(0);//rnd->Next((int)cfg->fishs->len));

	auto&& fish = xx::Make<PKG::CatchFish::WayFish>();
	fish->scene = this;
	fish->id = fishId;	// ++autoIncId;
	fish->cfgId = fishCfg->id;
	fish->cfg = &*fishCfg;
	if (coin) {
		fish->coin = coin;
	}
	else {
		if (fishCfg->minCoin < fishCfg->maxCoin) {
			fish->coin = rnd->Next((int)fishCfg->minCoin, (int)fishCfg->maxCoin + 1);
		}
		else {
			fish->coin = fishCfg->minCoin;
		}
	}
	fish->scale = (float)rnd->NextDouble(scaleFrom, scaleTo);
	fish->speedScale = 1 + (float)rnd->NextDouble() * 2;
	fish->wayPointIndex = 0;
	fish->wayPointDistance = 0;
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;
	fish->reverse = false;

	//fish->way = MakeBeeline(MakeRandomInOutPoint(fishCfg->maxDetectRadius * fishCfg->scale));
	fish->wayTypeIndex = 1 + rnd->Next(2);										// 1: 曲线, 2: 直线
	fish->wayIndex = rnd->Next((int)cfg->ways[fish->wayTypeIndex].len);			// 下标

	auto&& p = cfg->ways[fish->wayTypeIndex][fish->wayIndex]->points->At(fish->wayPointIndex);
	fish->pos = p.pos;
	fish->angle = p.angle;
	return fish;
}

// 生成一条随机角度的进出口( 主用于体积大于 cfg ways 设定的移动对象 )
// -45 ~ 45, 135 ~ 225 在这两段角度之间随机一个角度值,  + 180 之后的 45 度范围内再次随机一个角度, 用旋转函数转为两个坐标点. 连为1根直线, 最后找出安全出生框与直线的交点
// 由于最终计算出两个交点之后, 可以通过交换顺序的方式反向, 故只需要一段角度作为起始角度即可. 简化起见, 直接 135 ~ 225 ( 不考虑开区间误差 )
inline std::pair<xx::Pos, xx::Pos> PKG::CatchFish::Scene::MakeRandomInOutPoint(float const& itemRadius) noexcept {
	std::pair<xx::Pos, xx::Pos> rtv;
	auto&& w = (screenWidth + itemRadius) / 2.0f;
	auto && h = (screenHeight + itemRadius) / 2.0f;
	auto && a = rnd->Next(180);
	if (a < 90) {
		a -= 45;
	}
	else {
		a = a - 90 + 135;
	}
	rtv.first = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	xx::Pos abs{ std::fabs(rtv.first.x), std::fabs(rtv.first.y) };
	if (abs.x / (abs.x + abs.y) > screenWidthRatio) {
		rtv.first = rtv.first * (w / abs.x);
	}
	else {
		rtv.first = rtv.first * (h / abs.y);
	}
	a = a + 180 - 23 + rnd->Next(46);
	rtv.second = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	abs.x = std::fabs(rtv.second.x);
	abs.y = std::fabs(rtv.second.y);
	if (abs.x / (abs.x + abs.y) > screenWidthRatio) {
		rtv.second = rtv.second * (w / abs.x);
	}
	else {
		rtv.second = rtv.second * (h / abs.y);
	}
	return rtv;
}

