// fill by InitCascade
PKG::CatchFish::Scene* scene = nullptr;

// 计数器. InitCascade 时初始化. 鱼生成后塞入其 counters
std::shared_ptr<int> counter;

virtual int InitCascade(void* const& o) noexcept override;
virtual int Update(int const& ticks) noexcept override;

#ifndef CC_TARGET_PLATFORM
virtual PKG::CatchFish::Fish_s MakeFish() noexcept;
#endif
