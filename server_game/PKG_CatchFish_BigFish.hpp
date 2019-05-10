inline PKG::CatchFish::Configs::BigFish* PKG::CatchFish::BigFish::Cfg() const noexcept {
	assert(dynamic_cast<PKG::CatchFish::Configs::BigFish*>(this->cfg));
	return (PKG::CatchFish::Configs::BigFish*)this->cfg;
}


inline int PKG::CatchFish::BigFish::Update(int const& frameNumber) noexcept {
	if (int r = this->BaseType::Update(frameNumber)) return r;
	auto&& rf = childs->At(0);
	auto&& rfCfg = childs->At(0)->cfg;
	float fishRadius = Cfg()->maxDetectRadius * Cfg()->scale * scale;
	float rfRadius = rfCfg->maxDetectRadius * rfCfg->scale * rf->scale;
	float rfDistance = fishRadius - rfRadius * 2;
	auto angleInc = Cfg()->childsAngleInc;
	for (auto&& f : *childs) {
		f->tarPos = pos;
		f->tarRadius = rfDistance;
		f->angleInc = angleInc;
		int r = f->Update(frameNumber);
		assert(!r);
	}
	return 0;
}

inline int PKG::CatchFish::BigFish::HitCheck(PKG::CatchFish::Bullet* const& bullet) noexcept {
	for (auto&& f : *childs) {
		if (int r = f->HitCheck(bullet)) return r;
	}
	return this->BaseType::HitCheck(bullet);
}
