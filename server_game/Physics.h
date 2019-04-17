struct Physics : PKG::CatchFish::Configs::Physics {
	using BaseType = PKG::CatchFish::Configs::Physics;
	using BaseType::BaseType;

	cpSpace* space = nullptr;

	virtual int InitCascade(void* const& o) noexcept override;
	~Physics();
};
using Physics_s = std::shared_ptr<Physics>;
using Physics_w = std::weak_ptr<Physics>;
