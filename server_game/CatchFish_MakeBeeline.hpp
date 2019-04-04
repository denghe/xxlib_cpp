// 填充随机生成的有限角度的能立即出现在屏幕上的线段路径
// -45 ~ 45, 135 ~ 225 在这两段角度之间随机一个角度值,  + 180 之后的 45 度范围内再次随机一个角度, 用旋转函数转为两个坐标点. 连为1根直线, 最后找出安全出生框与直线的交点
// 由于最终计算出两个交点之后, 可以通过交换顺序的方式反向, 故只需要一段角度作为起始角度即可. 简化起见, 直接 135 ~ 225 ( 不考虑开区间误差 )
inline PKG::CatchFish::Way_s CatchFish::MakeBeeline(float const& itemRadius) {
	auto&& w = (ScreenWidth + itemRadius) / 2.0f;
	auto&& h = (ScreenHeight + itemRadius) / 2.0f;
	auto&& a = scene->rnd->Next(180);
	if (a < 90) {
		a -= 45;
	}
	else {
		a = a - 90 + 135;
	}
	auto&& p1 = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	xx::Pos abs{ std::fabs(p1.x), std::fabs(p1.y) };
	if (abs.x / (abs.x + abs.y) > ScreenWidthRatio) {
		p1 = p1 * (w / abs.x);
	}
	else {
		p1 = p1 * (h / abs.y);
	}
	a = a + 180 - 23 + scene->rnd->Next(46);
	auto&& p2 = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	abs.x = std::fabs(p2.x);
	abs.y = std::fabs(p2.y);
	if (abs.x / (abs.x + abs.y) > ScreenWidthRatio) {
		p2 = p2 * (w / abs.x);
	}
	else {
		p2 = p2 * (h / abs.y);
	}
	auto&& way = xx::Make<PKG::CatchFish::Way>();
	xx::MakeTo(way->points);
	way->points->Add(PKG::CatchFish::WayPoint{ p1, xx::GetAngle(p1, p2) * (180.0f / float(M_PI)), p1.distance(p2) });
	way->points->Add(PKG::CatchFish::WayPoint{ p2, 0, 0 });	// 非循环轨迹最后个点距离和角度不用计算, 也不做统计
	way->distance = way->points->At(0).distance;
	way->loop = false;
	return way;
}
