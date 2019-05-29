inline int PKG::CatchFish::Stages::Monitor_KeepBigFish::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	xx::MakeTo(counter);
	// todo: 查找当前符合监视条件的鱼, 塞 counter ?
	return InitCascadeCore(o);
}

#ifndef CC_TARGET_PLATFORM
inline PKG::CatchFish::Fish_s PKG::CatchFish::Stages::Monitor_KeepBigFish::MakeFish() noexcept {
	return scene->MakeRandomBigFish(--scene->autoDecId);
}
#endif

inline int PKG::CatchFish::Stages::Monitor_KeepBigFish::Update(int const& ticks) noexcept {
#ifndef CC_TARGET_PLATFORM
	// 如果鱼条数小于限定 且 生成 cd 到了 就补鱼
	if (counter.use_count() - 1 < cfg_numFishsLimit && bornAvaliableTicks <= ticks) {
		// 生成鱼并放入预约容器
		auto&& f = MakeFish();
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
