struct Bullet : PKG::CatchFish::Bullet {
	using BaseType = PKG::CatchFish::Bullet;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

	// 所在玩家( 由 Cannon 预填充 )
	Player* player = nullptr;

	// 所属炮台( 由 Cannon 预填充 )
	Cannon* cannon = nullptr;

	// 指向具体配置( 由 Cannon 预填充, Bullet 与 Cannon 共享配置 )
	PKG::CatchFish::Configs::Cannon* cfg = nullptr;

	int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;

	// 移动子弹。如果生命周期结束将返回非 0
	int Move() noexcept;

#ifdef CC_TARGET_PLATFORM
	RefHolder<cocos2d::Sprite> body;
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
#endif
};
using Bullet_s = std::shared_ptr<Bullet>;
using Bullet_w = std::weak_ptr<Bullet>;
