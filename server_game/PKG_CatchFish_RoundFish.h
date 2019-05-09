xx::Pos tarPos;
float tarRadius;
float angleInc;

// 围绕 tarPos 做圆周运动。参数传入 与 tarPos 的距离, 每帧的角度增量
virtual int Move() noexcept override;
