#ifdef CC_TARGET_PLATFORM
inline int PKG::CatchFish::Weapon::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	assert(player);
	assert(!cfg);
	cfg = &*scene->cfg->weapons->At(cfgId);
	if (int r = InitCascadeCore(o)) return r;
	DrawInit();
	return 0;
}

inline void PKG::CatchFish::Weapon::DrawInit() noexcept {
	assert(false);
}
inline void PKG::CatchFish::Weapon::DrawUpdate() noexcept {
	assert(false);
}
#endif
