inline int SpriteFrame::InitCascade(void* const& o) noexcept {
#ifdef CC_TARGET_PLATFORM
	auto&& sfc = cocos2d::SpriteFrameCache::getInstance();
	sfc->addSpriteFramesWithFile(*this->plistName);
	spriteFrame = sfc->getSpriteFrameByName(*this->frameName);
	if (!spriteFrame) return -1;
	spriteFrame->retain();
#endif
	return 0;
}

inline SpriteFrame::~SpriteFrame() {
#ifdef CC_TARGET_PLATFORM
	if (spriteFrame) {
		spriteFrame->release();
		spriteFrame = nullptr;
	}
	// todo: 卸载贴图?
#endif
}
