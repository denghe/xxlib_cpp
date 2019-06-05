#ifdef CC_TARGET_PLATFORM
inline void PKG::CatchFish::BombWeapon::DrawInit() noexcept {
	assert(!body);
	body = cocos2d::DrawNode::create();
	body->setLocalZOrder(cfg->zOrder);
	body->drawSolidCircle(cocos2d::Vec2::ZERO, 100, 0, 100, 1, 1, cocos2d::Color4F::RED);
	DrawUpdate();
	cc_fishNode->addChild(body);
}
inline void PKG::CatchFish::BombWeapon::DrawUpdate() noexcept {
	assert(body);
	// 碰撞多边形显示
	body->setPosition(pos);
}
#endif
