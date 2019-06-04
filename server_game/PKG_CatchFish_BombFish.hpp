#ifndef CC_TARGET_PLATFORM

inline PKG::CatchFish::Configs::ColorFish* PKG::CatchFish::BombFish::Cfg() const noexcept {
	assert(dynamic_cast<PKG::CatchFish::Configs::ColorFish*>(this->cfg));
	return (PKG::CatchFish::Configs::ColorFish*)this->cfg;
}

inline int PKG::CatchFish::BombFish::Die(PKG::CatchFish::Bullet* const& b) noexcept {
	// 创建 weapon 放入 b->player->weapons
	auto&& wc = Cfg()->weapon;
	auto&& p = b->player;
	auto&& w = xx::Make<PKG::CatchFish::BombWeapon>();
	w->cfgId = wc->id;
	w->cfg = &*wc;
	w->scene = scene;
	w->player = p;
	w->beginFrameNumber = scene->frameNumber + wc->showNumFrames;
	w->coin = b->coin;
	w->id = id;
	w->pos = pos;
	w->indexAtContainer = p->weapons->len;
	p->weapons->Add(w);

	// event PushWeapon
	auto&& pw = xx::Make<PKG::CatchFish::Events::PushWeapon>();
	pw->playerId = p->id;
	pw->weapon = w;
	scene->frameEvents->events->Add(pw);

	return 0;
}

#endif
