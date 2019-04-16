#pragma once
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include "xx_pos.h"
#include "xx_random.h"
#include "PKG_class.h"
#include "xx_random.hpp"
#include "chipmunk.h"

/**************************************************************************************************/
// ::
/**************************************************************************************************/

// 是否显示物理碰撞检测线
#define DRAW_PHYSICS_POLYGON 0

static constexpr int ScreenWidth = 1280;
static constexpr int ScreenHeight = 720;
static constexpr float ScreenWidthRatio = float(ScreenWidth) / float(ScreenWidth + ScreenHeight);
static constexpr xx::Pos ScreenCenter = xx::Pos{ ScreenWidth / 2, ScreenHeight / 2 };

#ifdef CC_TARGET_PLATFORM
struct Dialer;
struct CatchFish;
std::shared_ptr<Dialer> dialer;
CatchFish* catchFish = nullptr;

inline cocos2d::Scene* cc_scene = nullptr;
inline xx::List<cocos2d::Touch*> cc_touchs;
inline cocos2d::EventListenerTouchAllAtOnce* cc_listener = nullptr;
#endif



/**************************************************************************************************/
// SpriteFrame
/**************************************************************************************************/

struct SpriteFrame : PKG::CatchFish::Configs::SpriteFrame {
	using BaseType = PKG::CatchFish::Configs::SpriteFrame;
	using BaseType::BaseType;

#ifdef CC_TARGET_PLATFORM
	cocos2d::SpriteFrame* spriteFrame = nullptr;
#endif

	virtual int InitCascade(void* const& o) noexcept override;
	~SpriteFrame();
};
using SpriteFrame_s = std::shared_ptr<SpriteFrame>;
using SpriteFrame_w = std::weak_ptr<SpriteFrame>;


/**************************************************************************************************/
// Physics
/**************************************************************************************************/

struct Physics : PKG::CatchFish::Configs::Physics {
	using BaseType = PKG::CatchFish::Configs::Physics;
	using BaseType::BaseType;

	cpSpace* space = nullptr;

	virtual int InitCascade(void* const& o) noexcept override;
	~Physics();
};
using Physics_s = std::shared_ptr<Physics>;
using Physics_w = std::weak_ptr<Physics>;


/**************************************************************************************************/
// Scene
/**************************************************************************************************/

struct CatchFish;
struct Fish;
struct Scene : PKG::CatchFish::Scene, std::enable_shared_from_this<Scene> {
	using BaseType = PKG::CatchFish::Scene;
	using BaseType::BaseType;

	// 引用到配置. 由 Scene 创建者或调用 InitCascade 前填充. ( 需确保 cfg 比 Scene 死的晚 )
	PKG::CatchFish::Configs::Config* cfg = nullptr;

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

	// 生成随机直线路径
	PKG::CatchFish::Way_s MakeBeeline(float const& itemRadius) noexcept;

	// 生成随机鱼
	void MakeRandomFish() noexcept;

	// 将 Scene 指针刷到所有子
	virtual int InitCascade(void* const& o = nullptr) noexcept override;

	// 帧逻辑更新
	virtual int Update(int const& frameNumber = 0) noexcept override;



	// 调试专用。记录最近 120 fps 的鱼 id 列表, 以便在收到 server 下发的列表之后找出这帧的数据来对比
	std::unordered_map<int, xx::List<int>> fishIdss;


};
using Scene_s = std::shared_ptr<Scene>;
using Scene_w = std::weak_ptr<Scene>;


/**************************************************************************************************/
// Fish
/**************************************************************************************************/

struct Bullet;
struct Fish : PKG::CatchFish::Fish {
	using BaseType = PKG::CatchFish::Fish;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

	// 指向具体配置
	PKG::CatchFish::Configs::Fish* cfg = nullptr;

	virtual int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;
	~Fish();

	virtual int HitCheck(Bullet* const& bullet) noexcept;

#ifdef CC_TARGET_PLATFORM
	cocos2d::Sprite* body = nullptr;
	cocos2d::Sprite* shadow = nullptr;
#if DRAW_PHYSICS_POLYGON
	cocos2d::DrawNode* debugNode = nullptr;
#endif
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
	virtual void DrawDispose() noexcept;
#endif
};
using Fish_s = std::shared_ptr<Fish>;
using Fish_w = std::weak_ptr<Fish>;


