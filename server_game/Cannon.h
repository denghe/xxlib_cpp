struct Cannon : PKG::CatchFish::Cannon {
	using BaseType = PKG::CatchFish::Cannon;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

	// 所在玩家( 由 Player 预填充 )
	Player* player = nullptr;

	// 指向具体配置
	PKG::CatchFish::Configs::Cannon* cfg = nullptr;

	// 根据玩家位置从 cfg 读取到坐标记录在此以便于使用
	xx::Pos pos;

	// 存放下次可开火的 frameNumber
	int fireCD = 0;

	// 剩余子弹数量. 炮台初创时从 cfg 读取填充. ( -1 表示无限, 其余情况每次发射 -1, 到 0 时无法发射 )
	int quantity = -1;

	// 发射子弹. 成功返回 true
#ifdef CC_TARGET_PLATFORM
	virtual int Fire(int const& frameNumber) noexcept;
#else
	// player 在遍历 recvFires 的时候定位到炮台就 call 这个函数来发射
	virtual int Fire(PKG::Client_CatchFish::Fire_s& o) noexcept;

	// player 在遍历 recvHits 的时候定位到炮台就 call 这个函数来做子弹碰撞检测
	virtual int Hit(PKG::Client_CatchFish::Hit_s& o) noexcept;

	// 生成退钱事件
	void MakeRefundEvent(int64_t const& coin, bool isPersional = false) noexcept;
#endif

	int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;

#ifdef CC_TARGET_PLATFORM
	RefHolder<cocos2d::Sprite> body;
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
#endif
};
using Cannon_s = std::shared_ptr<Cannon>;
using Cannon_w = std::weak_ptr<Cannon>;
