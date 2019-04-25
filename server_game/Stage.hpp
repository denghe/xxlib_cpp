inline int Emitter_RandomFishs::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	return this->BaseType::InitCascade(o);
}

inline int Emitter_RandomFishs::Update(int const& ticks) noexcept {
	if (bornAvaliableTicks == ticks) {
		// 立刻生成小鱼并放入容器
		auto&& fish = scene->MakeRandomFish(++scene->autoIncId, cfg_coin, cfg_scaleFrom, cfg_scaleTo);
		fish->indexAtContainer = (int)scene->fishs->len;
		scene->fishs->Add(fish);
#ifdef CC_TARGET_PLATFORM
		fish->DrawInit();
#endif
		// 计算下次可生成鱼的时间点
		bornAvaliableTicks = ticks + cfg_bornTicksInterval;
	}
	return 0;
}


inline int Monitor_KeepBigFish::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	xx::MakeTo(counter);
	return this->BaseType::InitCascade(o);
}

inline int Monitor_KeepBigFish::Update(int const& ticks) noexcept {
#ifndef CC_TARGET_PLATFORM
	// 如果大鱼条数小于限定 且 生成 cd 到了 就补鱼
	if (counter.use_count() - 1 < cfg_numFishsLimit && bornAvaliableTicks <= ticks) {
		// 生成大鱼并放入预约容器
		auto&& f = scene->MakeRandomFish(--scene->autoDecId, cfg_coin, cfg_scaleFrom, cfg_scaleTo);
		f->counters.Add(counter);
		auto&& fb = xx::Make<PKG::CatchFish::FishBorn>();
		fb->fish = f;
		fb->beginFrameNumber = scene->frameNumber + cfg_bornDelayFrameNumber;
		scene->borns->Add(fb);

		// 生成同步事件
		{
			auto&& pf = xx::Make<PKG::CatchFish::Events::PushFish>();
			pf->born = fb;
			scene->frameEvents->events->Add(std::move(pf));
		}

		bornAvaliableTicks = ticks + cfg_bornTicksInterval;
	}
#endif
	return 0;
}


inline int Emitter_RingFishs::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	return this->BaseType::InitCascade(o);
}

inline int Emitter_RingFishs::Update(int const& ticks) noexcept {
	if (bornAvaliableTicks == ticks) {
		// 立刻生成一圈小鱼并放入容器
		auto&& fishCfg = scene->cfg->fishs->At(0);	// todo: 指定鱼的类型, 根据类型路由创建函数?

		auto&& step = float(M_PI * 2) / cfg_numFishsPerBatch;
		for (float a = 0; a < float(M_PI * 2); a += step) {
			auto&& fish = xx::Make<Fish>();
			fish->scene = scene;
			fish->id = ++scene->autoIncId;
			fish->cfgId = fishCfg->id;
			fish->cfg = &*fishCfg;
			fish->coin = cfg_coin;
			fish->scale = cfg_scale;
			fish->speedScale = 1;
			fish->spriteFrameIndex = 0;
			fish->pos = { 0,0 };
			fish->angle = a;
			fish->moveInc = xx::Rotate({ cfg_speed ,0 }, a);

			fish->indexAtContainer = (int)scene->fishs->len;
			scene->fishs->Add(fish);
#ifdef CC_TARGET_PLATFORM
			fish->DrawInit();
#endif
		}

		// 计算下次可生成鱼的时间点
		bornAvaliableTicks = ticks + cfg_bornTicksInterval;
	}
	return 0;
}
