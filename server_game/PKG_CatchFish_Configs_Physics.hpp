inline int PKG::CatchFish::Configs::Physics::InitCascade(void* const& o) noexcept {
	if (space) return 0;											// 多鱼共图
	static_assert(sizeof(cpVect) == sizeof(xx::Pos));
	if (polygons) {
		space = cpSpaceNew();
		auto&& body = cpSpaceGetStaticBody(space);
		for (auto&& polygon : *polygons) {
			auto&& shape = cpPolyShapeNew(body, (int)polygon->len, (cpVect*)polygon->buf, cpTransformIdentity, 0.0);
			cpSpaceAddShape(space, shape);
		}
	}
	return 0;
}

inline PKG::CatchFish::Configs::Physics::~Physics() {
	if (space) {
		cpSpaceEachShape(space, [](cpShape *shape, void *data) {
			cpSpaceAddPostStepCallback((cpSpace*)data, [](cpSpace *space, void *key, void *data) {
				cpSpaceRemoveShape(space, (cpShape *)key);
				cpShapeFree((cpShape *)key);
			}, shape, nullptr);
		}, space);
		cpSpaceFree(space);
		space = nullptr;
	}
}
