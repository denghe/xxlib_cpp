inline int PKG::CatchFish::WayFish::Move() noexcept {
	// 取到当前帧结束时应该前进的距离
	auto&& dist = cfg->moveFrames->At(spriteFrameIndex)->moveDistance * speedScale;

	// 帧下标循环前进
	if (++spriteFrameIndex == cfg->moveFrames->len) {
		spriteFrameIndex = 0;
	}

	// 定位到轨迹
	PKG::CatchFish::Way* way = nullptr;
	if (this->way) {
		way = &*this->way;
	}
	else {
		way = &*scene->cfg->ways[wayTypeIndex][wayIndex];
	}

LabKeepMoving:
	// 试累加鱼当前路点移动距离. 如果跨点, 则用多个点的距离来凑够
	// 如果累加距离之后跨点了, 则将 dist 视作可能跨更多点的距离容器, 继续用下一个点的距离从中减去

	auto&& p = &way->points->At(wayPointIndex);
	auto&& left = p->distance - wayPointDistance;
	if (dist > left) {
		// 循环路径: 最后一个点指向第一个点
		if (way->loop) {
			// 从 dist 中减去当前路点剩余距离
			dist -= left;
			wayPointDistance = 0;

			// 指向下一个路点, 如果到终点, 就指向起点
			if (++wayPointIndex == way->points->len) {
				wayPointIndex = 0;
			}

			// 继续 while 从 dist 减 p->distance
			goto LabKeepMoving;
		}
		// 如果还有下一个路点存在( 最后一个点不算 )
		else if (wayPointIndex + 2 < (int)way->points->len) {
			// 从 dist 中减去当前路点剩余距离
			dist -= left;
			wayPointDistance = 0;

			// 指向下一个路点
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

	// 按当前路点上已经前进的距离, 结合下一个点的坐标, 按比例修正 p 坐标 & 角度
	if (wayPointIndex == way->points->len - 1) {
		auto&& np = &way->points->At(0);
		pos = p->pos + ((np->pos - p->pos) * (wayPointDistance / p->distance));
	}
	else {
		pos = p->pos + (((p + 1)->pos - p->pos) * (wayPointDistance / p->distance));
	}

	angle = p->angle;
	return 0;
}
