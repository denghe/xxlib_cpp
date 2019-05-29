// 将配置转为具体类型
PKG::CatchFish::Configs::ColorFish* Cfg() const noexcept;

#ifdef CC_TARGET_PLATFORM
virtual void DrawInit() noexcept override;
#endif
