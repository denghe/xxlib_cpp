inline CatchFish::CatchFish() {
}

inline CatchFish::~CatchFish() {
	disposed = true;
	// todo: more release
#ifdef CC_TARGET_PLATFORM
	dialer.reset();
	cc_listener.Reset();
#endif
}

inline int CatchFish::Update() noexcept {
#ifdef CC_TARGET_PLATFORM
	return dialer->Update();
#else
	return scene->Update();
#endif
}

#ifndef CC_TARGET_PLATFORM
inline int ReadFile(const char* fn, xx::BBuffer& bb)
{
	FILE* fp = fopen(fn, "rb");
	if (nullptr == fp) return -1;
	fseek(fp, 0L, SEEK_END);	// 定位到文件末尾
	int flen = ftell(fp);		// 得到文件大小
	if (flen <= 0)
	{
		fclose(fp);
		return -1;
	}
	bb.Clear();
	bb.Reserve(flen);			// 申请内存空间
	fseek(fp, 0L, SEEK_SET);	// 定位到文件开头
	if (fread(bb.buf, flen, 1, fp) != 1) return -2; // 一次性读取全部文件内容
	bb.len = flen;
	fclose(fp);
	return 0;
}

inline int CatchFish::Init(std::string const& cfgName) noexcept {
	// 从文件加载 cfg. 出问题返回非 0
	{
		xx::BBuffer bb;
		if (int r = ReadFile(cfgName.c_str(), bb)) return r;
		if (int r = bb.ReadRoot(cfg)) return r;
	}
	// 初始化派生类的东西
	if (int r = cfg->InitCascade()) return r;

	// 场景初始化
	xx::MakeTo(scene);
	xx::MakeTo(scene->borns);
	xx::MakeTo(scene->fishs);
	xx::MakeTo(scene->freeSits);
	xx::MakeTo(scene->items);
	xx::MakeTo(scene->players);
	xx::MakeTo(scene->rnd, 123);				// todo: 时间 seed ?
	xx::MakeTo(scene->frameEvents);
	xx::MakeTo(scene->frameEvents->events);
	scene->cfg = &*cfg;
	scene->catchFish = this;
	
	// 关卡初始化
	int r = cfg->stageBufs[0].ReadRoot(scene->stage);
	assert(!r);
	scene->stage->InitCascade(&*scene);

	// 空位初始化
	scene->freeSits->Add(PKG::CatchFish::Sits::LeftTop
		, PKG::CatchFish::Sits::RightTop
		, PKG::CatchFish::Sits::RightBottom
		, PKG::CatchFish::Sits::LeftBottom);
#else
inline int CatchFish::Init(std::string const& ip, int const& port, std::string const& cfgName) noexcept {
	// 暂存 ip, port
	serverIp = ip;
	serverPort = port;

	assert(!cc_scene);
	// 初始化 cocos 相关
	cc_visibleSize = cocos2d::Director::getInstance()->getOpenGLView()->getDesignResolutionSize();
	cc_visibleSize_2 = cc_visibleSize / 2;
	cc_p1 = { -cc_visibleSize_2.width, -cc_visibleSize_2.height };
	cc_p2 = { 0, -cc_visibleSize_2.height };
	cc_p3 = { cc_visibleSize_2.width, -cc_visibleSize_2.height };
	cc_p4 = { -cc_visibleSize_2.width, 0 };
	cc_p5 = { 0, 0 };
	cc_p6 = { cc_visibleSize_2.width, 0 };
	cc_p7 = { -cc_visibleSize_2.width, cc_visibleSize_2.height };
	cc_p8 = { 0, cc_visibleSize_2.height };
	cc_p9 = { cc_visibleSize_2.width, cc_visibleSize_2.height };

	cc_scene = cocos2d::Director::getInstance()->getRunningScene();

	cc_fishNode = cocos2d::ClippingRectangleNode::create({ -designSize_2.x, -designSize_2.y, designSize.x, designSize.y });
	cc_fishNode->setScale(designSize.x / designSize.y > cc_visibleSize.width / cc_visibleSize.height ? cc_visibleSize.width / designSize.x : cc_visibleSize.height / designSize.y);
	cc_scene->addChild(cc_fishNode);

	cc_uiNode = cocos2d::Node::create();
	cc_scene->addChild(cc_uiNode);
	
	cc_listener = cocos2d::EventListenerTouchAllAtOnce::create();
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

	// 从文件加载 cfg. 出问题返回非 0
	{
		auto&& data = cocos2d::FileUtils::getInstance()->getDataFromFile(cfgName);
		xx::BBuffer bb;
		bb.Reset(data.getBytes(), data.getSize());
		auto&& r = bb.ReadRoot(cfg);
		bb.Reset();
		if (r) return r;
	}

	// 初始化派生类的东西
	if (int r = cfg->InitCascade()) return r;

	// 存易用单例
	::catchFish = this;

	// 初始化拨号器
	xx::MakeTo(::dialer);
#endif
	return 0;
}

inline void CatchFish::Cleanup(PKG::CatchFish::Player_s p) noexcept {
	assert(p);
#ifndef CC_TARGET_PLATFORM
	// 网络解绑
	if (p->peer) {
		assert(p->peer->player_w.lock() == p);
		p->peer->Dispose(1);
		p->peer.reset();
	}
#endif

	// 从玩家总容器移除
	assert(players.Find(p) != -1);
	players.Remove(p);

	// 从玩家所在场景移除
	auto && ps = *p->scene->players;
	assert(ps.len);
	size_t i = ps.len - 1;
	for (; i != -1; --i) {
		if (ps[i].lock() == p) {
			ps.SwapRemoveAt(i);
			break;
		}
	}
	assert(i != -1);

#ifndef CC_TARGET_PLATFORM
	// 归还座位
	assert(p->scene->freeSits->Find(p->sit) == -1);
	p->scene->freeSits->Add(p->sit);

	// 清理该玩家已产生的事件
	auto && es = *p->scene->frameEvents->events;
	if (es.len) {
		for (i = es.len - 1; i != -1; --i) {
			if (es[i]->playerId == p->id) {
				es.SwapRemoveAt(i);
			}
		}
	}

	// 生成离开事件
	{
		auto&& e = xx::Make<PKG::CatchFish::Events::Leave>();
		e->playerId = p->id;
		p->scene->frameEvents->events->Add(std::move(e));
	}
#endif

	xx::CoutTN("cleanup player: id = ", p->id);
}
