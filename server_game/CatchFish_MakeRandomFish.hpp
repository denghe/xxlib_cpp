Fish_s CatchFish::MakeRandomFish() {
	auto&& fish = xx::Make<Fish>();
	fish->id = ++scene->autoIncId;
	fish->cfgId = 0;
	fish->coin = 3;			// if minCoin < maxCoin    scene->rnd->Next(cfg->minCoin, cfg->maxCoin + 1);
	fish->speedScale = 1;
	fish->scale = 1;
	fish->wayIndex = 0;
	fish->wayPointIndex = 0;
	fish->wayPointDistance = 0;
	fish->spriteFrameIndex = 0;
	fish->frameRatio = 1;
	fish->reverse = false;
	fish->way = MakeBeeline(cfg->fishs->At(0)->maxDetectRadius);		// 先随便生成一条轨迹

	auto&& p = fish->way->points->At(fish->wayPointIndex);
	fish->pos = p.pos;
	fish->angle = p.angle;

	fish->InitCascade(&*scene);
	return fish;
}
