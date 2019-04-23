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

#include "RefHolder.h"

struct Dialer;
struct CatchFish;
std::shared_ptr<Dialer> dialer;
CatchFish* catchFish = nullptr;

inline cocos2d::Scene* cc_scene = nullptr;
inline xx::List<cocos2d::Touch*> cc_touchs;
inline RefHolder<cocos2d::EventListenerTouchAllAtOnce> cc_listener;

#endif

#include "Config.h"
#include "SpriteFrame.h"
#include "Physics.h"
#include "Scene.h"
#include "Fish.h"
#include "Player.h"
#include "Cannon.h"
#include "Bullet.h"

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
	Config_s cfg;

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
#endif

	// logic. 每帧调用一次. 返回非0 表示退出
	int Update() noexcept;

	// 清掉某玩家( 复制传入以避免引用到容器内的地址造成无法正常删除 )
	void Cleanup(Player_s p) noexcept;

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
#include "ClientPeer.h"
#include "Dialer.h"
#endif

/**************************************************************************************************/
// hpp includes
/**************************************************************************************************/

#include "CatchFish.hpp"
#include "Config.hpp"
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
#include "Panel_Dialer.hpp"
#include "Panel_Player.hpp"
#include "ClientPeer.hpp"
#include "Dialer.hpp"
#endif
