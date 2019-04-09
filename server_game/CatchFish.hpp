inline CatchFish::CatchFish() {
	// 注册替代类型
	xx::BBuffer::Register<SpriteFrame>(xx::TypeId_v<PKG::CatchFish::Configs::SpriteFrame>);
	xx::BBuffer::Register<Physics>(xx::TypeId_v<PKG::CatchFish::Configs::Physics>);
	xx::BBuffer::Register<Scene>(xx::TypeId_v<PKG::CatchFish::Scene>);
	xx::BBuffer::Register<Player>(xx::TypeId_v<PKG::CatchFish::Player>);
	xx::BBuffer::Register<Fish>(xx::TypeId_v<PKG::CatchFish::Fish>);
	xx::BBuffer::Register<Cannon>(xx::TypeId_v<PKG::CatchFish::Cannon>);
	// todo: more
}

inline void CatchFish::Dispose(int const& flag) noexcept {
	if (disposed) return;

	// todo: release all
#ifdef CC_TARGET_PLATFORM
	dialer.reset();

	cocos2d::Director::getInstance()->getEventDispatcher()->removeEventListener(cc_listener);
	cc_listener = nullptr;
#endif
}

inline CatchFish::~CatchFish() {
	Dispose(0);
}


inline int CatchFish::Init(std::string const& cfgName) noexcept {
#ifdef CC_TARGET_PLATFORM
	assert(!cc_scene);
	// 初始化 cocos 相关
	cc_scene = cocos2d::Director::getInstance()->getRunningScene();
	cc_listener = cocos2d::EventListenerTouchAllAtOnce::create();
	cc_listener->retain();
	cc_listener->onTouchesBegan = [](const std::vector<cocos2d::Touch*> & ts, cocos2d::Event * e) {
		cc_touchs.AddRange(ts.data(), ts.size());
	};
	cc_listener->onTouchesMoved = [](const std::vector<cocos2d::Touch*> & ts, cocos2d::Event * e) {
	};
	cc_listener->onTouchesEnded = [](const std::vector<cocos2d::Touch*> & ts, cocos2d::Event * e) {
		for (auto&& t : ts) {
			cc_touchs.Remove(t);
		}
	};
	cc_listener->onTouchesCancelled = cc_listener->onTouchesEnded;
	cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(cc_listener, cc_scene);
#endif

	// 从文件加载 cfg. 出问题返回非 0
	{
#ifndef CC_TARGET_PLATFORM
		xx::BBuffer bb;
		if (int r = ReadFile(cfgName.c_str(), bb)) return r;
		if (int r = bb.ReadRoot(cfg)) return r;
#else
		auto && data = cocos2d::FileUtils::getInstance()->getDataFromFile(cfgName);
		xx::BBuffer bb;
		bb.Reset(data.getBytes(), data.getSize());
		auto && r = bb.ReadRoot(cfg);
		bb.Reset();
		if (r) return r;
#endif
	}

	// 初始化派生类的东西
	if (int r = cfg->InitCascade()) return r;


#ifdef CC_TARGET_PLATFORM
	// 存易用单例
	::catchFish = this;

	// 初始化拨号器
	xx::MakeTo(::dialer, *uv);

#else
	xx::MakeTo(scene);
	xx::MakeTo(scene->borns);
	xx::MakeTo(scene->fishs);
	xx::MakeTo(scene->freeSits);
	xx::MakeTo(scene->items);
	xx::MakeTo(scene->players);
	xx::MakeTo(scene->rnd, 123);
	xx::MakeTo(scene->stage);
	xx::MakeTo(scene->frameEvents);
	xx::MakeTo(scene->frameEvents->events);
	scene->cfg = &*cfg;

	scene->freeSits->Add(PKG::CatchFish::Sits::LeftTop
		, PKG::CatchFish::Sits::RightTop
		, PKG::CatchFish::Sits::RightBottom
		, PKG::CatchFish::Sits::LeftBottom);
#endif

	//	if (int r = scene->InitCascade()) return r;
	//
	//	auto&& plr = xx::Make<Player>();
	//	players.Add(plr);
	//	scene->players->Add(plr);
	//	xx::MakeTo(plr->cannons);
	//	xx::MakeTo(plr->weapons);
	//	auto&& cannon = xx::Make<Cannon>();
	//	plr->cannons->Add(cannon);
	//	xx::MakeTo(cannon->bullets);
	//	cannon->id = 123;
	//	cannon->cfgId = 0;
	//	cannon->angle = float(cfg->cannons->At(cannon->cfgId)->angle);
	//	cannon->quantity = cfg->cannons->At(cannon->cfgId)->quantity;
	//	cannon->coin = 1;
	//	if (int r = players.InitCascade(&*scene)) return r;

	return 0;
}

inline int CatchFish::Update() noexcept {
#ifdef CC_TARGET_PLATFORM
	return dialer->Update();
#else
	return scene->Update();
#endif
}
