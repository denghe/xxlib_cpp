#ifndef CC_TARGET_PLATFORM
struct Peer;
#endif
struct Player : PKG::CatchFish::Player {
	using BaseType = PKG::CatchFish::Player;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

#ifndef CC_TARGET_PLATFORM
	// 超时 Cleanup 帧. 创建玩家时令其 = scene.frameNumber + 60 * N. 收到合法指令时重置.
	int timeoutFrameNumber = 0;

	// 绑定的网络连接
	std::shared_ptr<Peer> peer;

	// 重置超时判断变量
	void ResetTimeoutFrameNumber() noexcept;

	// 踢下线( 将清除各种接收队列 )
	template<typename ...Args>
	int Kick(Args const& ... reason) noexcept;

	// 分类收包容器( 在适当的生命周期读取并处理 )
	std::deque<PKG::Client_CatchFish::Fire_s> recvFires;
	std::deque<PKG::Client_CatchFish::Hit_s> recvHits;
#else
	// 标识这个玩家是本人
	bool isSelf = false;
#endif

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;


};
using Player_s = std::shared_ptr<Player>;
using Player_w = std::weak_ptr<Player>;
