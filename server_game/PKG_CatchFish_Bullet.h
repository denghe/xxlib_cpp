// 所在场景
PKG::CatchFish::Scene* scene = nullptr;

// 所在玩家( 由 Cannon 预填充 )
PKG::CatchFish::Player* player = nullptr;

// 所属炮台( 由 Cannon 预填充 )
PKG::CatchFish::Cannon* cannon = nullptr;

// 指向具体配置( 由 Cannon 预填充, Bullet 与 Cannon 共享配置 )
PKG::CatchFish::Configs::Cannon* cfg = nullptr;

virtual int Update(int const& frameNumber) noexcept override;

// 移动子弹。如果生命周期结束将返回非 0
int Move() noexcept;

#ifdef CC_TARGET_PLATFORM

int InitCascade(void* const& o) noexcept override;

RefHolder<cocos2d::Sprite> body;
virtual void DrawInit() noexcept;
virtual void DrawUpdate() noexcept;
#endif