/**************************************************************************************************/
// Player
/**************************************************************************************************/

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


/**************************************************************************************************/
// Cannon
/**************************************************************************************************/

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
#endif

	int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;
	~Cannon();

#ifdef CC_TARGET_PLATFORM
	cocos2d::Sprite* body = nullptr;
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
	virtual void DrawDispose() noexcept;
#endif
};
using Cannon_s = std::shared_ptr<Cannon>;
using Cannon_w = std::weak_ptr<Cannon>;


/**************************************************************************************************/
// Bullet
/**************************************************************************************************/

struct Bullet : PKG::CatchFish::Bullet {
	using BaseType = PKG::CatchFish::Bullet;
	using BaseType::BaseType;

	// 所在场景
	Scene* scene = nullptr;

	// 所在玩家( 由 Cannon 预填充 )
	Player* player = nullptr;

	// 所属炮台( 由 Cannon 预填充 )
	Cannon* cannon = nullptr;

	// 指向具体配置( 由 Cannon 预填充, Bullet 与 Cannon 共享配置 )
	PKG::CatchFish::Configs::Cannon* cfg = nullptr;

	int InitCascade(void* const& o) noexcept override;
	virtual int Update(int const& frameNumber) noexcept override;
	~Bullet();

	// 移动子弹。如果生命周期结束将返回非 0
	int Move() noexcept;

#ifdef CC_TARGET_PLATFORM
	cocos2d::Sprite* body = nullptr;
	virtual void DrawInit() noexcept;
	virtual void DrawUpdate() noexcept;
	virtual void DrawDispose() noexcept;
#endif
};
using Bullet_s = std::shared_ptr<Bullet>;
using Bullet_w = std::weak_ptr<Bullet>;

/**************************************************************************************************/
// todo: more
/**************************************************************************************************/

// todo: more 





/**************************************************************************************************/
// CatchFish
/**************************************************************************************************/

#ifdef CC_TARGET_PLATFORM
struct Dialer;
#endif
struct CatchFish {
	CatchFish();
	CatchFish(CatchFish&& o) = default;
	CatchFish(CatchFish const& o) = delete;
	CatchFish& operator=(CatchFish&& o) = default;
	CatchFish& operator=(CatchFish const& o) = delete;

	// 全局公用共享配置单例
	PKG::CatchFish::Configs::Config_s cfg;

	// 所有玩家的强存储
	xx::List<Player_s> players;

	// 游戏场景实例
	Scene_s scene;

	// server info( Init 时填充 )
	std::string serverIp;
	int serverPort = 0;

	// 初始化( 加载配置文件, .... )
#ifndef CC_TARGET_PLATFORM
	int Init(std::string const& cfgName) noexcept;
#else
	int Init(std::string const& ip, int const& port, std::string const& cfgName) noexcept;

	// 显示面板相关
	cocos2d::Label* labelPing = nullptr;
	void SetLabelPingText(std::string const& txt) noexcept;
#endif

	// logic. 每帧调用一次. 返回非0 表示退出
	int Update() noexcept;

	bool disposed = false;
	void Dispose(int const& flag = 1) noexcept;
	~CatchFish();
};
using CatchFish_s = std::shared_ptr<CatchFish>;





#ifndef CC_TARGET_PLATFORM

/**************************************************************************************************/
// Peer
/**************************************************************************************************/

struct Peer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
	using BaseType::BaseType;

	// 所在游戏实例( Listener Accept 时填充 )
	CatchFish* catchFish = nullptr;

	// Enter 成功后绑定到玩家
	Player_w player_w;

	// pong package cache for send
	inline static PKG::Generic::Pong_s pkgPong = xx::Make<PKG::Generic::Pong>();

	// 处理推送
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override;

	virtual int ReceiveRequest(int const& serial, xx::Object_s&& msg) noexcept override;

	virtual void Dispose(int const& flag = 1) noexcept override;
};


/**************************************************************************************************/
// Listener
/**************************************************************************************************/

struct Listener : xx::UvKcpListener<Peer> {
	using BaseType = xx::UvKcpListener<Peer>;

