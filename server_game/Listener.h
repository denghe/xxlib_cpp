struct Listener : xx::UvKcpListener<Peer> {
	using BaseType = xx::UvKcpListener<Peer>;

	// 游戏实例( 此物也可以与 Listener 同级. 为方便先放这 )
	CatchFish catchFish;

	// 用于自增产生玩家 id
	int playerAutoId = 0;

	// 定时器. 模拟一个游戏循环
	xx::UvTimer_s looper;
	int64_t ticksPool = 0;
	int64_t lastTicks = xx::NowEpoch10m();
	// 比 60 略快以兼容部分硬件显示刷新不标准, 超过 60 帧的垃圾设备, 例如部分华为高端型号
	const int64_t ticksPerFrame = int64_t(10000000 / 60.3);

	Listener(xx::Uv & uv, std::string const& ip, int const& port);
	virtual void Accept(std::shared_ptr<xx::UvKcpBasePeer> peer_) noexcept override;
};
using Listener_s = std::shared_ptr<Listener>;
