// 引用到配置. 由 Scene 创建者或调用 InitCascade 前填充. ( 需确保 cfg 比 Scene 死的晚 )
PKG::CatchFish::Configs::Config* cfg = nullptr;

// 指向所在服务实例. 由 Scene 创建者或调用 InitCascade 前填充.
::CatchFish* catchFish = nullptr;

#ifndef CC_TARGET_PLATFORM
// 自减id ( 从 -1 开始, 用于服务器下发鱼生成 )
int autoDecId = 0;

// server 下发鱼生成专用
xx::Random serverRnd;

// 每帧汇集所有事件下发. 如果没有任何事件, 则根据 frameNumber & 15 == 0 的条件下发空包
PKG::CatchFish_Client::FrameEvents_s frameEvents;

// 记录本帧刚进入的新玩家( 帧结束时清空 ) 用以判断是下发完整同步还是帧事件同步
xx::List<void*> frameEnters;

// 记录本次 update 过程产生的各式 hit 计算请求
PKG::CatchFish_Calc::HitCheck_s hitChecks;

// 计算结果更新( 回调 )
int UpdateCalc(xx::Object_s&& msg) noexcept;
int Handle(PKG::Calc_CatchFish::HitCheckResult_s&& msg) noexcept;

// hit 计算结果，同时也是执行标志位. 发起 Request 时该值为 1. 超时为 -1, 正常退出为 0
int calcResult = 0;

// 辅助计算容器
xx::List<int> fishIds;
#else
// 将 Scene 指针刷到所有子
virtual int InitCascade(void* const& o = nullptr) noexcept override;
#endif
// 随机生成一对具备合理显示效果的进出屏幕的关键点
std::pair<xx::Pos, xx::Pos> MakeRandomInOutPoint(float const& itemRadius) noexcept;

// 生成随机直线 WayFish
PKG::CatchFish::WayFish_s MakeRandomFish(int const& fishId, int64_t const& coin, float const& scaleFrom, float const& scaleTo) noexcept;

// 生成随机直线 BigFish
PKG::CatchFish::BigFish_s MakeRandomBigFish(int const& fishId) noexcept;

// 帧逻辑更新
int Update() noexcept;
