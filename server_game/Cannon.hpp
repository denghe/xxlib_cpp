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
#ifdef CC_TARGET_PLATFORM
	DrawInit();
#endif
	return 0;
}

inline int Cannon::Update(int const& frameNumber) noexcept {

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

#ifdef CC_TARGET_PLATFORM
	// 判断这是不是最上层炮台
	if (this != &*player->cannons->Top()) {
		// todo: 隐藏( 多个炮台, 只显示最上面的且只有最上面的能被操控 )
		return 0;
	}
	else {
		// todo: 显示
	}

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
	DrawUpdate();
#endif

	return 0;
};

#ifndef CC_TARGET_PLATFORM
inline void Cannon::Hit(PKG::Client_CatchFish::Hit_s& o) noexcept {
	// 合法性判断: 如果 bulletId, fishId 找不到就忽略
	for (auto&& bullet : *bullets) {
		if (bullet->id == o->bulletId) {
			for (auto&& fish : *scene->fishs) {
				//if (fish->isDead) continue;
				if (fish->id == o->fishId) {
					// 是否能打死的计算. 先根据 coin 来计算死亡比例
					if (scene->serverRnd.Next((int)fish->coin) == 0) {
						auto&& fishDead = xx::Make<PKG::CatchFish::Events::FishDead>();
						fishDead->bulletId = bullet->id;
						fishDead->coin = bullet->coin * fish->coin;
						fishDead->fishId = fish->id;
						fishDead->id = player->id;
						scene->frameEvents->events->Add(std::move(fishDead));
					}
					return;
				}
			}
			return;
		}
	}
}
#endif


#ifndef CC_TARGET_PLATFORM
inline bool Cannon::Shoot(PKG::Client_CatchFish::Shoot_s& o) noexcept {
	// 如果金币不足, 失败
	if (player->coin >= coin) return false;

	// 如果子弹编号已存在, 失败
	for (auto&& bullet : *bullets) {
		if (bullet->id == o->bulletId) return false;
	}

	// 根据开火目标坐标计算角度
	angle = xx::GetAngle(pos, o->pos);

	// 模拟客户端参数以兼容下方代码
	auto frameNumber = o->frameNumber;
#else
inline bool Cannon::Shoot(int const& frameNumber) noexcept {
#endif
	if (!quantity) return false;									// 剩余颗数为 0
	if (frameNumber < shootCD) return false;						// CD 中
	if (bullets->len == cfg->numLimit) return false;				// 总颗数限制
	// todo: 更多发射限制检测
	
	// 置 cd
	shootCD = frameNumber + cfg->shootCD;

	// 剩余颗数 -1
	if (quantity != -1) {
		--quantity;
	}

	auto&& bullet = xx::Make<Bullet>();
	bullet->indexAtContainer = (int)bullets->len;
	bullets->Add(bullet);
	bullet->scene = scene;
	bullet->player = player;
	bullet->cannon = this;
	bullet->cfg = cfg;
	bullet->pos = pos + xx::Rotate(xx::Pos{ cfg->muzzleLen ,0 }, angle * (float(M_PI) / 180.0f));		// 计算炮口坐标
	bullet->angle = angle;	// 角度沿用炮台的( 在发射前炮台已经调整了角度 )
	bullet->moveInc = xx::Rotate(xx::Pos{ cfg->distance ,0 }, angle * (float(M_PI) / 180.0f));	// 计算出每帧移动增量
	bullet->coin = coin;	// 存储发射时炮台的倍率
	player->coin -= coin;	// 扣钱
#ifdef CC_TARGET_PLATFORM
	bullet->id = ++player->autoIncId;
#else
	bullet->id = o->bulletId;

	// 追帧. 令子弹轨迹运行至当前帧编号
	while (frameNumber++ < scene->frameNumber) {
		if (!bullet->Update(frameNumber)) return true;
	}

	// 创建发射事件( 根据追帧后的结果来下发, 减少接收端追帧强度 )
	auto&& fire = xx::Make<PKG::CatchFish::Events::Fire>();
	fire->bulletId = bullet->id;
	fire->coin = bullet->coin;
	fire->frameNumber = scene->frameNumber;
	fire->id = player->id;
	fire->tarAngle = bullet->angle;
	fire->tarPos = bullet->pos;
#endif

	return true;
}

inline Cannon::~Cannon() {
#ifdef CC_TARGET_PLATFORM
	DrawDispose();
#endif
}

#ifdef CC_TARGET_PLATFORM
inline void Cannon::DrawInit() noexcept {
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
}

inline void Cannon::DrawUpdate() noexcept {
	assert(body);
	body->setRotation(-angle);
}

inline void Cannon::DrawDispose() noexcept {
	if (!body) return;

	if (body->getParent()) {
		body->removeFromParent();
	}
	body->release();
	body = nullptr;
}
#endif
