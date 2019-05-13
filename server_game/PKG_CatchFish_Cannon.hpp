#ifdef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Cannon::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	assert(player);
	assert(!cfg);
	cfg = &*scene->cfg->cannons->At(cfgId);
	pos = player->pos;

	// 前置填充
	for (auto&& b : *bullets) {
		b->player = player;
		b->cannon = this;
		b->cfg = cfg;
	}

	if (int r = InitCascadeCore(o)) return r;
	DrawInit();
	return 0;
}
#endif

#ifndef CC_TARGET_PLATFORM
inline void PKG::CatchFish::Cannon::MakeRefundEvent(int64_t const& coin, bool isPersional) noexcept {
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

inline int PKG::CatchFish::Cannon::Update(int const& frameNumber) noexcept {

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
			tpos = { ::dialer->rnd.Next((int)designSize.x) - designSize_2.x, ::dialer->rnd.Next((int)designSize.y) - designSize_2.y };
			fire = true;
		}
		else {
			// 输入检测. 炮台角度对准首个 touch 点( 暂定方案 )
			if (cc_touchs.len) {
				auto tloc = cc_fishNode->convertTouchToNodeSpace(cc_touchs[0]);
				tpos = { tloc.x, tloc.y };
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
inline int PKG::CatchFish::Cannon::SetCoin(PKG::Client_CatchFish::Bet_s& o) noexcept {
	if (o->cannonId != id) return -1;
	if (o->coin <= 0) return -2;
	if (o->coin > 1000) return -3;	// todo: 押注最大值最小值应该读取当前配置来判断
	if (o->coin == coin) return -4;
	// todo: 更多判断. 比如倍率必然为限定的几个值
	coin = o->coin;

	// 创建压住变化通知事件
	auto&& ccc = xx::Make<PKG::CatchFish::Events::CannonCoinChange>();
	ccc->playerId = player->id;
	ccc->coin = coin;
	scene->frameEvents->events->Add(std::move(ccc));
	return 0;
}

inline int PKG::CatchFish::Cannon::Hit(PKG::Client_CatchFish::Hit_s& o) noexcept {
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
inline int PKG::CatchFish::Cannon::Fire(PKG::Client_CatchFish::Fire_s& o) noexcept {
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
inline int PKG::CatchFish::Cannon::Fire(int const& frameNumber) noexcept {
	// 只有玩家本人发射行为受限
	if (player->isSelf) {
		// 如果金币不足, 失败
		if (player->coin < coin) return 0;	// todo: 显示提示?
#endif
		if (!quantity) return -4;									// 剩余颗数为 0
		if (frameNumber < fireCD) return -5;						// CD 中
		if (bullets->len == cfg->numLimit) {						// 总颗数限制( 并不算是一种错误. 前后端子弹消失时间差可能导致 )
#ifndef CC_TARGET_PLATFORM
			MakeRefundEvent(coin, true);							// 生成专有发射取消事件通知( 没必要发送给其他玩家 )
#else
			// todo: 看情况显示发射遇到上限?
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
	bullet->coin = coin;											// 存储发射时炮台的倍率
	player->coin -= coin;											// 扣钱
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
	else if (scene->cfg->enableBulletFastForward) {
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
		// 如果子弹生命周期已经到了
		if (int r = bullet->Move()) {
			player->coin += coin;									// 退钱
			MakeRefundEvent(coin, true);							// 生成专有退款事件通知( 没必要发送给其他玩家 )
			return 0;
		}
	}

	// 创建发射事件
	{
		auto&& fire = xx::Make<PKG::CatchFish::Events::Fire>();
		fire->bulletId = bullet->id;
		fire->frameNumber = o->frameNumber;
		fire->playerId = player->id;
		fire->cannonId = id;
		fire->tarAngle = bullet->angle;
		scene->frameEvents->events->Add(std::move(fire));
	}

	//xx::CoutN("fire. ", o);
#endif

	return 0;
}

#ifdef CC_TARGET_PLATFORM
inline void PKG::CatchFish::Cannon::DrawInit() noexcept {
	// 绘制炮台本体
	assert(!body);
	body = cocos2d::Sprite::create();
	body->setLocalZOrder(cfg->zOrder);
	auto&& sf = cfg->frames->At(0)->spriteFrame;
	body->setSpriteFrame(sf);
	body->setPosition(pos);
	body->setScale(cfg->scale);
	body->setRotation(-angle * (180.0f / float(M_PI)));
	cc_fishNode->addChild(body);

	// 当前倍率显示
	labelCoin = cocos2d::Label::createWithSystemFont("", "", 32);
	labelCoin->setPosition(pos);
	labelCoin->setLocalZOrder(cfg->zOrder);
	cc_fishNode->addChild(labelCoin);
	SetText_Coin();

	// 如果不是当前玩家则绘制 + - 按钮
	if (!player->isSelf) return;

	// 简单计算 + - 按钮绘制坐标偏移值( 也可以写死 )
	float btnOffset = sf->getOriginalSize().width * cfg->scale / 2 + 20;

	// 绘制倍率 + - 按钮
	btnInc = cocos2d::Label::createWithSystemFont("+", "", 64, cocos2d::Size::ZERO, cocos2d::TextHAlignment::LEFT, cocos2d::TextVAlignment::CENTER);
	btnInc->setAnchorPoint({ 1, 0.5 });	// 以 Label 右中为基点定位
	btnInc->setPosition(pos + xx::Pos{ btnOffset, 0});
	btnInc->setLocalZOrder(cfg->zOrder);
	cc_fishNode->addChild(btnInc);

	btnDec = cocos2d::Label::createWithSystemFont("-", "", 64, cocos2d::Size::ZERO, cocos2d::TextHAlignment::RIGHT, cocos2d::TextVAlignment::CENTER);
	btnDec->setAnchorPoint({ 0, 0.5 });	// 以 Label 左中为基点定位
	btnDec->setPosition(pos - xx::Pos{ btnOffset, 0});
	btnDec->setLocalZOrder(cfg->zOrder);
	cc_fishNode->addChild(btnDec);

	// 绑定触摸监听器
	listenerIncDec = cocos2d::EventListenerTouchOneByOne::create();
	listenerIncDec->setSwallowTouches(true);
	listenerIncDec->onTouchBegan = [this](cocos2d::Touch * t, cocos2d::Event * e) {
		auto&& tL = t->getLocation();
		bool b = false;
		{
			auto&& p = btnInc->convertToNodeSpace(tL);
			auto&& s = btnInc->getContentSize();
			cocos2d::Rect r{ 0,0, s.width, s.height };
			b = r.containsPoint(p);
		}
		if (b) {
			if (coin < 1000) {
				coin *= 10;
				ChangeCoin();
			}
			return b;
		}
		else {
			auto&& p = btnDec->convertToNodeSpace(tL);
			auto&& s = btnDec->getContentSize();
			cocos2d::Rect r{ 0,0, s.width, s.height };
			b = r.containsPoint(p);
		}
		if (b) {
			if (coin > 1) {
				coin /= 10;
				ChangeCoin();
			}
		}
		return b;
	};
	cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerIncDec, btnInc);
}

inline void PKG::CatchFish::Cannon::DrawUpdate() noexcept {
	assert(body);
	body->setRotation(-angle * (180.0f / float(M_PI)));
}


inline void PKG::CatchFish::Cannon::ChangeCoin() noexcept {
	SetText_Coin();
	auto&& pkg = xx::Make<PKG::Client_CatchFish::Bet>();
	pkg->cannonId = id;
	pkg->coin = coin;
	::dialer->peer->SendPush(pkg);
	::dialer->peer->Flush();
}

inline void PKG::CatchFish::Cannon::SetText_Coin() noexcept {
	assert(!::catchFish->disposed);
	if (!labelCoin) return;
	labelCoin->setString(std::to_string(coin));
}

#endif
