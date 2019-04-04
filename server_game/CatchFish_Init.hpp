inline int CatchFish::Init(std::string cfgName) {
#ifdef CC_TARGET_PLATFORM
	assert(!cc_scene);
	// 初始化 cocos 相关
	cc_scene = cocos2d::Director::getInstance()->getRunningScene();
	cc_listener = cocos2d::EventListenerTouchAllAtOnce::create();
	cc_listener->retain();
	cc_listener->onTouchesBegan = [](const std::vector<cocos2d::Touch*>& ts, cocos2d::Event* e) {
		cc_touchs.AddRange(ts.data(), ts.size());
	};
	cc_listener->onTouchesMoved = [](const std::vector<cocos2d::Touch*>& ts, cocos2d::Event* e) {
	};
	cc_listener->onTouchesEnded = [](const std::vector<cocos2d::Touch*>& ts, cocos2d::Event* e) {
		for (auto&& t : ts) {
			cc_touchs.Remove(t);
		}
	};
	cc_listener->onTouchesCancelled = cc_listener->onTouchesEnded;
	cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(cc_listener, cc_scene);
#endif

	// 从文件加载 cfg. 出问题返回非 0
	{
		xx::BBuffer bb;
		if (int r = ReadFile(cfgName.c_str(), bb)) return r;
		if (int r = bb.ReadRoot(cfg)) return r;
	}

	// 初始化派生类的东西
	if (int r = cfg->InitCascade()) return r;

	// 模拟收到 sync( 含 players & scene )
	xx::MakeTo(scene);
	xx::MakeTo(scene->borns);
	xx::MakeTo(scene->fishs);
	xx::MakeTo(scene->freeSits);
	xx::MakeTo(scene->items);
	xx::MakeTo(scene->players);
	xx::MakeTo(scene->rnd, 123);
	xx::MakeTo(scene->stage);

	scene->cfg = &*cfg;
	if (int r = scene->InitCascade()) return r;

	auto&& plr = xx::Make<Player>();
	players.Add(plr);
	scene->players->Add(plr);
	xx::MakeTo(plr->cannons);
	xx::MakeTo(plr->weapons);
	auto&& cannon = xx::Make<Cannon>();
	plr->cannons->Add(cannon);
	xx::MakeTo(cannon->bullets);
	cannon->id = 123;
	cannon->cfgId = 0;
	cannon->angle = float(cfg->cannons->At(cannon->cfgId)->angle);
	cannon->quantity = cfg->cannons->At(cannon->cfgId)->quantity;
	cannon->coin = 1;
	if (int r = players.InitCascade(&*scene)) return r;

	// todo
	// selfPlayer = plr;

	return 0;
}
