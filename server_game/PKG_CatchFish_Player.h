// 所在场景
PKG::CatchFish::Scene* scene = nullptr;

// 根据 sit 填充的坐标 以方便使用
xx::Pos pos;

#ifndef CC_TARGET_PLATFORM
std::string token;

// 超时 Cleanup 帧. 创建玩家时令其 = scene.frameNumber + 60 * N. 收到合法指令时重置.
int timeoutFrameNumber = 0;

// 绑定的网络连接
std::shared_ptr<Peer> peer;

// 重置超时判断变量
void ResetTimeoutFrameNumber() noexcept;

// 踢下线( 将清除各种接收队列 )
template<typename ...Args>
int Kick(Args const& ... reason) noexcept;

// 收包容器( 在适当的生命周期读取并处理 )
std::deque<xx::Object_s> recvs;

// 私有事件通知容器
xx::List_s<PKG::CatchFish::Events::Event_s> events;
#else
// 标识这个玩家是本人
bool isSelf = false;

// 面板显示相关
int64_t lastCoin = -1;
RefHolder<cocos2d::Label> labelCoin;
void SetText_Coin(int64_t const& value) noexcept;
void DrawInit() noexcept;
void DrawUpdate_Coin() noexcept;

virtual int InitCascade(void* const& o) noexcept override;
#endif

virtual int Update(int const& frameNumber) noexcept override;
