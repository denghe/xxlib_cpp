#ifndef CC_TARGET_PLATFORM
struct Peer;
#endif
struct Player : PKG::CatchFish::Player {
	using BaseType = PKG::CatchFish::Player;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

#ifndef CC_TARGET_PLATFORM
	//// 令牌
	//std::string token;

	//// 开炮等行为花掉的金币数汇总 ( 统计 )
	//int64_t consumeCoin = 0;

	// 绑定的网络连接
	std::shared_ptr<Peer> peer;

	// 分类收包容器( 在适当的生命周期读取并处理 )
	std::deque<PKG::Client_CatchFish::Fire_s> recvFires;
	std::deque<PKG::Client_CatchFish::Hit_s> recvHits;
#else
	// 标识这个玩家是本人
	bool isSelf = false;
#endif

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;
#ifndef CC_TARGET_PLATFORM
	// 归还座位
	~Player();
#endif
};
using Player_s = std::shared_ptr<Player>;
using Player_w = std::weak_ptr<Player>;
