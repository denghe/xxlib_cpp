inline int CatchFish::Update() {
	scene->Update();

	// 模拟关卡 鱼发生器. 每 10 帧生成一条
	if (scene->frameNumber % 10 == 0) {
		scene->fishs->Add(MakeRandomFish());
	}

	return 0;
}
