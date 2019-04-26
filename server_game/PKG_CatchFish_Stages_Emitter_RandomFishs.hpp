inline int PKG::CatchFish::Stages::Emitter_RandomFishs::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	return InitCascadeCore(o);
}

inline int PKG::CatchFish::Stages::Emitter_RandomFishs::Update(int const& ticks) noexcept {
	if (bornAvaliableTicks <= ticks) {
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
