#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

struct CatchFish;
#ifndef CC_TARGET_PLATFORM
struct Peer;
#else
struct Panel_Player;
#endif
#ifdef CC_TARGET_PLATFORM
#include "RefHolder.h"
struct Dialer;
std::shared_ptr<Dialer> dialer;
CatchFish* catchFish = nullptr;

inline cocos2d::Scene* cc_scene = nullptr;
inline RefHolder<cocos2d::Node> cc_fishNode;
inline RefHolder<cocos2d::Node> cc_uiNode;
inline RefHolder<cocos2d::EventListenerTouchAllAtOnce> cc_listener;
inline xx::List<cocos2d::Touch*> cc_touchs;
#endif

#ifndef CC_TARGET_PLATFORM
struct Peer;
#endif
#include "xx_pos.h"
#include "xx_random.h"
#include "chipmunk.h"
#include "PKG_class.h"

#include "xx_random.hpp"

/**************************************************************************************************/
// ::
/**************************************************************************************************/

// 是否显示物理碰撞检测线
#define DRAW_PHYSICS_POLYGON 0

static constexpr xx::Pos designSize = xx::Pos{ 1280, 720 };
static constexpr xx::Pos designSize_2 = xx::Pos{ designSize.x / 2, designSize.y / 2 };
static constexpr float designWidthRatio = designSize.x / (designSize.x + designSize.y);

#ifdef CC_TARGET_PLATFORM
inline static cocos2d::Size cc_visibleSize;
inline static cocos2d::Size cc_visibleSize_2;
// 基于实际设计尺寸的 9 点定位坐标
inline static xx::Pos cc_p1;
inline static xx::Pos cc_p2;
inline static xx::Pos cc_p3;
inline static xx::Pos cc_p4;
inline static xx::Pos cc_p5;
inline static xx::Pos cc_p6;
inline static xx::Pos cc_p7;
inline static xx::Pos cc_p8;
inline static xx::Pos cc_p9;
#endif
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
	xx::List<PKG::CatchFish::Player_s> players;

	// 游戏场景实例
	PKG::CatchFish::Scene_s scene;

	// server info( Init 时填充 )
	std::string serverIp;
	int serverPort = 0;

	// 初始化( 加载配置文件, .... )
#ifndef CC_TARGET_PLATFORM
	int Init(std::string const& cfgName) noexcept;
#else
	int Init(std::string const& ip, int const& port, std::string const& cfgName) noexcept;
#endif

	// logic. 每帧调用一次. 返回非0 表示退出
	int Update() noexcept;

	// 清掉某玩家( 复制传入以避免引用到容器内的地址造成无法正常删除 )
	void Cleanup(PKG::CatchFish::Player_s p) noexcept;

	// 被用于部分调用中检测是否正在发生析构
	bool disposed = false;

	// 析构全局对象
	~CatchFish();
};
using CatchFish_s = std::shared_ptr<CatchFish>;


#ifndef CC_TARGET_PLATFORM
#include "Peer.h"
#include "Listener.h"
#else
#include "Panel_Dialer.h"
#include "Panel_Player.h"
#include "Dialer.h"
#endif

/**************************************************************************************************/
// hpp includes
/**************************************************************************************************/

#include "CatchFish.hpp"
#include "PKG_CatchFish_Configs_Config.hpp"
#include "PKG_CatchFish_Configs_SpriteFrame.hpp"
#include "PKG_CatchFish_Configs_Physics.hpp"
#include "PKG_CatchFish_Stages_Emitter_CircleFishs.hpp"
#include "PKG_CatchFish_Stages_Emitter_RingFishs.hpp"
#include "PKG_CatchFish_Stages_Monitor_KeepBigFish.hpp"
#include "PKG_CatchFish_Stages_Monitor_KeepFatFish.hpp"
#include "PKG_CatchFish_Stages_Emitter_RandomFishs.hpp"
#include "PKG_CatchFish_Scene.hpp"
#include "PKG_CatchFish_Player.hpp"
#include "PKG_CatchFish_Cannon.hpp"
#include "PKG_CatchFish_Bullet.hpp"
#include "PKG_CatchFish_Fish.hpp"
#include "PKG_CatchFish_WayFish.hpp"
#include "PKG_CatchFish_RoundFish.hpp"
#include "PKG_CatchFish_BigFish.hpp"

#ifndef CC_TARGET_PLATFORM
#include "Peer.hpp"
#include "Listener.hpp"
#else
#include "Panel_Dialer.hpp"
#include "Panel_Player.hpp"
#include "Dialer.hpp"
#endif
