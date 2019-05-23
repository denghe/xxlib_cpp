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

inline int PKG::CatchFish::Cannon::Update(int const& frameNumber) noexcept {

	// 驱动子弹
	auto&& bs = *this->bullets;
	if (bs.len) {
		for (size_t i = bs.len - 1; i != -1; --i) {
			assert(bs[i]->indexAtContainer == (int)i);
			if (bs[i]->Update(frameNumber)) {
#ifdef CC_TARGET_PLATFORM
				bs[bs.len - 1]->indexAtContainer = (int)i;
				bs.SwapRemoveAt(i);
#endif
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
		if (fire) {
			(void)Fire(tpos);	// 返回 0 表示 Fire 成功. 暂时不关注返回值
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
	// 合法性判断: 如果 fishId 找不到就忽略( fishId == 0 表示客户端主动取消该子弹, 退款 )
	auto&& bs = *this->bullets;
	auto&& fs = *scene->fishs;
	// 从子弹队列定位子弹. 如果队列为空直接认为 hit 失败. 当前炮台的子弹逻辑, 子弹 id 必须能被找到
	if (bs.len) {
		for (auto&& i = bs.len - 1; i != -1; --i) {
			auto&& b = bs[i];
			assert(b->indexAtContainer == i);
			// 定位到子弹
			if (b->id == o->bulletId) {
				// 如果属于客户端 cancel, 直接退钱, 最后删子弹退出
				if (!o->fishId) {
					player->coin += b->coin;
					player->MakeRefundEvent(b->coin);
					//xx::CoutN("hit cancel. refund = ", b->coin);
				}
				else {
					// 从鱼队列定位鱼. 如果找到就 fish die check( 本地逻辑进而直接下发 fishdie ). 没找到就退钱. 最后删子弹退出
					if (fs.len) {
						size_t j = fs.len - 1;
						for (; j != -1; --j) {
							auto&& f = fs[j];
							assert(f->indexAtContainer == j);
							// 定位到鱼
							if (f->id == o->fishId) {
#if 1
								// 远程计算逻辑( 依赖 Calc 服务 )
								// 构造 hit 计算数据
								auto && hit = scene->hitChecks->hits->Emplace();
								hit.fishId = f->id;
								hit.fishCoin = f->coin;
								hit.playerId = player->id;
								hit.cannonId = id;
								hit.bulletId = b->id;
								hit.bulletCount = 1;		// 写死. 当前子弹就是单颗
								hit.bulletCoin = b->coin;
#else
								// 本地计算逻辑( 不依赖 Calc 服务 )
								// 先根据 1/coin 死亡比例 来判断是否打死
								if (scene->serverRnd.Next((int)f->coin) == 0) {
									// 算钱
									auto&& c = b->coin * f->coin;	// 数量只可能为 1
									// 构造鱼死事件包
									{
										auto&& fishDead = xx::Make<PKG::CatchFish::Events::FishDead>();
										fishDead->cannonId = id;
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
#endif
								break;
							}
						}
						// 未找到鱼：退钱 & 构造退钱事件包
						if (j == -1) {
							player->coin += b->coin;
							player->MakeRefundEvent(b->coin);
							//xx::CoutN("hit miss. refund = ", b->coin);
						}
					}
				}

				//xx::CoutN("hit bullet success. ", o);
				// 删子弹退出
				bs[bs.len - 1]->indexAtContainer = (int)i;
				bs.SwapRemoveAt(i);
				return 0;
			}
		}
	}
	//xx::CoutN("hit bullet not found. ", o);
	return -1;
}
#endif


#ifndef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Cannon::Fire(PKG::Client_CatchFish::Fire_s& o) noexcept {
	// 如果金币不足, 失败
	if (player->coin < coin) return -2;

	// 剩余颗数为 0
	if (!quantity) return -3;

	// CD 中
	if (o->frameNumber < fireCD) return -4;

	// 总颗数限制
	if (bullets->len == cfg->numLimit) return -5;

	// 如果子弹编号已存在, 失败
	for (auto&& bullet : *bullets) {
		if (bullet->id == o->bulletId) return -1;
	}

	// todo: 更多发射限制检测

	// 检测通过, 开始发射
	// 剩余颗数 -1
	if (quantity != -1) {
		--quantity;
	}

	// 置 cd
	fireCD = o->frameNumber + cfg->fireCD;

	// 根据开火目标坐标计算角度
	angle = o->angle;

	// 扣钱并创建子弹
	player->coin -= coin;
	auto&& bullet = xx::Make<Bullet>();
	bullet->id = o->bulletId;
	bullet->scene = scene;
	bullet->player = player;
	bullet->cannon = this;
	bullet->coin = coin;																		// 存储发射时炮台的倍率
	bullet->cfg = cfg;
	bullet->pos = pos + xx::Rotate(xx::Pos{ cfg->muzzleLen * cfg->scale ,0 }, angle);			// 计算炮口坐标
	bullet->angle = angle;	// 角度沿用炮台的( 在发射前炮台已经调整了角度 )
	bullet->moveInc = xx::Rotate(xx::Pos{ cfg->distance ,0 }, angle);							// 计算出每帧移动增量
	bullet->indexAtContainer = (int)bullets->len;
	bullet->move_status = cfg->move_status;
	bullets->Add(bullet);

	// 创建发射事件
	{
		auto&& fire = xx::Make<PKG::CatchFish::Events::Fire>();
		fire->bulletId = bullet->id;
		fire->frameNumber = o->frameNumber;
		fire->playerId = player->id;
		fire->cannonId = id;
		fire->angle = o->angle;
		scene->frameEvents->events->Add(std::move(fire));
	}

	// 追帧. 令子弹轨迹运行至当前帧编号. 如果已经消失还是要创建出来, 并且下发事件. 收到 HitCheck 时才移除
	auto frameNumber = o->frameNumber;
	while (frameNumber++ < scene->frameNumber) {
		if (bullet->Move()) break;
	}

	return 0;
}
#else
inline int PKG::CatchFish::Cannon::Fire(PKG::CatchFish::Events::Fire const& o) noexcept {
	// 修正炮台朝向
	angle = o.angle;

	// 剩余颗数 -1
	if (quantity != -1) {
		--quantity;
	}

	// 扣钱并创建子弹
	player->coin -= coin;
	auto&& bullet = xx::Make<Bullet>();
	bullet->id = o.bulletId;
	bullet->scene = scene;
	bullet->player = player;
	bullet->cannon = this;
	bullet->coin = coin;																		// 存储发射时炮台的倍率
	bullet->cfg = cfg;
	bullet->pos = pos + xx::Rotate(xx::Pos{ cfg->muzzleLen * cfg->scale ,0 }, angle);			// 计算炮口坐标
	bullet->angle = angle;
	bullet->moveInc = xx::Rotate(xx::Pos{ cfg->distance ,0 }, angle);							// 计算出每帧移动增量

	// 非自己的子弹：子弹坐标追帧同步
	if (!player->isSelf && scene->cfg->enableBulletFastForward) {
		for (auto frameNumber = o.frameNumber; frameNumber < scene->frameNumber; ++frameNumber) {
			// 如果子弹生命周期已经到了就退出( 不再放入容器 & 绘制 )
			if (int r = bullet->Move()) return 0;
		}
	}

	bullet->indexAtContainer = (int)bullets->len;
	bullets->Add(bullet);
	bullet->DrawInit();			// 绘制
	// todo: 开始播放炮台开火特效, 音效
	return 0;
}
inline int PKG::CatchFish::Cannon::Fire(xx::Pos const& tarPos) noexcept {
	// 如果金币不足, 失败
	if (player->coin < coin) return -2;

	// 剩余颗数为 0
	if (!quantity) return -3;

	// CD 中
	if (scene->frameNumber < fireCD) return -4;

	// 总颗数限制
	if (bullets->len == cfg->numLimit) return -5;

	// 模拟 Events::Fire 结构体 call 发射函数. 只用到 bulletId & tarAngle 这两个属性
	PKG::CatchFish::Events::Fire f;
	f.bulletId = ++player->autoIncId;
	f.angle = xx::GetAngle(pos, tarPos);
	int r = Fire(f);
	assert(!r);

	// 构造开火包
	auto&& pkg = xx::Make<PKG::Client_CatchFish::Fire>();
	pkg->bulletId = f.bulletId;
	pkg->cannonId = id;
	pkg->frameNumber = scene->frameNumber;
	pkg->angle = f.angle;
	::dialer->peer->SendPush(pkg);
	::dialer->peer->Flush();

	return 0;
}
#endif

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
