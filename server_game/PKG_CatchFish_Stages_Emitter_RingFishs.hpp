inline int PKG::CatchFish::Stages::Emitter_RingFishs::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	return this->BaseType::InitCascade(o);
}

inline int PKG::CatchFish::Stages::Emitter_RingFishs::Update(int const& ticks) noexcept {
	if (bornAvaliableTicks == ticks) {
		// 立刻生成一圈小鱼并放入容器
		auto&& fishCfg = scene->cfg->fishs->At(0);	// todo: 指定鱼的类型, 根据类型路由创建函数?

		auto&& step = float(M_PI * 2) / cfg_numFishsPerBatch;
		for (float a = 0; a < float(M_PI * 2); a += step) {
			auto&& fish = xx::Make<PKG::CatchFish::Fish>();
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
