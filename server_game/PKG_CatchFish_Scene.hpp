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
#ifndef CC_TARGET_PLATFORM
	// 如果到计算服务的连接未就绪, 无限等待			// todo: 超时处理
	if (!service->IsAlive_CalcPeer()) {
		xx::CoutTN("waiting Calc service...");
		return 0;
	}

	// 如果正在等待计算服回调, 就直接返回. ( 正常情况下不该发生. 除非与 Calc 连接异常 )
	if (calcResult) return calcResult > 0 ? 0 : calcResult;
#endif

	// 一开始就累加帧数, 确保后续步骤( 含追帧递归 )生命周期正确
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
			if (e->cfg_endTicks <= ticks) {
				es.SwapRemoveAt(i);
				continue;
			}
			if (e->cfg_beginTicks <= ticks) {
				if (int r = e->Update(ticks)) {
					es.SwapRemoveAt(i);
					continue;
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
			if (m->cfg_endTicks <= ticks) {
				ms.SwapRemoveAt(i);
				continue;
			}
			if (m->cfg_beginTicks <= ticks) {
				if (int r = m->Update(ticks)) {
					ms.SwapRemoveAt(i);
					continue;
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

	// 倒序遍历玩家，Update 返回非 0 则杀掉( 玩家会进一步驱动 cannons, bullets )
	auto&& ps = *players;
	if (ps.len) {
		for (size_t i = ps.len - 1; i != -1; --i) {
			auto w = ps[i];								// 后面 assert 要用. 中途可能被删掉. 不存 && 引用
			int r = 0;
			{
				auto&& p = xx::As<Player>(w.lock());
				if ((r = p->Update(frameNumber))) {
					catchFish->Cleanup(p);
				}
			}
			assert(!r || r && !w.lock());
		}
	}

#ifndef CC_TARGET_PLATFORM

	if (hitChecks->hits->len) {
		// 设置标志位
		calcResult = 1;

		// 将 hitChecks 发给 Calc 计算
		service->calcPeer->SendRequest(hitChecks, [this](xx::Object_s && msg)->int { return UpdateCalc(std::move(msg)); }, 1000);

		// 清理 for next fill
		hitChecks->hits->Clear();
	}
	else {
		// 直接做后续 Update
		calcResult = Handle(PKG::Calc_CatchFish::HitCheckResult_s());
	}
#endif

	return 0;
};

#ifndef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Scene::UpdateCalc(xx::Object_s&& msg) noexcept {
	// 超时检查
	if (!msg) {
		calcResult = -1;
		return 0;
	}

	switch (msg->GetTypeId()) {
	case xx::TypeId_v<PKG::Calc_CatchFish::HitCheckResult>: {
		calcResult = Handle(xx::As<PKG::Calc_CatchFish::HitCheckResult>(msg));
		break;
	}
	case xx::TypeId_v<PKG::Generic::Error>: {
		xx::CoutTN("recv error: ", msg);
		calcResult = -2;
		break;
	}
	default:
		xx::CoutTN("recv unhandled msg: ", msg);
		calcResult = -3;
		break;
	}
	return 0;
}

inline int PKG::CatchFish::Scene::Handle(PKG::Calc_CatchFish::HitCheckResult_s&& msg) noexcept {
	if (msg) {
#if ENABLE_CALC_SERVICE
		// 令相应的鱼死掉( 子弹在 hit 请求产生时便已被移除 ), 同步玩家 coin, 生成各种 鱼死 & 退款 事件

		for (auto&& f : *msg->fishs) {
			// 放入待删除列表
			fishIds.Add(f.fishId);

			// 算钱
			auto&& c = f.fishCoin * f.bulletCoin;

			// 构造鱼死事件包
			{
				auto&& fishDead = xx::Make<PKG::CatchFish::Events::FishDead>();
				fishDead->bulletId = f.bulletId;
				fishDead->coin = c;
				fishDead->fishId = f.fishId;
				fishDead->playerId = f.playerId;
				frameEvents->events->Add(std::move(fishDead));
			}

			// 定位到玩家加钱( 这部分代码容忍 player 在 Calc 回调后找不到 )
			for (auto&& p : *players) {
				auto&& player = p.lock();
				assert(player);
				if (player->id == f.playerId) {
					player->coin += c;
				}
			}
		}

		// 批量删鱼 by fishIds
		auto&& fs = *fishs;
		if (fs.len && fishIds.len) {
			for (size_t j = fs.len - 1; j != -1; --j) {
				auto&& f = fs[j];
				assert(f->indexAtContainer == j);
				// 如果 鱼id 存在于 fishIds
				if (auto && idx = fishIds.Find(f->id); idx != -1) {
					// 删鱼id
					fishIds.SwapRemoveAt(idx);
					// 删鱼
					fs[fs.len - 1]->indexAtContainer = (int)j;
					fs.SwapRemoveAt(j);
				}
				// 没鱼删了 直接退出
				if (!fishIds.len) break;
			}
		}

		// cleanup
		fishIds.Clear();

		for (auto&& b : *msg->bullets) {
			// 定位到玩家退钱 & 生成退钱事件包
			for (auto&& p : *players) {
				auto&& player = p.lock();
				assert(player);
				if (player->id == b.playerId) {
					auto&& c = b.bulletCoin * b.bulletCount;
					player->coin += c;
					player->MakeRefundEvent(c);
				}
			}
		}

		calcResult = 0;
	}
#endif

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
				if (frameEvents->events->len || !(frameNumber & 0xF)) {
					plr->peer->SendPush(frameEvents);
					plr->peer->Flush();
				}
			}
		}
	}
	frameEvents->events->Clear();		// 清除发送过的数据
	frameEnters.Clear();				// 清除发送过的数据

	return 0;
}
#endif










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
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;

	fish->reverse = false;
	fish->wayPointIndex = 0;
	fish->wayPointDistance = 0;
	//fish->way = MakeBeeline(MakeRandomInOutPoint(fishCfg->maxDetectRadius * fishCfg->scale));
	fish->wayTypeIndex = 1 + rnd->Next(2);										// 1: 曲线, 2: 直线
	fish->wayIndex = rnd->Next((int)cfg->ways[fish->wayTypeIndex].len);			// 下标

	auto&& p = cfg->ways[fish->wayTypeIndex][fish->wayIndex]->points->At(fish->wayPointIndex);
	fish->pos = p.pos;
	fish->angle = p.angle;
	return fish;
}



