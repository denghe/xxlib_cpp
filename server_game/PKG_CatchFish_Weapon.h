// 所在场景
PKG::CatchFish::Scene* scene = nullptr;

// 所在玩家( 由 Player 或收到 Push 时预填充 )
PKG::CatchFish::Player* player = nullptr;

// 指向具体配置
PKG::CatchFish::Configs::Weapon* cfg = nullptr;

#ifdef CC_TARGET_PLATFORM
int InitCascade(void* const& o) noexcept override;
virtual void DrawInit() noexcept;
virtual void DrawUpdate() noexcept;
#endif
