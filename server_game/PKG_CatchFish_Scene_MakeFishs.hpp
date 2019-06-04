inline PKG::CatchFish::WayFish_s PKG::CatchFish::Scene::MakeRandomFish(int const& fishId, int64_t const& coin, float const& scaleFrom, float const& scaleTo) noexcept {
	auto&& fishCfg = cfg->fishs->At(0);//rnd->Next((int)cfg->fishs->len));

	auto&& fish = xx::Make<PKG::CatchFish::WayFish>();
	fish->scene = this;
	fish->id = fishId;	// ++autoIncId;
	fish->cfgId = fishCfg->id;
	fish->cfg = &*fishCfg;
	if (coin) {
		fish->coin = coin;
	}
	else {
		if (fishCfg->minCoin < fishCfg->maxCoin) {
			fish->coin = rnd->Next((int)fishCfg->minCoin, (int)fishCfg->maxCoin + 1);
		}
		else {
			fish->coin = fishCfg->minCoin;
		}
	}
	fish->scale = (float)rnd->NextDouble(scaleFrom, scaleTo);
	fish->speedScale = 1 + (float)rnd->NextDouble() * 2;
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;

	fish->reverse = false;
	fish->wayPointIndex = 0;
	fish->wayPointDistance = 0;
	//fish->way = MakeBeeline(MakeRandomInOutPoint(fishCfg->maxDetectRadius * fishCfg->scale));
	fish->wayTypeIndex = 1 + rnd->Next(2);										// 1: 曲线, 2: 直线
	fish->wayIndex = rnd->Next((int)cfg->ways[fish->wayTypeIndex].len);			// 下标

	auto&& p = cfg->ways[fish->wayTypeIndex][fish->wayIndex]->points->At(fish->wayPointIndex);
	fish->pos = p.pos;
	fish->angle = p.angle;
	return fish;
}



inline PKG::CatchFish::BigFish_s PKG::CatchFish::Scene::MakeRandomBigFish(int const& fishId) noexcept {
	auto&& fishCfg = xx::As<PKG::CatchFish::Configs::BigFish>(cfg->fishs->At(1));
	assert(fishCfg);

	auto&& fish = xx::Make<PKG::CatchFish::BigFish>();
	fish->scene = this;
	fish->id = fishId;	// ++autoIncId;
	fish->cfgId = fishCfg->id;
	fish->cfg = &*fishCfg;
	if (fishCfg->minCoin < fishCfg->maxCoin) {
		fish->coin = rnd->Next((int)fishCfg->minCoin, (int)fishCfg->maxCoin + 1);
	}
	else {
		fish->coin = fishCfg->minCoin;
	}
	fish->scale = 1;
	fish->speedScale = 1;
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;

	auto&& rfCfg = cfg->fishs->At(0);
	float rfScale = 1;
	float fishRadius = fishCfg->maxDetectRadius * fishCfg->scale * fish->scale;
	float rfRadius = rfCfg->maxDetectRadius * rfCfg->scale * rfScale;
	float rfDistance = fishRadius - rfRadius * 2;

	auto&& posFromTo = MakeRandomInOutPoint(rfDistance);
	fish->angle = xx::GetAngle(posFromTo);
	fish->pos = posFromTo.first;
	fish->moveInc = xx::Rotate({ fishCfg->moveFrameDistance, 0 }, fish->angle);

	xx::MakeTo(fish->childs);
	float aStep = (float)(M_PI * 2 / fishCfg->numChilds);
	for (auto&& i = 0; i < fishCfg->numChilds; ++i) {
		auto&& rf = xx::Make<PKG::CatchFish::RoundFish>();
		rf->scene = this;
		//rf->id;
		rf->cfgId = rfCfg->id;
		rf->cfg = &*rfCfg;
		//rf->coin;
		rf->scale = rfScale;
		rf->speedScale = 1;
		rf->spriteFrameIndex = 0;
		rf->frameRatio = 1;

		rf->tarAngle = aStep * (float)i;
		rf->pos = fish->pos + xx::Rotate({ rfDistance, 0 }, rf->tarAngle);
		rf->angle = rf->tarAngle + (fishCfg->childsAngleInc > 0 ? (float)M_PI_2 : -(float)M_PI_2);
		//rf->moveInc

		fish->childs->Add(std::move(rf));
	}

	return fish;
}




inline PKG::CatchFish::BombFish_s PKG::CatchFish::Scene::MakeRandomBombFish(int const& fishId) noexcept {
	auto&& fishCfg = xx::As<PKG::CatchFish::Configs::ColorFish>(cfg->fishs->At(2));
	assert(fishCfg);

	auto&& fish = xx::Make<PKG::CatchFish::BombFish>();
	fish->scene = this;
	fish->id = fishId;
	fish->cfgId = fishCfg->id;
	fish->cfg = &*fishCfg;
	if (fishCfg->minCoin < fishCfg->maxCoin) {
		fish->coin = rnd->Next((int)fishCfg->minCoin, (int)fishCfg->maxCoin + 1);
	}
	else {
		fish->coin = fishCfg->minCoin;
	}
	fish->scale = 1;
	fish->speedScale = 1;
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;

	float fishRadius = fishCfg->maxDetectRadius * fishCfg->scale * fish->scale;
	auto&& posFromTo = MakeRandomInOutPoint(fishRadius);
	fish->angle = xx::GetAngle(posFromTo);
	fish->pos = posFromTo.first;
	fish->moveInc = xx::Rotate({ fishCfg->moveFrameDistance, 0 }, fish->angle);

	return fish;
}




// 生成一条随机角度的进出口( 主用于体积大于 cfg ways 设定的移动对象 )
// -45 ~ 45, 135 ~ 225 在这两段角度之间随机一个角度值,  + 180 之后的 45 度范围内再次随机一个角度, 用旋转函数转为两个坐标点. 连为1根直线, 最后找出安全出生框与直线的交点
// 由于最终计算出两个交点之后, 可以通过交换顺序的方式反向, 故只需要一段角度作为起始角度即可. 简化起见, 直接 135 ~ 225 ( 不考虑开区间误差 )
inline std::pair<xx::Pos, xx::Pos> PKG::CatchFish::Scene::MakeRandomInOutPoint(float const& itemRadius) noexcept {
	std::pair<xx::Pos, xx::Pos> rtv;
	auto&& w = designSize_2.x + itemRadius;
	auto&& h = designSize_2.y + itemRadius;
	auto&& a = rnd->Next(180);
	if (a < 90) {
		a -= 45;
	}
	else {
		a = a - 90 + 135;
	}
	rtv.first = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	xx::Pos abs{ std::fabs(rtv.first.x), std::fabs(rtv.first.y) };
	if (abs.x / (abs.x + abs.y) > designWidthRatio) {
		rtv.first = rtv.first * (w / abs.x);
	}
	else {
		rtv.first = rtv.first * (h / abs.y);
	}
	a = a + 180 - 23 + rnd->Next(46);
	rtv.second = xx::Rotate(xx::Pos{ 1, 0 }, a * (float(M_PI) / 180.0f));
	abs.x = std::fabs(rtv.second.x);
	abs.y = std::fabs(rtv.second.y);
	if (abs.x / (abs.x + abs.y) > designWidthRatio) {
		rtv.second = rtv.second * (w / abs.x);
	}
	else {
		rtv.second = rtv.second * (h / abs.y);
	}
	return rtv;
}

