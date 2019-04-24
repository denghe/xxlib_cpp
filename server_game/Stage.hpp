inline int Emitter1::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	return this->BaseType::InitCascade(o);
}

inline int Emitter1::Update(int const& ticks) noexcept {
	if (bornAvaliableTicks == ticks) {
		// 立刻生成小鱼并放入容器
		auto&& fish = scene->MakeRandomFish(++scene->autoIncId);
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


inline int Monitor1::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	return this->BaseType::InitCascade(o);
}

inline int Monitor1::Update(int const& ticks) noexcept {
#ifndef CC_TARGET_PLATFORM
	if (bornAvaliableTicks == ticks) {
		// 扫描已存在的大鱼个数. 当前用 scale >= 5 来判读是否为大鱼
		int count = 0;
		for (auto&& f : *scene->fishs) {
			if (f->scale >= 5) ++count;
		}
		if (count < cfg_numFishsLimit) {
			// 生成大鱼并放入预约容器
			auto&& fb = xx::Make<PKG::CatchFish::FishBorn>();
			fb->fish = scene->MakeRandomFish(--scene->autoDecId);
			fb->fish->scale = 5;
			fb->fish->coin = 20;
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
	}
#endif
	return 0;
}
