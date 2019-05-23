// 所在场景
PKG::CatchFish::Scene* scene = nullptr;

// 所在玩家( 由 Player 预填充 )
PKG::CatchFish::Player* player = nullptr;

// 指向具体配置
PKG::CatchFish::Configs::Cannon* cfg = nullptr;

// 根据玩家位置从 cfg 读取到坐标记录在此以便于使用
xx::Pos pos;

// 存放下次可开火的 frameNumber
int fireCD = 0;

// 剩余子弹数量. 炮台初创时从 cfg 读取填充. ( -1 表示无限, 其余情况每次发射 -1, 到 0 时无法发射 )
int quantity = -1;

virtual int Update(int const& frameNumber) noexcept override;

// 发射子弹. 成功返回 true
#ifndef CC_TARGET_PLATFORM
// player 在遍历 recvs 的时候定位到炮台就 call 这个函数来发射
virtual int Fire(PKG::Client_CatchFish::Fire_s& o) noexcept;

// player 在遍历 recvs 的时候定位到炮台就 call 这个函数来做子弹碰撞检测
virtual int Hit(PKG::Client_CatchFish::Hit_s& o) noexcept;

// player 在遍历 recvs 的时候定位到炮台就 call 这个函数来修改炮台倍率
virtual int SetCoin(PKG::Client_CatchFish::Bet_s& o) noexcept;
#else
virtual int Fire(PKG::CatchFish::Events::Fire const& o) noexcept;
virtual int Fire(xx::Pos const& tarPos) noexcept;

int InitCascade(void* const& o) noexcept override;
virtual void DrawInit() noexcept;
virtual void DrawUpdate() noexcept;

void SetText_Coin() noexcept;
void ChangeCoin() noexcept;

RefHolder<cocos2d::Sprite> body;

RefHolder<cocos2d::Label> labelCoin;
RefHolder<cocos2d::Label> btnInc;
RefHolder<cocos2d::Label> btnDec;
RefHolder<cocos2d::EventListenerTouchOneByOne> listenerIncDec;
#endif
