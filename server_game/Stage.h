struct Emitter1 : PKG::CatchFish::Stages::Emitter1 {
	using BaseType = PKG::CatchFish::Stages::Emitter1;
	using BaseType::BaseType;

	// fill by InitCascade
	Scene* scene = nullptr;

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& ticks) noexcept override;
};

struct Monitor1 : PKG::CatchFish::Stages::Monitor1 {
	using BaseType = PKG::CatchFish::Stages::Monitor1;
	using BaseType::BaseType;

	// fill by InitCascade
	Scene* scene = nullptr;

	// 大鱼计数器. InitCascade 时初始化. 鱼生成后塞入其 counters
	std::shared_ptr<int> counter;

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& ticks) noexcept override;
};
