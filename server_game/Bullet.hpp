inline int Bullet::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	assert(player);
	assert(cannon);
	assert(cfg);

	if (int r = this->BaseType::InitCascade(o)) return r;
#ifdef CC_TARGET_PLATFORM
	DrawInit();
#endif
	return 0;
}

inline int Bullet::Move() noexcept {
	pos += moveInc;

	// 飞出屏幕就消失
	auto&& w = ::ScreenCenter.x + cfg->maxRadius;
	auto&& h = ::ScreenCenter.y + cfg->maxRadius;
	if (pos.x > w || pos.x < -w || pos.y > h || pos.y < -h) {
#ifndef CC_TARGET_PLATFORM
		// 退钱
		auto&& refund = xx::Make<PKG::CatchFish::Events::Refund>();
		refund->coin = coin;
		refund->playerId = player->id;
		//xx::CoutN("bullet fly out the screen. ", refund);
		scene->frameEvents->events->Add(std::move(refund));
#endif
		return -1;
	}
	return 0;
}

inline int Bullet::Update(int const& frameNumber) noexcept {
	if (int r = Move()) return r;
#ifdef CC_TARGET_PLATFORM
	// 遍历所有鱼
	auto&& fs = *scene->fishs;
	if (fs.len) {
		for (size_t i = fs.len - 1; i != -1; --i) {
			// 命中检查
			if (xx::As<Fish>(fs[i])->HitCheck(this)) {
				// 发命中检查包
				auto&& o = xx::Make<PKG::Client_CatchFish::Hit>();
				o->bulletId = id;
				o->cannonId = cannon->id;
				o->fishId = fs[i]->id;
				::dialer->peer->SendPush(o);
				::dialer->peer->Flush();
				// todo: 播放子弹爆炸特效?
				return -1;
			}
		}
	}
	DrawUpdate();
#endif
	return 0;
};

#ifdef CC_TARGET_PLATFORM
inline void Bullet::DrawInit() noexcept {
	assert(!body);
	body = cocos2d::Sprite::create();
	body->setGlobalZOrder(cfg->zOrder);
	auto&& sf = xx::As<SpriteFrame>(cfg->frames->At(1))->spriteFrame;
	body->setSpriteFrame(sf);
	body->setPosition(pos);
	body->setScale(cfg->scale);
	body->setRotation(-angle * (180.0f / float(M_PI)));
	cc_scene->addChild(body);
}

inline void Bullet::DrawUpdate() noexcept {
	assert(body);
	body->setRotation(-angle * (180.0f / float(M_PI)));
	body->setPosition(pos);
}
#endif
