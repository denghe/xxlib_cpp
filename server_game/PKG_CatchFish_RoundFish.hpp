inline int PKG::CatchFish::RoundFish::Move() noexcept {
	// 每帧帧递增角度( 以 tarPos 为原点，计算到 this.Pos 的角度，加上该值 得到 newAngle, 计算出基于新角度的坐标. this.angle = newAngle +- 90度 垂直显示
	tarAngle += angleInc;
	pos = tarPos + xx::Rotate({ tarRadius + cfg->maxDetectRadius * cfg->scale * scale, 0 }, tarAngle);
	angle = tarAngle + (angleInc > 0 ? (float)M_PI_2 : -(float)M_PI_2);

	// 帧下标循环前进
	if (++spriteFrameIndex == cfg->moveFrames->len) {
		spriteFrameIndex = 0;
	}

	return 0;
}
