struct Emitter_RandomFishs : PKG::CatchFish::Stages::Emitter_RandomFishs {
	using BaseType = PKG::CatchFish::Stages::Emitter_RandomFishs;
	using BaseType::BaseType;

	// fill by InitCascade
	Scene* scene = nullptr;

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& ticks) noexcept override;
};

struct Monitor_KeepBigFish : PKG::CatchFish::Stages::Monitor_KeepBigFish {
	using BaseType = PKG::CatchFish::Stages::Monitor_KeepBigFish;
	using BaseType::BaseType;

	// fill by InitCascade
	Scene* scene = nullptr;

	// 大鱼计数器. InitCascade 时初始化. 鱼生成后塞入其 counters
	std::shared_ptr<int> counter;

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& ticks) noexcept override;
};

struct Emitter_RingFishs : PKG::CatchFish::Stages::Emitter_RingFishs {
	using BaseType = PKG::CatchFish::Stages::Emitter_RingFishs;
	using BaseType::BaseType;

	// fill by InitCascade
	Scene* scene = nullptr;

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& ticks) noexcept override;
};
