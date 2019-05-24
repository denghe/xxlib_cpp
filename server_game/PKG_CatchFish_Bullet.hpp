#ifdef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Bullet::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	assert(player);
	assert(cannon);
	assert(cfg);

	if (int r = InitCascadeCore(o)) return r;
	DrawInit();
	return 0;
}
#endif

inline int PKG::CatchFish::Bullet::Move() noexcept {
	if (!cfg->enableBulletBounce) {
#ifdef CC_TARGET_PLATFORM
		// 飞出屏幕就消失
		auto&& w = ::designSize_2.x + cfg->maxRadius;
		auto&& h = ::designSize_2.y + cfg->maxRadius;
		if (pos.x > w || pos.x < -w || pos.y > h || pos.y < -h) {
			// 如果是本人: 发子弹撤销包
			if (player->isSelf) {
				auto&& o = xx::Make<PKG::Client_CatchFish::Hit>();
				o->bulletId = id;
				o->cannonId = cannon->id;
				o->fishId = 0;
				::dialer->peer->SendPush(o);
				::dialer->peer->Flush();
			}
			return -1;
		}
#endif
		pos += moveInc;
	}
	else {
		// 飞出屏幕边缘就反弹
		auto w = ::designSize_2.x - cfg->radius * cfg->scale;
		auto h = ::designSize_2.y - cfg->radius * cfg->scale;
		auto p = pos + moveInc;
		if (p.x > w || p.x < -w) {
			angle = angle > 0 ? M_PI - angle : -angle - M_PI;
			moveInc.x = -moveInc.x;
			pos.x = p.x > 0 ? w - (p.x - pos.x) : -w - (p.x - pos.x);
		}
		else {
			pos.x = p.x;
		}
		if (p.y > h || p.y < -h) {
			angle = -angle;
			moveInc.y = -moveInc.y;
			pos.y = p.y > 0 ? h - (p.y - pos.y) : -h - (p.y - pos.y);
		}
		else {
			pos.y = p.y;
		}
	}
	return 0;
}

inline int PKG::CatchFish::Bullet::Update(int const& frameNumber) noexcept {
	if (int r = Move()) return r;
#ifdef CC_TARGET_PLATFORM
	// 遍历所有鱼
	auto&& fs = *scene->fishs;
	if (fs.len) {
		for (size_t i = fs.len - 1; i != -1; --i) {
			// 命中检查
			if (fs[i]->HitCheck(this)) {
				// 如果是本人: 发命中检查包
				if (player->isSelf) {
					auto&& o = xx::Make<PKG::Client_CatchFish::Hit>();
					o->bulletId = id;
					o->cannonId = cannon->id;
					o->fishId = fs[i]->id;
					::dialer->peer->SendPush(o);
					::dialer->peer->Flush();
				}
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
inline void PKG::CatchFish::Bullet::DrawInit() noexcept {
	assert(!body);
	body = cocos2d::Sprite::create();
	body->setLocalZOrder(cfg->zOrder);
	auto&& sf = cfg->frames->At(1)->spriteFrame;
	body->setSpriteFrame(sf);
	body->setPosition(pos);
	body->setScale(cfg->scale);
	body->setRotation(-angle * (180.0f / float(M_PI)));
	cc_fishNode->addChild(body);
}

inline void PKG::CatchFish::Bullet::DrawUpdate() noexcept {
	assert(body);
	body->setRotation(-angle * (180.0f / float(M_PI)));
	body->setPosition(pos);
}
#endif
