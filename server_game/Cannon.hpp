inline int Cannon::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	assert(player);
	assert(!cfg);
	cfg = &*scene->cfg->cannons->At(cfgId);
	pos = player->pos;

	// 前置填充
	for (auto&& bullet : *bullets) {
		auto&& b = xx::As<Bullet>(bullet);
		b->player = player;
		b->cannon = this;
		b->cfg = cfg;
	}

	if (int r = this->BaseType::InitCascade(o)) return r;
#ifdef CC_TARGET_PLATFORM
	DrawInit();
#endif
	return 0;
}

#ifndef CC_TARGET_PLATFORM
inline void Cannon::MakeRefundEvent(int64_t const& coin, bool isPersional) noexcept {
	auto&& refund = xx::Make<PKG::CatchFish::Events::Refund>();
	refund->coin = coin;
	refund->playerId = player->id;
	if (isPersional) {
		player->events->Add(std::move(refund));
	}
	else {
		scene->frameEvents->events->Add(std::move(refund));
	}
}
#endif

inline int Cannon::Update(int const& frameNumber) noexcept {

	// 驱动子弹
	auto&& bs = *this->bullets;
	if (bs.len) {
		for (size_t i = bs.len - 1; i != -1; --i) {
			assert(bs[i]->indexAtContainer == (int)i);
			if (bs[i]->Update(frameNumber)) {
#ifndef CC_TARGET_PLATFORM
				auto&& coin = bs[i]->coin;
				// 退钱 & 构造退钱事件包
				player->coin += coin;
				MakeRefundEvent(coin);
				//xx::CoutN("bullet fly out the screen. refund " + coin);
#endif
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

	// 玩家本人的炮可射击
	if (player->isSelf) {
		bool fire = false;
		xx::Pos tpos;

		// 自动射击。直接在屏幕上随机一个坐标来用
		if (::dialer->autoFire) {
			tpos = { ::dialer->rnd.Next(ScreenWidth) - ScreenCenter.x, ::dialer->rnd.Next(ScreenHeight) - ScreenCenter.y };
			fire = true;
		}
		else {
			// 输入检测. 炮台角度对准首个 touch 点( 暂定方案 )
			if (cc_touchs.len) {
				// 世界坐标
				auto tloc = cc_touchs[0]->getLocationInView();
				// 转为游戏坐标系
				tpos = { tloc.x - ScreenCenter.x, ScreenCenter.y - tloc.y };
				fire = true;
			}
		}
		if(fire) {
			// 算出朝向角度
			angle = xx::GetAngle(pos, tpos);

			// 试着发射
			int r = Fire(frameNumber);
			if (!r) {
				// todo: 开始播放炮台开火特效, 音效
			}
		}
	}
	DrawUpdate();
#endif

	return 0;
};

#ifndef CC_TARGET_PLATFORM
inline int Cannon::Hit(PKG::Client_CatchFish::Hit_s& o) noexcept {
	// 合法性判断: 如果 bulletId / fishId 找不到就忽略
	auto&& bs = *this->bullets;
	auto&& fs = *scene->fishs;
	if (bs.len) {
		// 从子弹队列定位子弹
		for (size_t i = bs.len - 1; i != -1; --i) {
			auto&& b = bs[i];
			assert(b->indexAtContainer == i);
			// 定位到子弹
			if (b->id == o->bulletId) {
				if (o->fishId && fs.len) {
					// 从鱼队列定位鱼
					size_t j = fs.len - 1;
					for (; j != -1; --j) {
						auto&& f = fs[j];
						assert(f->indexAtContainer == j);
						// 定位到鱼
						if (f->id == o->fishId) {
							// 先根据 1/coin 死亡比例 来判断是否打死
							if (scene->serverRnd.Next((int)f->coin) == 0) {
								// 算钱
								auto&& c = b->coin * f->coin;
								// 构造鱼死事件包
								{
									auto&& fishDead = xx::Make<PKG::CatchFish::Events::FishDead>();
									fishDead->bulletId = b->id;
									fishDead->coin = c;
									fishDead->fishId = f->id;
									fishDead->playerId = player->id;
									scene->frameEvents->events->Add(std::move(fishDead));
								}
								// 加钱
								player->coin += c;
								// 删鱼
								fs[fs.len - 1]->indexAtContainer = (int)j;
								fs.SwapRemoveAt(j);
								//xx::CoutN("hit fish dead. ", o);
							}
							else {
								//xx::CoutN("hit fish not dead. ", o);
							}
							break;
						}
					}
					// 未找到鱼：退钱 & 构造退钱事件包
					if (j == -1)
					{
						player->coin += b->coin;
						MakeRefundEvent(b->coin);
						//xx::CoutN("hit miss. refund = ", b->coin);
					}
				}
				// 删子弹
				bs[bs.len - 1]->indexAtContainer = (int)i;
				bs.SwapRemoveAt(i);
				break;
			}
		}
	}

	//xx::CoutN("hit bullet not found. ", o);
	return 0;
}
#endif


#ifndef CC_TARGET_PLATFORM
inline int Cannon::Fire(PKG::Client_CatchFish::Fire_s& o) noexcept {
	// 如果金币不足, 失败
	if (player->coin < coin) return -1;

	// 如果子弹编号已存在, 失败
	for (auto&& bullet : *bullets) {
		if (bullet->id == o->bulletId) return -2;
	}

	// 根据开火目标坐标计算角度
	angle = xx::GetAngle(pos, o->pos);

	// 模拟客户端参数以兼容下方代码
	auto frameNumber = o->frameNumber;
#else
inline int Cannon::Fire(int const& frameNumber) noexcept {
	// 只有玩家本人发射行为受限
	if (player->isSelf) {
#endif
		if (!quantity) return -3;									// 剩余颗数为 0
		if (frameNumber < fireCD) return -4;						// CD 中
		if (bullets->len == cfg->numLimit) {						// 总颗数限制( 并不算是一种错误. 前后端子弹消失时间差可能导致 )
#ifndef CC_TARGET_PLATFORM
			MakeRefundEvent(coin, true);							// 生成专有发射取消事件通知( 没必要发送给其他玩家 )
#endif
			return 0;
		}
		// todo: 更多发射限制检测

		// 置 cd
		fireCD = frameNumber + cfg->fireCD;
#ifdef CC_TARGET_PLATFORM
	}
#endif
	// 剩余颗数 -1
	if (quantity != -1) {
		--quantity;
	}

	auto&& bullet = xx::TryMake<Bullet>();
	if (!bullet) return -6;
	bullet->indexAtContainer = (int)bullets->len;
	bullets->Add(bullet);
	bullet->scene = scene;
	bullet->player = player;
	bullet->cannon = this;
	bullet->cfg = cfg;
	bullet->pos = pos + xx::Rotate(xx::Pos{ cfg->muzzleLen * cfg->scale ,0 }, angle);			// 计算炮口坐标
	bullet->angle = angle;	// 角度沿用炮台的( 在发射前炮台已经调整了角度 )
	bullet->moveInc = xx::Rotate(xx::Pos{ cfg->distance ,0 }, angle);							// 计算出每帧移动增量
	bullet->coin = coin;	// 存储发射时炮台的倍率
	player->coin -= coin;	// 扣钱
#ifdef CC_TARGET_PLATFORM
	bullet->id = ++player->autoIncId;
	bullet->DrawInit();

	if (player->isSelf) {
		// 如果是玩家本人就发包
		auto&& pkg = xx::TryMake<PKG::Client_CatchFish::Fire>();
		if (!pkg) return -9;
		pkg->bulletId = bullet->id;
		pkg->cannonId = this->id;
		pkg->frameNumber = scene->frameNumber;
		pkg->pos = bullet->pos;
		::dialer->peer->SendPush(pkg);
		::dialer->peer->Flush();
	}
	else {
		// 其他玩家: 子弹追帧并绘制( 追或不追, 这是个选项 )
		auto&& times = scene->frameNumber - frameNumber;
		assert(times >= 0);
		while (--times > 0) {
			if (int r = bullet->Move()) return 0;
		}
	}
#else
	bullet->id = o->bulletId;

	// 追帧. 令子弹轨迹运行至当前帧编号
	while (frameNumber++ < scene->frameNumber) {
		if (int r = bullet->Move()) return 0;
	}

	// 创建发射事件
	auto&& fire = xx::Make<PKG::CatchFish::Events::Fire>();
	fire->bulletId = bullet->id;
	fire->coin = bullet->coin;
	fire->frameNumber = o->frameNumber;				// 有些子弹不信任 client 帧编号, 下发 scene->frameNumber
	fire->playerId = player->id;
	fire->cannonId = id;
	fire->tarAngle = bullet->angle;
	scene->frameEvents->events->Add(std::move(fire));

	//xx::CoutN("fire. ", o);
#endif

	return 0;
}

#ifdef CC_TARGET_PLATFORM
inline void Cannon::DrawInit() noexcept {
	assert(!body);
	body = cocos2d::Sprite::create();
	body->setGlobalZOrder(cfg->zOrder);
	auto&& sf = xx::As<SpriteFrame>(cfg->frames->At(0))->spriteFrame;
	body->setSpriteFrame(sf);
	body->setPosition(pos);
	body->setScale(cfg->scale);
	body->setRotation(-angle * (180.0f / float(M_PI)));
	cc_scene->addChild(body);
}

inline void Cannon::DrawUpdate() noexcept {
	assert(body);
	body->setRotation(-angle * (180.0f / float(M_PI)));
}
#endif
