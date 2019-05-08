inline int PKG::CatchFish::BigFish::Move() noexcept {
	int r = this->BaseType::Move();
	return r;
}

inline int PKG::CatchFish::BigFish::HitCheck(PKG::CatchFish::Bullet* const& bullet) noexcept {
	for (auto&& f : *childs) {
		if (int r = f->HitCheck(bullet)) return r;
	}
	return this->BaseType::HitCheck(bullet);
}
