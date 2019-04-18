template<typename T, typename ENABLED = std::enable_if_t<std::is_base_of_v<cocos2d::Ref, T>>>
struct RefHolder {
	T* t = nullptr;
	RefHolder(T* const& t = nullptr) {
		this->operator=(t);
	}
	RefHolder& operator=(T* const& t) {
		if (this->t == t) return *this;
		Reset();
		this->t = t;
		if (t) {
			t->retain();
		}
		return *this;
	}
	RefHolder(RefHolder const& o) = delete;
	RefHolder(RefHolder&& o) {
		std::swap(t, o.t);
	}
	RefHolder& operator=(RefHolder&& o) {
		std::swap(t, o.t);
	}
	T* operator->() {
		return t;
	}
	operator T* () {
		return t;
	}
	operator bool() {
		return (bool)t;
	}
	void Reset() {
		if (t) {
			if constexpr (std::is_base_of_v<cocos2d::Node, T>) {
				if (t->getParent()) {
					t->removeFromParent();
				}
			}
			if constexpr (std::is_base_of_v<cocos2d::EventListener, T>) {
				cocos2d::Director::getInstance()->getEventDispatcher()->removeEventListener(t);
			}
			t->release();
			t = nullptr;
		}
	}
	~RefHolder() {
		Reset();
	}
};