inline PKG::CatchFish::BigFish_s PKG::CatchFish::Scene::MakeRandomBigFish(int const& fishId) noexcept {
	auto&& fishCfg = xx::As<PKG::CatchFish::Configs::BigFish>(cfg->fishs->At(1));
	assert(fishCfg);

	auto&& fish = xx::Make<PKG::CatchFish::BigFish>();
	fish->scene = this;
	fish->id = fishId;	// ++autoIncId;
	fish->cfgId = fishCfg->id;
	fish->cfg = &*fishCfg;
	if (fishCfg->minCoin < fishCfg->maxCoin) {
		fish->coin = rnd->Next((int)fishCfg->minCoin, (int)fishCfg->maxCoin + 1);
	}
	else {
		fish->coin = fishCfg->minCoin;
	}
	fish->scale = 1;
	fish->speedScale = 1;
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;

	auto&& rfCfg = cfg->fishs->At(0);
	float rfScale = 1;
	float fishRadius = fishCfg->maxDetectRadius * fishCfg->scale * fish->scale;
	float rfRadius = rfCfg->maxDetectRadius * rfCfg->scale * rfScale;
	float rfDistance = fishRadius - rfRadius * 2;

	auto&& posFromTo = MakeRandomInOutPoint(rfDistance);
	fish->angle = xx::GetAngle(posFromTo);
	fish->pos = posFromTo.first;
	fish->moveInc = xx::Rotate({ fishCfg->moveFrameDistance, 0 }, fish->angle);

	xx::MakeTo(fish->childs);
	float aStep = (float)(M_PI * 2 / fishCfg->numChilds);
	for (auto&& i = 0; i < fishCfg->numChilds; ++i) {
		auto&& rf = xx::Make<PKG::CatchFish::RoundFish>();
		rf->scene = this;
		//rf->id;
		rf->cfgId = rfCfg->id;
		rf->cfg = &*rfCfg;
		//rf->coin;
		rf->scale = rfScale;
		rf->speedScale = 1;
		rf->spriteFrameIndex = 0;
		rf->frameRatio = 1;

		rf->tarAngle = aStep * (float)i;
		rf->pos = fish->pos + xx::Rotate({ rfDistance, 0 }, rf->tarAngle);
		rf->angle = rf->tarAngle + (fishCfg->childsAngleInc > 0 ? (float)M_PI_2 : -(float)M_PI_2);
		//rf->moveInc

		fish->childs->Add(std::move(rf));
	}

	return fish;
}





// 生成一条随机角度的进出口( 主用于体积大于 cfg ways 设定的移动对象 )
// -45 ~ 45, 135 ~ 225 在这两段角度之间随机一个角度值,  + 180 之后的 45 度范围内再次随机一个角度, 用旋转函数转为两个坐标点. 连为1根直线, 最后找出安全出生框与直线的交点
// 由于最终计算出两个交点之后, 可以通过交换顺序的方式反向, 故只需要一段角度作为起始角度即可. 简化起见, 直接 135 ~ 225 ( 不考虑开区间误差 )
inline std::pair<xx::Pos, xx::Pos> PKG::CatchFish::Scene::MakeRandomInOutPoint(float const& itemRadius) noexcept {
	std::pair<xx::Pos, xx::Pos> rtv;
	auto&& w = designSize_2.x + itemRadius;
	auto&& h = designSize_2.y + itemRadius;
	auto&& a = rnd->Next(180);
	if (a < 90) {
		a -= 45;
	}
	else {
		a = a - 90 + 135;
	}
	rtv.first = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	xx::Pos abs{ std::fabs(rtv.first.x), std::fabs(rtv.first.y) };
	if (abs.x / (abs.x + abs.y) > designWidthRatio) {
		rtv.first = rtv.first * (w / abs.x);
	}
	else {
		rtv.first = rtv.first * (h / abs.y);
	}
	a = a + 180 - 23 + rnd->Next(46);
	rtv.second = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	abs.x = std::fabs(rtv.second.x);
	abs.y = std::fabs(rtv.second.y);
	if (abs.x / (abs.x + abs.y) > designWidthRatio) {
		rtv.second = rtv.second * (w / abs.x);
	}
	else {
		rtv.second = rtv.second * (h / abs.y);
	}
	return rtv;
}

