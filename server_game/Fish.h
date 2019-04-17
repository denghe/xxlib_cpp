struct Bullet;
struct Fish : PKG::CatchFish::Fish {
	using BaseType = PKG::CatchFish::Fish;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

	// 指向具体配置
	PKG::CatchFish::Configs::Fish* cfg = nullptr;

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;

	virtual int HitCheck(Bullet* const& bullet) noexcept;

#ifdef CC_TARGET_PLATFORM
	RefHolder<cocos2d::Sprite> body;
	RefHolder<cocos2d::Sprite> shadow;
#if DRAW_PHYSICS_POLYGON
	RefHolder<cocos2d::DrawNode> debugNode;
#endif
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
#endif
};
using Fish_s = std::shared_ptr<Fish>;
using Fish_w = std::weak_ptr<Fish>;
