inline int Fish::InitCascade(void* const& o) noexcept {
	scene = (Scene*)o;
	assert(!cfg);
	cfg = &*scene->cfg->fishs->At(cfgId);
	if (int r = this->BaseType::InitCascade(o)) return r;
#ifdef CC_TARGET_PLATFORM
	DrawInit();
#endif
	return 0;
}

inline int Fish::Update(int const& frameNumber) noexcept {
	// 取到当前帧结束时应该前进的距离
	auto&& dist = cfg->moveFrames->At(spriteFrameIndex)->moveDistance * speedScale;

	// 帧下标循环前进
	if (++spriteFrameIndex == cfg->moveFrames->len) {
		spriteFrameIndex = 0;
	}

	// 定位到轨迹
	auto&& way = this->way ? this->way : cfg->ways->At(wayIndex);

LabKeepMoving:
	// 试累加鱼当前鱼点移动距离. 如果跨点, 则用多个点的距离来凑够
	// 如果累加距离之后跨点了, 则将 dist 视作可能跨更多点的距离容器, 继续用下一个点的距离从中减去

	auto&& p = &way->points->At(wayPointIndex);
	auto&& left = p->distance - wayPointDistance;
	if (dist > left) {
		// 循环鱼线: 最后一个点指向第一个点
		if (way->loop) {
			// 从 dist 中减去当前鱼线点剩余距离
			dist -= left;
			wayPointDistance = 0;

			// 指向下一个鱼线点, 如果到终点, 就指向起点
			if (++wayPointIndex == way->points->len) {
				wayPointIndex = 0;
			}

			// 继续 while 从 dist 减 p->distance
			goto LabKeepMoving;
		}
		// 如果还有下一个鱼线点存在( 最后一个点不算 )
		else if (wayPointIndex + 2 < (int)way->points->len) {
			// 从 dist 中减去当前鱼线点剩余距离
			dist -= left;
			wayPointDistance = 0;

			// 指向下一个鱼线点
			++wayPointIndex;

			// 继续 while 从 dist 减 p->distance
			goto LabKeepMoving;
		}
		else
			return -1;			// 通知删鱼
	}
	else {
		wayPointDistance += dist;
	}

	// 按当前鱼线点上已经前进的距离, 结合下一个点的坐标, 按比例修正 p 坐标 & 角度
	if (wayPointIndex == way->points->len - 1) {
		auto&& np = &way->points->At(0);
		pos = p->pos + ((np->pos - p->pos) * (wayPointDistance / p->distance));
	}
	else {
		pos = p->pos + (((p + 1)->pos - p->pos) * (wayPointDistance / p->distance));
	}

	angle = p->angle;
#ifdef CC_TARGET_PLATFORM
	DrawUpdate();
#endif
	return 0;
};

int Fish::HitCheck(Bullet* const& bullet) noexcept {
	// 计算出鱼中心与子弹中心的距离
	auto d = pos - bullet->pos;
	auto&& d2 = d.x * d.x + d.y * d.y;

	// 1 判: 如果有配置最小半径, 即位于圆心的这个半径内 非空心, 一定命中, 如果距离小于两半径和， 则判定成立
	if (cfg->minDetectRadius > 0) {
		auto&& r2 = cfg->minDetectRadius * cfg->scale * this->scale + bullet->cfg->radius * bullet->cfg->scale;
		if (r2 * r2 > d2) return 1;
	}
	// 2 判: 如果有配置最大半径, 即只要进入这个范围, 就可以进一步物理判断, 如果距离超过, 则判定失败
	if (cfg->maxDetectRadius > 0) {
		auto&& r2 = cfg->maxDetectRadius * cfg->scale * this->scale + bullet->cfg->radius * bullet->cfg->scale;
		if (r2 * r2 < d2) return 0;
	}
	// 3 判: 物理检测. 用子弹半径经过坐标转换, 去物理 space 选取 shapes. 如果有选到, 则判定成功
	auto&& space = xx::As<Physics>(cfg->moveFrames->At(spriteFrameIndex)->physics)->space;
	auto&& s = cfg->scale * this->scale;
	auto&& p = xx::Rotate(xx::Pos{ sqrtf(d2) / s, 0 }, this->angle - xx::GetAngle(pos, bullet->pos));
	if (cpSpacePointQueryNearest(space, cpv(p.x, p.y), cpFloat(bullet->cfg->radius * bullet->cfg->scale / s), CP_SHAPE_FILTER_ALL, nullptr)) {
		return 1;
	}
	return 0;
}

inline Fish::~Fish() {
#ifdef CC_TARGET_PLATFORM
	DrawDispose();
#endif
}

#ifdef CC_TARGET_PLATFORM
inline void Fish::DrawInit() noexcept {
	assert(!body);
	body = cocos2d::Sprite::create();
	body->setGlobalZOrder(cfg->zOrder);
	body->retain();

	shadow = cocos2d::Sprite::create();
	shadow->setGlobalZOrder(cfg->zOrder);
	shadow->setColor(cocos2d::Color3B::BLACK);
	shadow->setOpacity(125);
	shadow->retain();
#if DRAW_PHYSICS_POLYGON
	debugNode = cocos2d::DrawNode::create();
	debugNode->setGlobalZOrder(cfg->zOrder);
	debugNode->retain();
#endif
	DrawUpdate();
	cc_scene->addChild(shadow);
	cc_scene->addChild(body);
#if DRAW_PHYSICS_POLYGON
	cc_scene->addChild(debugNode);
#endif
}

inline void Fish::DrawUpdate() noexcept {
	assert(body);
	auto&& sf = xx::As<SpriteFrame>(cfg->moveFrames->At(spriteFrameIndex)->frame)->spriteFrame;

	auto&& a = -angle * (180.0f / float(M_PI));

	// 设鱼的帧图, 坐标, 方向, 缩放
	body->setSpriteFrame(sf);
	body->setRotation(a);
	body->setPosition(pos);
	body->setScale(scale * cfg->scale);

	shadow->setSpriteFrame(sf);
	shadow->setRotation(a);
	shadow->setPosition(pos + cfg->shadowOffset * scale * cfg->scale);
	shadow->setScale(scale * cfg->scale * cfg->shadowScale);
#if DRAW_PHYSICS_POLYGON
	// 碰撞多边形显示
	debugNode->setPosition(pos);
	debugNode->setRotation(a);
	debugNode->setScale(scale * cfg->scale);
	auto && sfs = cfg->moveFrames->At(spriteFrameIndex);
	if (sfs->physics) {
		debugNode->clear();
		debugNode->drawCircle({ 0,0 }, cfg->maxDetectRadius, 0, 50, true, cocos2d::Color4F::RED);
		for (auto&& polygon : *sfs->physics->polygons) {
			for (size_t i = 0; i < polygon->len - 1; ++i) {
				debugNode->drawLine(polygon->At(i), polygon->At(i + 1), cocos2d::Color4F::GREEN);
			}
			debugNode->drawLine(polygon->At(0), polygon->At(polygon->len - 1), cocos2d::Color4F::GREEN);
		}
	}
#endif
}

inline void Fish::DrawDispose() noexcept {
	if (!body) return;

	if (body->getParent()) {
		body->removeFromParent();
	}
	body->release();
	body = nullptr;

	if (shadow->getParent()) {
		shadow->removeFromParent();
	}
	shadow->release();
	shadow = nullptr;

#if DRAW_PHYSICS_POLYGON
	if (debugNode->getParent()) {
		debugNode->removeFromParent();
	}
	debugNode->release();
	debugNode = nullptr;
#endif
}
#endif
