inline int Physics::InitCascade(void* const& o) noexcept {
	assert(!space);
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

inline Physics::~Physics() {
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
