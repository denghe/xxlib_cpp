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
	cocos2d::Director::getInstance()->getEventDispatcher()->removeEventListener(cc_listener);
	cc_listener = nullptr;
#endif
}

inline CatchFish::~CatchFish() {
	Dispose(0);
}