	// 游戏实例( 此物也可以与 Listener 同级. 为方便先放这 )
	CatchFish catchFish;

	// 定时器. 模拟一个游戏循环
	xx::UvTimer_s looper;
	int64_t ticksPool = 0;
	int64_t lastTicks = xx::NowEpoch10m();
	// 比 60 略快以兼容部分硬件显示刷新不标准, 超过 60 帧的垃圾设备, 例如部分华为高端型号
	const int64_t ticksPerFrame = int64_t(10000000 / 60.3);	

	Listener(xx::Uv& uv, std::string const& ip, int const& port);
	virtual void Accept(std::shared_ptr<xx::UvKcpBasePeer> peer_) noexcept override;
};
using Listener_s = std::shared_ptr<Listener>;

#else

/**************************************************************************************************/
// ClientPeer
/**************************************************************************************************/

struct Dialer;
struct ClientPeer : xx::UvKcpPeer {
	using BaseType = xx::UvKcpPeer;
	using BaseType::BaseType;

	// 处理推送( 向 dialer.recvs 压入数据 )
	virtual int ReceivePush(xx::Object_s&& msg) noexcept override;
};
using ClientPeer_s = std::shared_ptr<ClientPeer>();
using ClientPeer_w = std::weak_ptr<ClientPeer>();

/**************************************************************************************************/
// Dialer
/**************************************************************************************************/

struct Dialer : xx::UvKcpDialer<ClientPeer> {
	using BaseType = xx::UvKcpDialer<ClientPeer>;
	using BaseType::BaseType;

	// 收到的数据
	std::deque<xx::Object_s> recvs;

	// 脚本行号
	int lineNumber = 0;

	// 脚本用变量
	bool finished = false;
	PKG::Client_CatchFish::Enter_s pkgEnter;
	int r = 0;
	int64_t waitMS = 0;
	std::vector<std::string> ips;
	xx::UvResolver_s resolver;
	int timeoutFrameNumber = 0;

	// 处理首包( EnterSuccess || Error )
	int HandleFirstPackage() noexcept;

	// 处理一般数据包( 总路由 )
	int HandlePackagesOrUpdateScene() noexcept;

	// 分别处理事件包
	int Handle(PKG::CatchFish::Events::Enter_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Leave_s o) noexcept;
	int Handle(PKG::CatchFish::Events::NoMoney_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Refund_s o) noexcept;
	int Handle(PKG::CatchFish::Events::FishDead_s o) noexcept;
	int Handle(PKG::CatchFish::Events::PushWeapon_s o) noexcept;
	int Handle(PKG::CatchFish::Events::PushFish_s o) noexcept;
	int Handle(PKG::CatchFish::Events::OpenAutoLock_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Aim_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CloseAutoLock_s o) noexcept;
	int Handle(PKG::CatchFish::Events::OpenAutoFire_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CloseAutoFire_s o) noexcept;
	int Handle(PKG::CatchFish::Events::Fire_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CannonSwitch_s o) noexcept;
	int Handle(PKG::CatchFish::Events::CannonCoinChange_s o) noexcept;
	int Handle(PKG::CatchFish::Events::DebugInfo_s o, int const& frameNumber) noexcept;



	// 清空 recvs, player, catchFish->players, scene
	void Reset() noexcept;

	// 脚本实现
	int UpdateCore(int const& lineNumber) noexcept;

	// 每帧驱动脚本
	int Update() noexcept;

	// 指向当前玩家
	Player_s player;


	// ping package cache for send
	inline static PKG::Generic::Ping_s pkgPing = xx::Make<PKG::Generic::Ping>();

};
using Dialer_s = std::shared_ptr<Dialer>;

#endif



/**************************************************************************************************/
// hpp includes
/**************************************************************************************************/

#include "CatchFish.hpp"

#include "SpriteFrame.hpp"
#include "Physics.hpp"
#include "Scene.hpp"
#include "Player.hpp"
#include "Fish.hpp"
#include "Cannon.hpp"
#include "Bullet.hpp"

#ifndef CC_TARGET_PLATFORM
#include "Peer.hpp"
#include "Listener.hpp"
#else
#include "ClientPeer.hpp"
#include "Dialer.hpp"
#endif
