struct CatchFish;
struct Fish;
struct Scene : PKG::CatchFish::Scene, std::enable_shared_from_this<Scene> {
	using BaseType = PKG::CatchFish::Scene;
	using BaseType::BaseType;

	// 引用到配置. 由 Scene 创建者或调用 InitCascade 前填充. ( 需确保 cfg 比 Scene 死的晚 )
	Config* cfg = nullptr;

	// 指向所在服务实例. 由 Scene 创建者或调用 InitCascade 前填充.
	CatchFish* catchFish = nullptr;

#ifndef CC_TARGET_PLATFORM
	// 自减id ( 从 -1 开始, 用于服务器下发鱼生成 )
	int autoDecId = 0;

	// server 下发鱼生成专用
	xx::Random serverRnd;

	// 每帧汇集所有事件下发. 如果没有任何事件, 则根据 frameNumber & 15 == 0 的条件下发空包
	PKG::CatchFish_Client::FrameEvents_s frameEvents;

	// 记录本帧刚进入的新玩家( 帧结束时清空 ) 用以判断是下发完整同步还是帧事件同步
	xx::List<void*> frameEnters;
#endif

	// 随机生成一对具备合理显示效果的进出屏幕的关键点
	std::pair<xx::Pos, xx::Pos> MakeRandomInOutPoint(float const& itemRadius) noexcept;

	// 生成随机鱼
	void MakeRandomFish() noexcept;

	// 将 Scene 指针刷到所有子
	virtual int InitCascade(void* const& o = nullptr) noexcept override;

	// 帧逻辑更新
	virtual int Update(int const& frameNumber = 0) noexcept override;
};
using Scene_s = std::shared_ptr<Scene>;
using Scene_w = std::weak_ptr<Scene>;
