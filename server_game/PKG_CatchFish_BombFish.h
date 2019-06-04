#ifndef CC_TARGET_PLATFORM

// 将配置转为具体类型
PKG::CatchFish::Configs::ColorFish* Cfg() const noexcept;

// 返回 0: 变成 weapon
virtual int Die(PKG::CatchFish::Bullet* const& b) noexcept override;

#endif
