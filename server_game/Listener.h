struct Service {
	// 多协议监听器
	xx::UvListener_s listener;

	// 当前只有一个游戏实例
	std::shared_ptr<CatchFish> catchFish;

	// 用于自增产生玩家 id
	int playerAutoId = 0;

	// 定时器. 模拟一个游戏循环
	xx::UvTimer_s looper;

	int64_t ticksPool = 0;
	int64_t lastTicks = xx::NowEpoch10m();
	// 比 60 略快以兼容部分硬件显示刷新不标准, 超过 60 帧的垃圾设备, 例如部分华为高端型号
	const int64_t ticksPerFrame = int64_t(10000000 / 60.3);


	// calc 服务连接
	xx::UvPeer_s calcPeer;

	// 方便判断 calcPeer 是否存活( 连接成功状态 )
	bool IsAlive_CalcPeer();

	// calc 服务拨号器( 在 loop 中 check 如果发现 !calcPeer || calcPeer->Disposed() 就 Dial )
	xx::UvDialer_s calcDialer;

	// 正在拨号的标志位
	bool dialing = false;

	// 拨号到 calc 服务器
	void Dial_Calc();

	Service();
};
