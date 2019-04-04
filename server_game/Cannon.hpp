inline int Cannon::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	assert(player);
	assert(!cfg);
	cfg = &*scene->cfg->cannons->At(cfgId);
	pos = scene->cfg->sitPositons->At((int)player->sit);

	// 前置填充
	for (auto&& bullet : *bullets) {
		auto&& b = xx::As<Bullet>(bullet);
		b->player = player;
		b->cannon = this;
	}

	if (int r = this->BaseType::InitCascade(o)) return r;
	DrawInit();
	return 0;
}

inline int Cannon::Update(int const& frameNumber) noexcept {
	if (this->indexAtContainer) {
		// todo: 隐藏( 多个炮台, 只显示最上面的且只有最上面的能被操控 )
		return 0;
	}
	else {
		// todo: 显示
	}

	// todo: 根据收到的数据来发射. 在收到 Shoot 时根据炮台投递到炮台的队列? 在此处扫到并发射? 如果快于当前帧, 

#ifdef CC_TARGET_PLATFORM
	// 输入检测. 炮台角度对准首个 touch 点( 暂定方案 )
	if (cc_touchs.len) {
		// 世界坐标
		auto tloc = cc_touchs[0]->getLocationInView();

		// 转为游戏坐标系
		xx::Pos tpos{ tloc.x - ScreenCenter.x, ScreenCenter.y - tloc.y };

		// 算出朝向角度
		angle = xx::GetAngle(pos, tpos) * (180.0f / float(M_PI));

		// 试着发射
		if (Shoot(frameNumber)) {
			// todo: 开始播放炮台开火特效, 音效
		}
	}
#endif

	DrawUpdate();

	// 驱动子弹
	auto&& bs = *this->bullets;
	if (bs.len) {
		for (size_t i = bs.len - 1; i != -1; --i) {
			if (bs[i]->Update(frameNumber)) {
				bs[bs.len - 1]->indexAtContainer = (int)i;
				bs.SwapRemoveAt(i);
			}
		}
	}
	return 0;
};

inline bool Cannon::Shoot(int const& frameNumber) noexcept {
	// todo: 更多发射限制检测
	if (!quantity) return false;									// 剩余颗数为 0
	if (frameNumber < shootCD) return false;						// CD 中
	if (bullets->len == cfg->numLimit) return false;				// 总颗数限制
	
	// 置 cd
	shootCD = frameNumber + cfg->shootCD;

	// 剩余颗数 -1
	if (quantity != -1) {
		--quantity;
	}

	auto&& bullet = xx::Make<Bullet>();
	bullet->indexAtContainer = (int)bullets->len;
	bullets->Add(bullet);
	bullet->player = player;
	bullet->cannon = this;
	bullet->cfg = cfg;
	bullet->InitCascade(scene);										// 进一步初始化

	return true;
}

inline Cannon::~Cannon() {
	DrawDispose();
}

#pragma region

inline void Cannon::DrawInit() noexcept {
#ifdef CC_TARGET_PLATFORM
	assert(!body);
	body = cocos2d::Sprite::create();
	body->retain();
	body->setGlobalZOrder(cfg->zOrder);
	auto&& sf = xx::As<SpriteFrame>(cfg->frames->At(0))->spriteFrame;
	body->setSpriteFrame(sf);
	body->setPosition(pos);
	body->setScale(cfg->scale);
	body->setRotation(-angle);
	cc_scene->addChild(body);
#endif
}

inline void Cannon::DrawUpdate() noexcept {
#ifdef CC_TARGET_PLATFORM
	assert(body);
	body->setRotation(-angle);
#endif
}

inline void Cannon::DrawDispose() noexcept {
#ifdef CC_TARGET_PLATFORM
	if (!body) return;

	if (body->getParent()) {
		body->removeFromParent();
	}
	body->release();
	body = nullptr;
#endif
}

#pragma endregion
