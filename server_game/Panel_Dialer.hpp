inline Panel_Dialer::Panel_Dialer(Dialer* dialer)
	: dialer(dialer)
{
	{
		labelServer = cocos2d::Label::createWithSystemFont("", "", 24);
		labelServer->setPosition(10 - ScreenCenter.x, 240 - ScreenCenter.y);
		labelServer->setAnchorPoint({ 0, 0.5 });
		labelServer->setGlobalZOrder(1000);
		cc_scene->addChild(labelServer);
	}
	{
		btnRedial = cocos2d::Label::createWithSystemFont("Redial", "", 32);
		btnRedial->setPosition(10 - ScreenCenter.x, 90 - ScreenCenter.y);
		btnRedial->setAnchorPoint({ 0, 0.5 });
		btnRedial->setGlobalZOrder(1000);
		cc_scene->addChild(btnRedial);

		listenerRedial = cocos2d::EventListenerTouchOneByOne::create();
		listenerRedial->onTouchBegan = [this](cocos2d::Touch * t, cocos2d::Event * e) {
			auto&& tL = t->getLocation();
			auto&& p = btnRedial->convertToNodeSpace(tL);
			auto&& s = btnRedial->getContentSize();
			cocos2d::Rect r{ 0,0, s.width, s.height };
			auto&& b = r.containsPoint(p);
			if (b) {
				if (this->dialer->peer) {
					this->dialer->peer->Dispose();
				}
			}
			return b;
		};
		cocos2d::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerRedial, btnRedial);
	}
	{
		btnAutoFire = cocos2d::Label::createWithSystemFont("", "", 32);
		btnAutoFire->setPosition(10 - ScreenCenter.x, 180 - ScreenCenter.y);
		btnAutoFire->setAnchorPoint({ 0, 0.5 });
		btnAutoFire->setGlobalZOrder(1000);
		cc_scene->addChild(btnAutoFire);
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
	}
	{
		labelNumDialTimes = cocos2d::Label::createWithSystemFont("", "", 32);
		labelNumDialTimes->setPosition(10 - ScreenCenter.x, 150 - ScreenCenter.y);
		labelNumDialTimes->setAnchorPoint({ 0, 0.5 });
		labelNumDialTimes->setGlobalZOrder(1000);
		cc_scene->addChild(labelNumDialTimes);
	}
	{
		labelNumFishs = cocos2d::Label::createWithSystemFont("", "", 32);
		labelNumFishs->setPosition(10 - ScreenCenter.x, 120 - ScreenCenter.y);
		labelNumFishs->setAnchorPoint({ 0, 0.5 });
		labelNumFishs->setGlobalZOrder(1000);
		cc_scene->addChild(labelNumFishs);
	}
	{
		labelPing = cocos2d::Label::createWithSystemFont("", "", 32);
		labelPing->setPosition(10 - ScreenCenter.x, 210 - ScreenCenter.y);
		labelPing->setAnchorPoint({ 0, 0.5 });
		labelPing->setGlobalZOrder(1000);
		cc_scene->addChild(labelPing);
	}
}

inline void Panel_Dialer::SetText_AutoFire(bool const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!btnAutoFire) return;
	std::string s = "auto fire: ";
	s += value ? "ON" : "OFF";
	btnAutoFire->setString(s);
}
inline void Panel_Dialer::SetText_NumDialTimes(int64_t const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelNumDialTimes) return;
	labelNumDialTimes->setString("reconnect times: " + std::to_string(value));
}
inline void Panel_Dialer::SetText_Ping(int64_t const& value) noexcept {
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
inline void Panel_Dialer::SetText_NumFishs(size_t const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelNumFishs) return;
	labelNumFishs->setString("num fishs: " + std::to_string(value));
}
inline void Panel_Dialer::SetText_Server(std::string const& value) noexcept {
	assert(!::catchFish->disposed);
	if (!labelServer) return;
	labelServer->setString(value);
}
