inline PKG::CatchFish::Configs::ColorFish* PKG::CatchFish::ColorFish::Cfg() const noexcept {
	assert(dynamic_cast<PKG::CatchFish::Configs::ColorFish*>(this->cfg));
	return (PKG::CatchFish::Configs::ColorFish*)this->cfg;
}

#ifdef CC_TARGET_PLATFORM
inline void PKG::CatchFish::ColorFish::DrawInit() noexcept {
	this->BaseType::DrawInit();
	auto&& cfg = Cfg();
	body->setColor(cocos2d::Color3B{ cfg->r, cfg->g, cfg->b });
}
#endif
