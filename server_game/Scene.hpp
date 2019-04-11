inline int Scene::InitCascade(void* const& o) noexcept {
	// 将 scene 指针刷下去保存在类里面以便于使用
	return this->BaseType::InitCascade(this);
}

inline int Scene::Update(int const&) noexcept {
	++frameNumber;

	// 遍历更新. 倒序扫描, 交换删除. 如果存在内部乱序删除的情况, 则需要 名单机制 或 标记机制 在更新结束之后挨个删掉
	auto&& fs = *this->fishs;
	if (fs.len) {
		for (size_t i = fs.len - 1; i != -1; --i) {
			assert(fs[i]->indexAtContainer == (int)i);
			if (fs[i]->Update(frameNumber)) {
				fs[fs.len - 1]->indexAtContainer = (int)i;
				fs.SwapRemoveAt(i);
			}
		}
	}
	auto&& ps = *this->players;
	if (ps.len) {
		for (size_t i = ps.len - 1; i != -1; --i) {
			auto&& p = xx::As<Player>(ps[i].lock());
			if (p->Update(frameNumber)) {
				ps.SwapRemoveAt(i);
				catchFish->players.Remove(p);
			}
		}
	}
	// todo: foreach  items, ..... call Update

	// 模拟关卡 鱼发生器. 每 xx 帧生成一条
	if (frameNumber % 64 == 0) {
		MakeRandomFish();
	}

#ifndef CC_TARGET_PLATFORM
	// 存帧序号
	frameEvents->frameNumber = frameNumber;

	//// 下发调试信息
	//{
	//	auto&& di = xx::Make<PKG::CatchFish::Events::DebugInfo>();
	//	xx::MakeTo(di->fishIds);
	//	for (auto&& f : fs) {
	//		di->fishIds->Add(f->id);
	//	}
	//	frameEvents->events->Add(std::move(di));
	//}

	// 完整同步数据包( 先不创建 )
	PKG::CatchFish_Client::EnterSuccess_s enterSuccess;

	// 将本帧事件推送给已在线未断线玩家
	for (auto&& plr_w : *players) {
		auto&& plr = xx::As<Player>(plr_w.lock());
		// 只给没断线的发
		if (plr->peer && !plr->peer->Disposed()) {
			// 如果是本帧内进入的玩家, 就下发完整同步
			if (frameEnters.Find(&*plr) != -1) {
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
				plr->peer->Flush();
			}
			// 老玩家直接下发帧事件同步数据
			else {
				// 如果有数据就立即下发, 没有就慢发
				if (frameEvents->events->len || !(frameNumber & 0xF)) {
					plr->peer->SendPush(frameEvents);
					plr->peer->Flush();
					//xx::CoutN(frameEvents->events->len);
				}
			}
		}
	}

	// 清除发送过的数据
	frameEnters.Clear();
	frameEvents->events->Clear();
#endif

	//// 调试: 存储 fishIds
	//{
	//	xx::List<int> fishIds;
	//	for (auto&& f : fs) {
	//		fishIds.Add(f->id);
	//	}
	//	fishIdss[frameNumber] = std::move(fishIds);
	//	fishIdss.erase(frameNumber - 120);
	//}

	return 0;
};


inline void Scene::MakeRandomFish() noexcept {
	auto&& fishCfg = cfg->fishs->At(rnd->Next((int)cfg->fishs->len));

	auto&& fish = xx::Make<Fish>();
	fish->scene = this;
	fish->id = ++autoIncId;
	fish->cfgId = fishCfg->id;
	fish->cfg = &*fishCfg;
	if (fishCfg->minCoin < fishCfg->maxCoin) {
		fish->coin = rnd->Next((int)fishCfg->minCoin, (int)fishCfg->maxCoin + 1);
	}
	else {
		fish->coin = fishCfg->minCoin;			
	}
	fish->speedScale = 1;
	fish->scale = 1;
	fish->wayIndex = 0;
	fish->wayPointIndex = 0;
	fish->wayPointDistance = 0;
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;
	fish->reverse = false;
	fish->way = MakeBeeline(fishCfg->maxDetectRadius * fishCfg->scale);		// 先随便生成一条轨迹

	auto&& p = fish->way->points->At(fish->wayPointIndex);
	fish->pos = p.pos;
	fish->angle = p.angle;

#ifdef CC_TARGET_PLATFORM
	fish->DrawInit();
#endif
	fish->indexAtContainer = (int)fishs->len;
	fishs->Add(std::move(fish));
}

// 填充随机生成的有限角度的能立即出现在屏幕上的线段路径
// -45 ~ 45, 135 ~ 225 在这两段角度之间随机一个角度值,  + 180 之后的 45 度范围内再次随机一个角度, 用旋转函数转为两个坐标点. 连为1根直线, 最后找出安全出生框与直线的交点
// 由于最终计算出两个交点之后, 可以通过交换顺序的方式反向, 故只需要一段角度作为起始角度即可. 简化起见, 直接 135 ~ 225 ( 不考虑开区间误差 )
inline PKG::CatchFish::Way_s Scene::MakeBeeline(float const& itemRadius) noexcept {
	auto&& w = (ScreenWidth + itemRadius) / 2.0f;
	auto && h = (ScreenHeight + itemRadius) / 2.0f;
	auto && a = rnd->Next(180);
	if (a < 90) {
		a -= 45;
	}
	else {
		a = a - 90 + 135;
	}
	auto&& p1 = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	xx::Pos abs{ std::fabs(p1.x), std::fabs(p1.y) };
	if (abs.x / (abs.x + abs.y) > ScreenWidthRatio) {
		p1 = p1 * (w / abs.x);
	}
	else {
		p1 = p1 * (h / abs.y);
	}
	a = a + 180 - 23 + rnd->Next(46);
	auto&& p2 = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	abs.x = std::fabs(p2.x);
	abs.y = std::fabs(p2.y);
	if (abs.x / (abs.x + abs.y) > ScreenWidthRatio) {
		p2 = p2 * (w / abs.x);
	}
	else {
		p2 = p2 * (h / abs.y);
	}
	auto && way = xx::Make<PKG::CatchFish::Way>();
	xx::MakeTo(way->points);
	way->points->Add(PKG::CatchFish::WayPoint{ p1, xx::GetAngle(p1, p2), p1.distance(p2) });
	way->points->Add(PKG::CatchFish::WayPoint{ p2, 0, 0 });	// 非循环轨迹最后个点距离和角度不用计算, 也不做统计
	way->distance = way->points->At(0).distance;
	way->loop = false;
	return way;
}
