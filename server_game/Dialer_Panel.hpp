inline Dialer_Panel::Dialer_Panel(Dialer* dialer)
	: dialer(dialer)
{
	// 从下往上流式布局. g1 坐标向上偏移 70 以避开 cocos ogl 统计区
	auto&& pos = cc_p1 + xx::Pos{ 10, 70 };
	std::function<cocos2d::Label*(char const* const& txt, float const& fontSize)> CreateLabelToUI = [&](char const* const& txt, float const& fontSize) {
		auto&& L = cocos2d::Label::createWithSystemFont(txt, "", fontSize);
		L->setAnchorPoint({ 0, 0 });	// 以 Label 左下角为基点定位
		L->setPosition(pos);
		pos.y += fontSize + 3;			// 更新 y 坐标指向下一个可用位置. 行距 3
		cc_uiNode->addChild(L);
		return L;
	};

	btnRedial = CreateLabelToUI("Redial", 32);

	labelServer = CreateLabelToUI("", 24);

	labelPing = CreateLabelToUI("", 32);

	btnSwitchTcpKcp = CreateLabelToUI("", 32);

	labelNumDialTimes = CreateLabelToUI("", 32);

	labelNumFishs = CreateLabelToUI("", 32);

	btnAutoFire = CreateLabelToUI("", 32);

	SetText_AutoFire(dialer->autoFire);

	listenerAutoFire = cocos2d::EventListenerTouchOneByOne::create();
	listenerAutoFire->onTouchBegan = [this](cocos2d::Touch * t, cocos2d::Event * e) {
		auto&& tL = t->getLocation();
		auto&& p = btnAutoFire->convertToNodeSpace(tL);
		auto&& s = btnAutoFire->getContentSize();
		cocos2d::Rect r{ 0,0, s.width, s.height };
		auto&& b = r.containsPoint(p);
		if (b) {
			this->dialer->autoFire = !this->dialer->autoFire;
			SetText_AutoFire(this->dialer->autoFire);
		}
		return b;
	};
	cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerAutoFire, btnAutoFire);

	listenerRedial = cocos2d::EventListenerTouchOneByOne::create();
	listenerRedial->onTouchBegan = [this](cocos2d::Touch * t, cocos2d::Event * e) {
		auto&& tL = t->getLocation();
		auto&& p = btnRedial->convertToNodeSpace(tL);
		auto&& s = btnRedial->getContentSize();
		cocos2d::Rect r{ 0,0, s.width, s.height };
		auto&& b = r.containsPoint(p);
		if (b) {
			if (this->dialer->peer) {
				this->dialer->peer->Dispose(1);
			}
		}
		return b;
	};
	cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerRedial, btnRedial);

	//listenerSwitchTcpKcp = cocos2d::EventListenerTouchOneByOne::create();
	//listenerSwitchTcpKcp->onTouchBegan = [this](cocos2d::Touch * t, cocos2d::Event * e) {
	//	auto&& tL = t->getLocation();
	//	auto&& p = btnSwitchTcpKcp->convertToNodeSpace(tL);
	//	auto&& s = btnSwitchTcpKcp->getContentSize();
	//	cocos2d::Rect r{ 0,0, s.width, s.height };
	//	auto&& b = r.containsPoint(p);
	//	if (b) {
	//		if (this->dialer->dialer) {
	//			this->dialer->dialer->Dispose(1);
	//		}
	//		if (this->dialer->peer) {
	//			this->dialer->peer->Dispose(1);
	//		}
	//		this->dialer->useKcp = !this->dialer->useKcp;
	//		this->SetText_TcpKcp(this->dialer->useKcp);
	//	}
	//	return b;
	//};
	//cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerSwitchTcpKcp, btnSwitchTcpKcp);
}

inline void Dialer_Panel::SetText_TcpKcp(bool const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!btnSwitchTcpKcp) return;
	std::string s = "protocol: ";
	s += value ? "KCP" : "TCP";
	btnSwitchTcpKcp->setString(s);
}
inline void Dialer_Panel::SetText_AutoFire(bool const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!btnAutoFire) return;
	std::string s = "auto fire: ";
	s += value ? "ON" : "OFF";
	btnAutoFire->setString(s);
}
inline void Dialer_Panel::SetText_NumDialTimes(int64_t const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelNumDialTimes) return;
	labelNumDialTimes->setString("reconnect times: " + std::to_string(value));
}
inline void Dialer_Panel::SetText_Ping(int64_t const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelPing) return;
	if (value < 0) {
		labelPing->setString("ping: timeout");
	}
	else {
		std::string s;
		xx::Append(s, "ping: ", value, "ms");
		labelPing->setString(s);
	}
}
inline void Dialer_Panel::SetText_NumFishs(size_t const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelNumFishs) return;
	labelNumFishs->setString("num fishs: " + std::to_string(value));
}
inline void Dialer_Panel::SetText_Server(std::string const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelServer) return;
	labelServer->setString(value);
}
