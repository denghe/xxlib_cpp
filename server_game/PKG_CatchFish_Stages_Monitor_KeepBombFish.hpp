#ifndef CC_TARGET_PLATFORM
inline PKG::CatchFish::Fish_s PKG::CatchFish::Stages::Monitor_KeepBombFish::MakeFish() noexcept {
	return scene->MakeRandomBombFish(--scene->autoDecId);
}
#endif
