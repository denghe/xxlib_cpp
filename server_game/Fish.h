struct Bullet;

struct IHitCheck {
	virtual int HitCheck(Bullet* const& bullet) noexcept = 0;
};

struct Fish : PKG::CatchFish::Fish, IHitCheck {
	using BaseType = PKG::CatchFish::Fish;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

	// 指向具体配置
	PKG::CatchFish::Configs::Fish* cfg = nullptr;

	// 执行简单移动逻辑
	virtual int Update(int const& frameNumber) noexcept override;

	// 碰撞检测。如果与传入子弹相撞则返回 1
	virtual int HitCheck(Bullet* const& bullet) noexcept override;

#ifdef CC_TARGET_PLATFORM
	virtual int InitCascade(void* const& o) noexcept override;

	RefHolder<cocos2d::Sprite> body;
	RefHolder<cocos2d::Sprite> shadow;
#if DRAW_PHYSICS_POLYGON
	RefHolder<cocos2d::DrawNode> debugNode;
#endif
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
#else
	// 分类统计计数器集合( 利用引用计数来统计个数 )
	xx::List<std::shared_ptr<int>> counters;
#endif
};
using Fish_s = std::shared_ptr<Fish>;
using Fish_w = std::weak_ptr<Fish>;



struct WayFish : PKG::CatchFish::WayFish, IHitCheck {
	using BaseType = PKG::CatchFish::WayFish;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

	// 指向具体配置
	PKG::CatchFish::Configs::Fish* cfg = nullptr;

	// 沿着 Way 做一次移动
	virtual int Move() noexcept;

	// 执行移动逻辑( frameRatio 控制了 Move 次数 )
	virtual int Update(int const& frameNumber) noexcept override;

	// 碰撞检测。如果与传入子弹相撞则返回 1
	virtual int HitCheck(Bullet* const& bullet) noexcept override;

#ifdef CC_TARGET_PLATFORM
	virtual int InitCascade(void* const& o) noexcept override;

	RefHolder<cocos2d::Sprite> body;
	RefHolder<cocos2d::Sprite> shadow;
#if DRAW_PHYSICS_POLYGON
	RefHolder<cocos2d::DrawNode> debugNode;
#endif
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
#else
	// 分类统计计数器集合( 利用引用计数来统计个数 )
	xx::List<std::shared_ptr<int>> counters;
#endif
};
using WayFish_s = std::shared_ptr<WayFish>;
using WayFish_w = std::weak_ptr<WayFish>;
