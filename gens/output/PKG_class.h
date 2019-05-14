#pragma once
namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "d3dcf5399903d8167009da64931ac7f4";
    };

namespace Generic {
    // 通用返回
    struct Success;
    using Success_s = std::shared_ptr<Success>;
    using Success_w = std::weak_ptr<Success>;

    // 通用错误返回
    struct Error;
    using Error_s = std::shared_ptr<Error>;
    using Error_w = std::weak_ptr<Error>;

    // 心跳保持兼延迟测试 -- 请求
    struct Ping;
    using Ping_s = std::shared_ptr<Ping>;
    using Ping_w = std::weak_ptr<Ping>;

    // 心跳保持兼延迟测试 -- 回应
    struct Pong;
    using Pong_s = std::shared_ptr<Pong>;
    using Pong_w = std::weak_ptr<Pong>;

}
namespace CatchFish_Client {
    // 申请进入游戏 成功
    struct EnterSuccess;
    using EnterSuccess_s = std::shared_ptr<EnterSuccess>;
    using EnterSuccess_w = std::weak_ptr<EnterSuccess>;

    // 帧事件同步包
    struct FrameEvents;
    using FrameEvents_s = std::shared_ptr<FrameEvents>;
    using FrameEvents_w = std::weak_ptr<FrameEvents>;

}
namespace Client_CatchFish {
    // 申请进入游戏. 成功返回 EnterSuccess. 失败直接被 T
    struct Enter;
    using Enter_s = std::shared_ptr<Enter>;
    using Enter_w = std::weak_ptr<Enter>;

    // 调整炮台倍率
    struct Bet;
    using Bet_s = std::shared_ptr<Bet>;
    using Bet_w = std::weak_ptr<Bet>;

    // 开火
    struct Fire;
    using Fire_s = std::shared_ptr<Fire>;
    using Fire_w = std::weak_ptr<Fire>;

    // 碰撞检测
    struct Hit;
    using Hit_s = std::shared_ptr<Hit>;
    using Hit_w = std::weak_ptr<Hit>;

}
namespace CatchFish {
    // 场景
    struct Scene;
    using Scene_s = std::shared_ptr<Scene>;
    using Scene_w = std::weak_ptr<Scene>;

    // 场景元素的共通基类
    struct Item;
    using Item_s = std::shared_ptr<Item>;
    using Item_w = std::weak_ptr<Item>;

    // 玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
    struct Player;
    using Player_s = std::shared_ptr<Player>;
    using Player_w = std::weak_ptr<Player>;

    // 炮台基类. 下列属性适合大多数炮
    struct Cannon;
    using Cannon_s = std::shared_ptr<Cannon>;
    using Cannon_w = std::weak_ptr<Cannon>;

    // 子弹 & 鱼 & 武器 的基类
    struct MoveItem;
    using MoveItem_s = std::shared_ptr<MoveItem>;
    using MoveItem_w = std::weak_ptr<MoveItem>;

    // 子弹基类
    struct Bullet;
    using Bullet_s = std::shared_ptr<Bullet>;
    using Bullet_w = std::weak_ptr<Bullet>;

    // 鱼基类( 支持每帧 pos += moveInc 简单移动 )
    struct Fish;
    using Fish_s = std::shared_ptr<Fish>;
    using Fish_w = std::weak_ptr<Fish>;

    // 武器基类 ( 有一些特殊鱼死后会变做 某种武器 / 炮台，死时有个滞空展示时间，被用于解决网络同步延迟。所有端应该在展示时间结束前收到该预约。展示完成后武器将飞向炮台变为附加炮台 )
    struct Weapon;
    using Weapon_s = std::shared_ptr<Weapon>;
    using Weapon_w = std::weak_ptr<Weapon>;

    // 预约出鱼
    struct FishBorn;
    using FishBorn_s = std::shared_ptr<FishBorn>;
    using FishBorn_w = std::weak_ptr<FishBorn>;

    // 路点
    struct WayPoint;
    using WayPoint_s = std::shared_ptr<WayPoint>;
    using WayPoint_w = std::weak_ptr<WayPoint>;

    // 路径. 预约下发安全, 将复制路径完整数据
    struct Way;
    using Way_s = std::shared_ptr<Way>;
    using Way_w = std::weak_ptr<Way>;

    // 基于路径移动的鱼基类
    struct WayFish;
    using WayFish_s = std::shared_ptr<WayFish>;
    using WayFish_w = std::weak_ptr<WayFish>;

    // 围绕目标鱼 圆周 旋转的小鱼( 实现自己的 Move 函数并附加几个计算参数, 被 BigFish Move 调用 )
    struct RoundFish;
    using RoundFish_s = std::shared_ptr<RoundFish>;
    using RoundFish_w = std::weak_ptr<RoundFish>;

    // 一只大鱼, 身边围了几只小鱼. 分摊伤害. 随机直线慢移. 自动再生. 切换关卡时快速逃离
    struct BigFish;
    using BigFish_s = std::shared_ptr<BigFish>;
    using BigFish_w = std::weak_ptr<BigFish>;

}
namespace CatchFish::Events {
    // 事件基类
    struct Event;
    using Event_s = std::shared_ptr<Event>;
    using Event_w = std::weak_ptr<Event>;

    // 通知: 玩家进入. 大部分字段从 Player 类复制. 添加了部分初始数值, 可还原出玩家类实例.
    struct Enter;
    using Enter_s = std::shared_ptr<Enter>;
    using Enter_w = std::weak_ptr<Enter>;

    // 通知: 玩家离开
    struct Leave;
    using Leave_s = std::shared_ptr<Leave>;
    using Leave_w = std::weak_ptr<Leave>;

    // 通知: 玩家破产
    struct NoMoney;
    using NoMoney_s = std::shared_ptr<NoMoney>;
    using NoMoney_w = std::weak_ptr<NoMoney>;

    // 通知: 退钱( 常见于子弹打空 )
    struct Refund;
    using Refund_s = std::shared_ptr<Refund>;
    using Refund_w = std::weak_ptr<Refund>;

    // 通知: 鱼被打死
    struct FishDead;
    using FishDead_s = std::shared_ptr<FishDead>;
    using FishDead_w = std::weak_ptr<FishDead>;

    // 通知: 下发已生效 Weapon, 需要判断 flyFrameNumber, 放入 player.weapon 队列
    struct PushWeapon;
    using PushWeapon_s = std::shared_ptr<PushWeapon>;
    using PushWeapon_w = std::weak_ptr<PushWeapon>;

    // 预约: 出鱼( 需判定 beginFrameNumber ), 放入 scene.borns 队列. 用不到 playerId
    struct PushFish;
    using PushFish_s = std::shared_ptr<PushFish>;
    using PushFish_w = std::weak_ptr<PushFish>;

    // 转发: 开启开火锁定
    struct OpenAutoLock;
    using OpenAutoLock_s = std::shared_ptr<OpenAutoLock>;
    using OpenAutoLock_w = std::weak_ptr<OpenAutoLock>;

    // 转发: 玩家锁定后瞄准某鱼
    struct Aim;
    using Aim_s = std::shared_ptr<Aim>;
    using Aim_w = std::weak_ptr<Aim>;

    // 转发: 玩家开火解除锁定
    struct CloseAutoLock;
    using CloseAutoLock_s = std::shared_ptr<CloseAutoLock>;
    using CloseAutoLock_w = std::weak_ptr<CloseAutoLock>;

    // 转发: 玩家自动开火
    struct OpenAutoFire;
    using OpenAutoFire_s = std::shared_ptr<OpenAutoFire>;
    using OpenAutoFire_w = std::weak_ptr<OpenAutoFire>;

    // 转发: 玩家解除自动开火
    struct CloseAutoFire;
    using CloseAutoFire_s = std::shared_ptr<CloseAutoFire>;
    using CloseAutoFire_w = std::weak_ptr<CloseAutoFire>;

    // 转发: 发子弹( 单次 ). 非特殊子弹, 只可能是 cannons[0] 原始炮台发射
    struct Fire;
    using Fire_s = std::shared_ptr<Fire>;
    using Fire_w = std::weak_ptr<Fire>;

    // 转发: 切换炮台
    struct CannonSwitch;
    using CannonSwitch_s = std::shared_ptr<CannonSwitch>;
    using CannonSwitch_w = std::weak_ptr<CannonSwitch>;

    // 转发: 切换炮台倍率
    struct CannonCoinChange;
    using CannonCoinChange_s = std::shared_ptr<CannonCoinChange>;
    using CannonCoinChange_w = std::weak_ptr<CannonCoinChange>;

    // 调试信息( 开发阶段校验用 )
    struct DebugInfo;
    using DebugInfo_s = std::shared_ptr<DebugInfo>;
    using DebugInfo_w = std::weak_ptr<DebugInfo>;

}
namespace CatchFish::Stages {
    // 游戏关卡. 一切元素皆使用 Stage.ticks 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化
    struct Stage;
    using Stage_s = std::shared_ptr<Stage>;
    using Stage_w = std::weak_ptr<Stage>;

    // 关卡元素基类
    struct StageElement;
    using StageElement_s = std::shared_ptr<StageElement>;
    using StageElement_w = std::weak_ptr<StageElement>;

    // 发射器: 随机小鱼
    struct Emitter_RandomFishs;
    using Emitter_RandomFishs_s = std::shared_ptr<Emitter_RandomFishs>;
    using Emitter_RandomFishs_w = std::weak_ptr<Emitter_RandomFishs>;

    // 监视器: 自动再生肥鱼, 服务端预约下发
    struct Monitor_KeepFatFish;
    using Monitor_KeepFatFish_s = std::shared_ptr<Monitor_KeepFatFish>;
    using Monitor_KeepFatFish_w = std::weak_ptr<Monitor_KeepFatFish>;

    // 监视器: 自动再生大鱼, 服务端预约下发
    struct Monitor_KeepBigFish;
    using Monitor_KeepBigFish_s = std::shared_ptr<Monitor_KeepBigFish>;
    using Monitor_KeepBigFish_w = std::weak_ptr<Monitor_KeepBigFish>;

    // 发射器: 从屏幕中间圆环批量出小鱼
    struct Emitter_RingFishs;
    using Emitter_RingFishs_s = std::shared_ptr<Emitter_RingFishs>;
    using Emitter_RingFishs_w = std::weak_ptr<Emitter_RingFishs>;

    // 发射器: 从屏幕中间 0 度开始旋转式出小鱼
    struct Emitter_CircleFishs;
    using Emitter_CircleFishs_s = std::shared_ptr<Emitter_CircleFishs>;
    using Emitter_CircleFishs_w = std::weak_ptr<Emitter_CircleFishs>;

}
namespace CatchFish::Configs {
    // 游戏配置主体
    struct Config;
    using Config_s = std::shared_ptr<Config>;
    using Config_w = std::weak_ptr<Config>;

    // 配置基类
    struct Item;
    using Item_s = std::shared_ptr<Item>;
    using Item_w = std::weak_ptr<Item>;

    // 鱼配置基类 ( 派生类中不再包含 sprite frame 相关, 以便于资源加载管理扫描 )
    struct Fish;
    using Fish_s = std::shared_ptr<Fish>;
    using Fish_w = std::weak_ptr<Fish>;

    // 炮台 & 子弹配置基类
    struct Cannon;
    using Cannon_s = std::shared_ptr<Cannon>;
    using Cannon_w = std::weak_ptr<Cannon>;

    // 打爆部分特殊鱼出现的特殊武器配置基类
    struct Weapon;
    using Weapon_s = std::shared_ptr<Weapon>;
    using Weapon_w = std::weak_ptr<Weapon>;

    // 精灵帧
    struct SpriteFrame;
    using SpriteFrame_s = std::shared_ptr<SpriteFrame>;
    using SpriteFrame_w = std::weak_ptr<SpriteFrame>;

    // 物理建模 for 鱼与子弹碰撞检测
    struct Physics;
    using Physics_s = std::shared_ptr<Physics>;
    using Physics_w = std::weak_ptr<Physics>;

    // 带物理检测区和锁定线等附加数据的鱼移动帧动画
    struct FishSpriteFrame;
    using FishSpriteFrame_s = std::shared_ptr<FishSpriteFrame>;
    using FishSpriteFrame_w = std::weak_ptr<FishSpriteFrame>;

    // 小鱼环绕的大鱼的特殊配置
    struct BigFish;
    using BigFish_s = std::shared_ptr<BigFish>;
    using BigFish_w = std::weak_ptr<BigFish>;

}
namespace CatchFish {
    // 座位列表
    enum class Sits : int32_t {
        // 左下
        LeftBottom = 0,
        // 右下
        RightBottom = 1,
        // 右上
        RightTop = 2,
        // 左上
        LeftTop = 3,
    };
}
namespace CatchFish {
    // 路点
    struct WayPoint {
        // 坐标
        ::xx::Pos pos;
        // 角度
        float angle = 0;
        // 当前点到下一个点的物理/逻辑距离( 下一个点可能是相同坐标, 停在原地转身的效果 )
        float distance = 0;
    };
}
namespace CatchFish::Events {
    // 事件基类
    struct Event : xx::Object {
        // 相关玩家id
        int32_t playerId = 0;

        typedef Event ThisType;
        typedef xx::Object BaseType;
	    Event() = default;
		Event(Event const&) = delete;
		Event& operator=(Event const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish {
    // 场景元素的共通基类
    struct Item : xx::Object {
        // 标识码
        int32_t id = 0;
        // 位于容器时的下标 ( 用于快速交换删除. 部分类型不一定用到 )
        int32_t indexAtContainer = 0;

        typedef Item ThisType;
        typedef xx::Object BaseType;
	    Item() = default;
		Item(Item const&) = delete;
		Item& operator=(Item const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
#include <PKG_CatchFish_Item.h>
    };
    // 子弹 & 鱼 & 武器 的基类
    struct MoveItem : PKG::CatchFish::Item {
        // 中心点坐标
        ::xx::Pos pos;
        // 当前角度
        float angle = 0;
        // 每帧的直线移动坐标增量( 不一定用得上 )
        ::xx::Pos moveInc;

        typedef MoveItem ThisType;
        typedef PKG::CatchFish::Item BaseType;
	    MoveItem() = default;
		MoveItem(MoveItem const&) = delete;
		MoveItem& operator=(MoveItem const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Stages {
    // 关卡元素基类
    struct StageElement : xx::Object {
        // 生效时间点
        int32_t cfg_beginTicks = 0;
        // 结束时间点
        int32_t cfg_endTicks = 0;

        typedef StageElement ThisType;
        typedef xx::Object BaseType;
	    StageElement() = default;
		StageElement(StageElement const&) = delete;
		StageElement& operator=(StageElement const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
#include <PKG_CatchFish_Stages_StageElement.h>
    };
}
namespace CatchFish::Configs {
    // 配置基类
    struct Item : xx::Object {
        // 内部编号. 通常等同于所在容器下标
        int32_t id = 0;
        // 放大系数( 影响各种判定, 坐标计算 )
        float scale = 0;
        // 初始z轴( 部分 boss 可能临时改变自己的 z )
        int32_t zOrder = 0;
        // 帧集合 ( 用于贴图动态加载 / 卸载管理. 派生类所有帧都应该在此放一份 )
        xx::List_s<PKG::CatchFish::Configs::SpriteFrame_s> frames;

        typedef Item ThisType;
        typedef xx::Object BaseType;
	    Item() = default;
		Item(Item const&) = delete;
		Item& operator=(Item const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish {
    // 鱼基类( 支持每帧 pos += moveInc 简单移动 )
    struct Fish : PKG::CatchFish::MoveItem {
        // 配置id
        int32_t cfgId = 0;
        // 币值 / 倍率
        int64_t coin = 0;
        // 移动速度系数 ( 默认为 1 )
        float speedScale = 0;
        // 运行时缩放比例( 通常为 1 )
        float scale = 0;
        // 当前帧下标( 循环累加 )
        int32_t spriteFrameIndex = 0;
        // 帧比值, 平时为 1, 如果为 0 则表示鱼不动( 比如实现冰冻效果 ), 帧图也不更新. 如果大于 1, 则需要在 1 帧内多次驱动该鱼( 比如实现快速离场的效果 )
        int32_t frameRatio = 0;

        typedef Fish ThisType;
        typedef PKG::CatchFish::MoveItem BaseType;
	    Fish() = default;
		Fish(Fish const&) = delete;
		Fish& operator=(Fish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Fish.h>
    };
}
namespace CatchFish::Stages {
    // 发射器: 随机小鱼
    struct Emitter_RandomFishs : PKG::CatchFish::Stages::StageElement {
        // 配置: 两条鱼生成帧间隔
        int32_t cfg_bornTicksInterval = 0;
        // 配置: 币值
        int64_t cfg_coin = 0;
        // 配置: 体积随机起始范围
        float cfg_scaleFrom = 0;
        // 配置: 体积随机结束范围
        float cfg_scaleTo = 0;
        // 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        int32_t bornAvaliableTicks = 0;

        typedef Emitter_RandomFishs ThisType;
        typedef PKG::CatchFish::Stages::StageElement BaseType;
	    Emitter_RandomFishs() = default;
		Emitter_RandomFishs(Emitter_RandomFishs const&) = delete;
		Emitter_RandomFishs& operator=(Emitter_RandomFishs const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Stages_Emitter_RandomFishs.h>
    };
}
namespace CatchFish::Configs {
    // 鱼配置基类 ( 派生类中不再包含 sprite frame 相关, 以便于资源加载管理扫描 )
    struct Fish : PKG::CatchFish::Configs::Item {
        // 金币 / 倍率随机范围 ( 最小值 )
        int64_t minCoin = 0;
        // 金币 / 倍率随机范围 ( 最大值 )
        int64_t maxCoin = 0;
        // 基于整个鱼的最大晃动范围的圆形碰撞检测半径( 2 判. <= 0 则直接进行 3 判: 物理检测 )
        float maxDetectRadius = 0;
        // 必然命中的最小检测半径( 1 判. <= 0 则直接进行 2 判. 如果 bulletRadius + minDetectRadius > 子弹中心到鱼中心的距离 就认为命中 )
        float minDetectRadius = 0;
        // 移动帧集合 ( 部分鱼可能具有多种移动状态, 硬编码确定下标范围 )
        xx::List_s<PKG::CatchFish::Configs::FishSpriteFrame_s> moveFrames;
        // 鱼死帧集合
        xx::List_s<PKG::CatchFish::Configs::SpriteFrame_s> dieFrames;
        // 点选优先级说明参数, 越大越优先
        int32_t touchRank = 0;
        // 影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.
        float shadowScale = 0;
        // 影子的偏移坐标
        ::xx::Pos shadowOffset;

        typedef Fish ThisType;
        typedef PKG::CatchFish::Configs::Item BaseType;
	    Fish() = default;
		Fish(Fish const&) = delete;
		Fish& operator=(Fish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Events {
    // 转发: 玩家自动开火
    struct OpenAutoFire : PKG::CatchFish::Events::Event {

        typedef OpenAutoFire ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    OpenAutoFire() = default;
		OpenAutoFire(OpenAutoFire const&) = delete;
		OpenAutoFire& operator=(OpenAutoFire const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 转发: 玩家解除自动开火
    struct CloseAutoFire : PKG::CatchFish::Events::Event {

        typedef CloseAutoFire ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    CloseAutoFire() = default;
		CloseAutoFire(CloseAutoFire const&) = delete;
		CloseAutoFire& operator=(CloseAutoFire const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 转发: 玩家开火解除锁定
    struct CloseAutoLock : PKG::CatchFish::Events::Event {

        typedef CloseAutoLock ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    CloseAutoLock() = default;
		CloseAutoLock(CloseAutoLock const&) = delete;
		CloseAutoLock& operator=(CloseAutoLock const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 转发: 切换炮台
    struct CannonSwitch : PKG::CatchFish::Events::Event {
        // 炮台配置id
        int32_t cfgId = 0;

        typedef CannonSwitch ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    CannonSwitch() = default;
		CannonSwitch(CannonSwitch const&) = delete;
		CannonSwitch& operator=(CannonSwitch const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 转发: 切换炮台倍率
    struct CannonCoinChange : PKG::CatchFish::Events::Event {
        // 炮台id
        int32_t cannonId = 0;
        // 币值 / 倍率
        int64_t coin = 0;

        typedef CannonCoinChange ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    CannonCoinChange() = default;
		CannonCoinChange(CannonCoinChange const&) = delete;
		CannonCoinChange& operator=(CannonCoinChange const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 转发: 玩家锁定后瞄准某鱼
    struct Aim : PKG::CatchFish::Events::Event {
        // 被瞄准的鱼id
        int32_t fishId = 0;

        typedef Aim ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    Aim() = default;
		Aim(Aim const&) = delete;
		Aim& operator=(Aim const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 调试信息( 开发阶段校验用 )
    struct DebugInfo : PKG::CatchFish::Events::Event {
        // 鱼id集合
        xx::List_s<int32_t> fishIds;

        typedef DebugInfo ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    DebugInfo() = default;
		DebugInfo(DebugInfo const&) = delete;
		DebugInfo& operator=(DebugInfo const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Stages {
    // 游戏关卡. 一切元素皆使用 Stage.ticks 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化
    struct Stage : xx::Object {
        // 关卡 id( 通常等于下标值 )
        int32_t cfg_id = 0;
        // 结束时间点
        int32_t cfg_endTicks = 0;
        // 帧编号( 运行时每帧 +1 )
        int32_t ticks = 0;
        // 元素集合
        xx::List_s<PKG::CatchFish::Stages::StageElement_s> elements;
        // 监视器集合, 服务端专用
        xx::List_s<PKG::CatchFish::Stages::StageElement_s> monitors;

        typedef Stage ThisType;
        typedef xx::Object BaseType;
	    Stage() = default;
		Stage(Stage const&) = delete;
		Stage& operator=(Stage const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Events {
    // 转发: 发子弹( 单次 ). 非特殊子弹, 只可能是 cannons[0] 原始炮台发射
    struct Fire : PKG::CatchFish::Events::Event {
        // 起始帧编号 ( 来自客户端 )
        int32_t frameNumber = 0;
        // 炮台id
        int32_t cannonId = 0;
        // 子弹id
        int32_t bulletId = 0;
        // 子弹的发射角度
        float tarAngle = 0;

        typedef Fire ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    Fire() = default;
		Fire(Fire const&) = delete;
		Fire& operator=(Fire const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace Generic {
    // 通用返回
    struct Success : xx::Object {

        typedef Success ThisType;
        typedef xx::Object BaseType;
	    Success() = default;
		Success(Success const&) = delete;
		Success& operator=(Success const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Stages {
    // 监视器: 自动再生肥鱼, 服务端预约下发
    struct Monitor_KeepFatFish : PKG::CatchFish::Stages::Emitter_RandomFishs {
        // 配置: 鱼总数限制( 可优化为鱼创建 & 析构时去 + - 同步分类统计表. 这个表似乎也可以用个下标来定位元素, 下标存放在 fish 类里面, 可以是个数组 )
        int32_t cfg_numFishsLimit = 0;
        // 配置: 预约延迟
        int32_t cfg_bornDelayFrameNumber = 0;

        typedef Monitor_KeepFatFish ThisType;
        typedef PKG::CatchFish::Stages::Emitter_RandomFishs BaseType;
	    Monitor_KeepFatFish() = default;
		Monitor_KeepFatFish(Monitor_KeepFatFish const&) = delete;
		Monitor_KeepFatFish& operator=(Monitor_KeepFatFish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Stages_Monitor_KeepFatFish.h>
    };
    // 监视器: 自动再生大鱼, 服务端预约下发
    struct Monitor_KeepBigFish : PKG::CatchFish::Stages::StageElement {
        // 配置: 两条鱼生成帧间隔
        int32_t cfg_bornTicksInterval = 0;
        // 配置: 鱼总数限制( 可优化为鱼创建 & 析构时去 + - 同步分类统计表. 这个表似乎也可以用个下标来定位元素, 下标存放在 fish 类里面, 可以是个数组 )
        int32_t cfg_numFishsLimit = 0;
        // 配置: 预约延迟
        int32_t cfg_bornDelayFrameNumber = 0;
        // 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        int32_t bornAvaliableTicks = 0;

        typedef Monitor_KeepBigFish ThisType;
        typedef PKG::CatchFish::Stages::StageElement BaseType;
	    Monitor_KeepBigFish() = default;
		Monitor_KeepBigFish(Monitor_KeepBigFish const&) = delete;
		Monitor_KeepBigFish& operator=(Monitor_KeepBigFish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Stages_Monitor_KeepBigFish.h>
    };
    // 发射器: 从屏幕中间圆环批量出小鱼
    struct Emitter_RingFishs : PKG::CatchFish::Stages::StageElement {
        // 配置: 每波鱼只数
        int32_t cfg_numFishsPerBatch = 0;
        // 配置: 两波鱼生成帧间隔
        int32_t cfg_bornTicksInterval = 0;
        // 配置: 每只鱼币值
        int64_t cfg_coin = 0;
        // 配置: 每只鱼体积
        float cfg_scale = 0;
        // 配置: 每只鱼移动速度( 帧跨越像素距离 )
        float cfg_speed = 0;
        // 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        int32_t bornAvaliableTicks = 0;

        typedef Emitter_RingFishs ThisType;
        typedef PKG::CatchFish::Stages::StageElement BaseType;
	    Emitter_RingFishs() = default;
		Emitter_RingFishs(Emitter_RingFishs const&) = delete;
		Emitter_RingFishs& operator=(Emitter_RingFishs const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Stages_Emitter_RingFishs.h>
    };
    // 发射器: 从屏幕中间 0 度开始旋转式出小鱼
    struct Emitter_CircleFishs : PKG::CatchFish::Stages::StageElement {
        // 配置: 起始角度
        float cfg_angleBegin = 0;
        // 配置: 每只鱼偏转角度
        float cfg_angleIncrease = 0;
        // 配置: 两只鱼生成帧间隔
        int32_t cfg_bornTicksInterval = 0;
        // 配置: 每只鱼币值
        int64_t cfg_coin = 0;
        // 配置: 每只鱼体积
        float cfg_scale = 0;
        // 配置: 每只鱼移动速度( 帧跨越像素距离 )
        float cfg_speed = 0;
        // 记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )
        int32_t bornAvaliableTicks = 0;
        // 当前角度
        float angle = 0;

        typedef Emitter_CircleFishs ThisType;
        typedef PKG::CatchFish::Stages::StageElement BaseType;
	    Emitter_CircleFishs() = default;
		Emitter_CircleFishs(Emitter_CircleFishs const&) = delete;
		Emitter_CircleFishs& operator=(Emitter_CircleFishs const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Stages_Emitter_CircleFishs.h>
    };
}
namespace CatchFish::Configs {
    // 游戏配置主体
    struct Config : xx::Object {
        // 所有固定路径( 工具创建 )
        xx::List_s<PKG::CatchFish::Way_s> fixedWays;
        // 所有鱼的配置信息
        xx::List_s<PKG::CatchFish::Configs::Fish_s> fishs;
        // 所有炮台的配置信息
        xx::List_s<PKG::CatchFish::Configs::Cannon_s> cannons;
        // 所有武器的配置信息
        xx::List_s<PKG::CatchFish::Configs::Weapon_s> weapons;
        // 循环关卡数据( Scene 初次创建时，从 stages[0] clone. 可以在内存中 cache 序列化后的 binary )
        xx::List_s<PKG::CatchFish::Stages::Stage_s> stages;
        // 基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )
        xx::List_s<::xx::Pos> sitPositons;
        // 锁定点击范围 ( 增加容错, 不必点的太精确. 点击作用是 枚举该范围内出现的鱼, 找出并选取 touchRank 最大值那个 )
        float aimTouchRadius = 0;
        // 普通鱼最大半径 ( 用于生成鱼线确保鱼出现时刚好位于屏幕外 )
        float normalFishMaxRadius = 0;
        // 显示非当前玩家子弹时是否启用追帧快进令其同步( 会导致高延迟玩家发射的子弹看上去离炮口有点远 )
        bool enableBulletFastForward = false;

        typedef Config ThisType;
        typedef xx::Object BaseType;
	    Config() = default;
		Config(Config const&) = delete;
		Config& operator=(Config const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Configs_Config.h>
    };
    // 炮台 & 子弹配置基类
    struct Cannon : PKG::CatchFish::Configs::Item {
        // 初始角度
        float angle = 0;
        // 炮管长度
        float muzzleLen = 0;
        // 拥有的数量( -1: 无限 )
        int32_t quantity = 0;
        // 同屏颗数限制 ( 到达上限就不允许继续发射 )
        int32_t numLimit = 0;
        // 发射间隔帧数
        int32_t fireCD = 0;
        // 子弹检测半径
        int32_t radius = 0;
        // 子弹最大 / 显示半径
        int32_t maxRadius = 0;
        // 子弹每帧前进距离
        float distance = 0;

        typedef Cannon ThisType;
        typedef PKG::CatchFish::Configs::Item BaseType;
	    Cannon() = default;
		Cannon(Cannon const&) = delete;
		Cannon& operator=(Cannon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 打爆部分特殊鱼出现的特殊武器配置基类
    struct Weapon : PKG::CatchFish::Configs::Item {
        // 每帧移动距离
        float distance = 0;
        // 展示时长 ( 帧数 )
        float showNumFrames = 0;
        // 飞到玩家坐标之后变化出来的炮台 cfg 之基类
        PKG::CatchFish::Configs::Cannon_s cannon;

        typedef Weapon ThisType;
        typedef PKG::CatchFish::Configs::Item BaseType;
	    Weapon() = default;
		Weapon(Weapon const&) = delete;
		Weapon& operator=(Weapon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 精灵帧
    struct SpriteFrame : xx::Object {
        // plist资源名
        std::string_s plistName;
        // 帧名
        std::string_s frameName;

        typedef SpriteFrame ThisType;
        typedef xx::Object BaseType;
	    SpriteFrame() = default;
		SpriteFrame(SpriteFrame const&) = delete;
		SpriteFrame& operator=(SpriteFrame const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Configs_SpriteFrame.h>
    };
    // 物理建模 for 鱼与子弹碰撞检测
    struct Physics : xx::Object {
        // 基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模碰撞判定 )
        xx::List_s<xx::List_s<::xx::Pos>> polygons;

        typedef Physics ThisType;
        typedef xx::Object BaseType;
	    Physics() = default;
		Physics(Physics const&) = delete;
		Physics& operator=(Physics const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Configs_Physics.h>
    };
}
namespace CatchFish::Events {
    // 转发: 开启开火锁定
    struct OpenAutoLock : PKG::CatchFish::Events::Event {

        typedef OpenAutoLock ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    OpenAutoLock() = default;
		OpenAutoLock(OpenAutoLock const&) = delete;
		OpenAutoLock& operator=(OpenAutoLock const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 预约: 出鱼( 需判定 beginFrameNumber ), 放入 scene.borns 队列. 用不到 playerId
    struct PushFish : PKG::CatchFish::Events::Event {
        // 已于 server 端构造好的, 无牵挂的, 能干净下发的实例
        PKG::CatchFish::FishBorn_s born;

        typedef PushFish ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    PushFish() = default;
		PushFish(PushFish const&) = delete;
		PushFish& operator=(PushFish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 通知: 退钱( 常见于子弹打空 )
    struct Refund : PKG::CatchFish::Events::Event {
        // 币值
        int64_t coin = 0;

        typedef Refund ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    Refund() = default;
		Refund(Refund const&) = delete;
		Refund& operator=(Refund const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 通知: 鱼被打死
    struct FishDead : PKG::CatchFish::Events::Event {
        // 鱼id
        int32_t fishId = 0;
        // 子弹id
        int32_t bulletId = 0;
        // 金币所得( fish.coin * bullet.coin 或 server 计算牵连鱼之后的综合结果 )
        int64_t coin = 0;
        // 牵连死的鱼
        xx::List_s<PKG::CatchFish::Events::FishDead_s> fishDeads;

        typedef FishDead ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    FishDead() = default;
		FishDead(FishDead const&) = delete;
		FishDead& operator=(FishDead const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace Generic {
    // 通用错误返回
    struct Error : xx::Object {
        int64_t number = 0;
        std::string_s message;

        typedef Error ThisType;
        typedef xx::Object BaseType;
	    Error() = default;
		Error(Error const&) = delete;
		Error& operator=(Error const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 心跳保持兼延迟测试 -- 请求
    struct Ping : xx::Object {
        int64_t ticks = 0;

        typedef Ping ThisType;
        typedef xx::Object BaseType;
	    Ping() = default;
		Ping(Ping const&) = delete;
		Ping& operator=(Ping const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 心跳保持兼延迟测试 -- 回应
    struct Pong : xx::Object {
        int64_t ticks = 0;

        typedef Pong ThisType;
        typedef xx::Object BaseType;
	    Pong() = default;
		Pong(Pong const&) = delete;
		Pong& operator=(Pong const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish_Client {
    // 申请进入游戏 成功
    struct EnterSuccess : xx::Object {
        // 完整的游戏场景
        PKG::CatchFish::Scene_s scene;
        // 玩家强引用容器
        xx::List_s<PKG::CatchFish::Player_s> players;
        // 指向当前玩家
        std::weak_ptr<PKG::CatchFish::Player> self;
        // 当前 token( 为简化设计先放这. 正常情况下是前置服务告知 )
        std::string_s token;

        typedef EnterSuccess ThisType;
        typedef xx::Object BaseType;
	    EnterSuccess() = default;
		EnterSuccess(EnterSuccess const&) = delete;
		EnterSuccess& operator=(EnterSuccess const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 帧事件同步包
    struct FrameEvents : xx::Object {
        // 帧编号
        int32_t frameNumber = 0;
        // 帧事件集合
        xx::List_s<PKG::CatchFish::Events::Event_s> events;
        // 私有帧事件集合( 发送时会临时等于 player.events )
        xx::List_s<PKG::CatchFish::Events::Event_s> persionalEvents;

        typedef FrameEvents ThisType;
        typedef xx::Object BaseType;
	    FrameEvents() = default;
		FrameEvents(FrameEvents const&) = delete;
		FrameEvents& operator=(FrameEvents const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace Client_CatchFish {
    // 申请进入游戏. 成功返回 EnterSuccess. 失败直接被 T
    struct Enter : xx::Object {
        // 传递先前保存的 token 以便断线重连. 没有传空
        std::string_s token;

        typedef Enter ThisType;
        typedef xx::Object BaseType;
	    Enter() = default;
		Enter(Enter const&) = delete;
		Enter& operator=(Enter const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 调整炮台倍率
    struct Bet : xx::Object {
        int32_t cannonId = 0;
        int64_t coin = 0;

        typedef Bet ThisType;
        typedef xx::Object BaseType;
	    Bet() = default;
		Bet(Bet const&) = delete;
		Bet& operator=(Bet const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 开火
    struct Fire : xx::Object {
        int32_t frameNumber = 0;
        int32_t cannonId = 0;
        int32_t bulletId = 0;
        ::xx::Pos pos;

        typedef Fire ThisType;
        typedef xx::Object BaseType;
	    Fire() = default;
		Fire(Fire const&) = delete;
		Fire& operator=(Fire const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 碰撞检测
    struct Hit : xx::Object {
        int32_t cannonId = 0;
        int32_t bulletId = 0;
        int32_t fishId = 0;

        typedef Hit ThisType;
        typedef xx::Object BaseType;
	    Hit() = default;
		Hit(Hit const&) = delete;
		Hit& operator=(Hit const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish {
    // 场景
    struct Scene : xx::Object {
        // 游戏id
        int32_t gameId = 0;
        // 级别id
        int32_t levelId = 0;
        // 房间id
        int32_t roomId = 0;
        // 准入金
        double minMoney = 0;
        // 最低炮注( coin )( 针对普通炮台 )
        int64_t minBet = 0;
        // 最高炮注( coin )( 针对普通炮台 )
        int64_t maxBet = 0;
        // 加减炮注跨度( coin )( 针对普通炮台 )
        int64_t stepBet = 0;
        // 进出游戏时 money 自动兑换成 coin 要 乘除 的系数
        int32_t exchangeCoinRatio = 0;
        // 帧编号, 每帧 + 1. 用于同步
        int32_t frameNumber = 0;
        // 本地鱼生成专用随机数发生器
        ::xx::Random_s rnd;
        // 自增id ( 从 1 开始, 用于填充 本地鱼 id )
        int32_t autoIncId = 0;
        // 所有活鱼 ( 乱序 )
        xx::List_s<PKG::CatchFish::Fish_s> fishs;
        // 所有已创建非活鱼 ( 乱序 )
        xx::List_s<PKG::CatchFish::Item_s> items;
        // 所有鱼预约生成 ( 乱序 )
        xx::List_s<PKG::CatchFish::FishBorn_s> borns;
        // 当前关卡. endFrameNumber 到达时切换到下一关( clone from cfg.stages[(stage.id + 1) % cfg.stages.len] 并修正 各种 frameNumber )
        PKG::CatchFish::Stages::Stage_s stage;
        // 空闲座位下标( 初始时填入 Sits.LeftBottom RightBottom LeftTop RightTop )
        xx::List_s<PKG::CatchFish::Sits> freeSits;
        // 所有玩家( 弱引用. 具体容器在 Scene 之外 )
        xx::List_s<std::weak_ptr<PKG::CatchFish::Player>> players;

        typedef Scene ThisType;
        typedef xx::Object BaseType;
	    Scene() = default;
		Scene(Scene const&) = delete;
		Scene& operator=(Scene const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Scene.h>
    };
    // 玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
    struct Player : PKG::CatchFish::Item {
        // 昵称 用于客户端显示 ( 填充自 db )
        std::string_s nickname;
        // 头像id 用于客户端显示 ( 填充自 db )
        int32_t avatar_id = 0;
        // 破产标识 ( 每帧检测一次总资产是否为 0, 是就标记之. 总资产包括 coin, 已爆出的 weapons, 已获得的附加炮台, 飞行中的 bullets )
        bool noMoney = false;
        // 剩余金币值( 不代表玩家总资产 ). 当玩家进入到游戏时, 该值填充 money * exchangeCoinRatio. 玩家退出时, 做除法还原为 money.
        int64_t coin = 0;
        // 座位
        PKG::CatchFish::Sits sit = (PKG::CatchFish::Sits)0;
        // 自动锁定状态
        bool autoLock = false;
        // 自动开火状态
        bool autoFire = false;
        // 锁定瞄准的鱼
        std::weak_ptr<PKG::CatchFish::Fish> aimFish;
        // 自增id ( 从 1 开始, 用于填充 炮台, 子弹 id )
        int32_t autoIncId = 0;
        // 炮台堆栈 ( 例如: 常规炮 打到 钻头, 钻头飞向玩家变为 钻头炮, 覆盖在常规炮上 )
        xx::List_s<PKG::CatchFish::Cannon_s> cannons;
        // 武器集合 ( 被打死的特殊鱼转为武器对象, 飞向玩家, 变炮消失前都在这里 )
        xx::List_s<PKG::CatchFish::Weapon_s> weapons;

        typedef Player ThisType;
        typedef PKG::CatchFish::Item BaseType;
	    Player() = default;
		Player(Player const&) = delete;
		Player& operator=(Player const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Player.h>
    };
    // 炮台基类. 下列属性适合大多数炮
    struct Cannon : PKG::CatchFish::Item {
        // 配置id
        int32_t cfgId = 0;
        // 币值 / 倍率 ( 初始填充自 db. 玩家可调整数值. 范围限制为 Scene.minBet ~ maxBet )
        int64_t coin = 0;
        // 炮管角度 ( 每次发射时都填充一下 )
        float angle = 0;
        // 所有子弹
        xx::List_s<PKG::CatchFish::Bullet_s> bullets;

        typedef Cannon ThisType;
        typedef PKG::CatchFish::Item BaseType;
	    Cannon() = default;
		Cannon(Cannon const&) = delete;
		Cannon& operator=(Cannon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Cannon.h>
    };
    // 子弹基类
    struct Bullet : PKG::CatchFish::MoveItem {
        // 金币 / 倍率( 记录炮台开火时的 Bet 值 )
        int64_t coin = 0;

        typedef Bullet ThisType;
        typedef PKG::CatchFish::MoveItem BaseType;
	    Bullet() = default;
		Bullet(Bullet const&) = delete;
		Bullet& operator=(Bullet const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascadeCore(void* const& o = nullptr) noexcept;
#include <PKG_CatchFish_Bullet.h>
    };
    // 武器基类 ( 有一些特殊鱼死后会变做 某种武器 / 炮台，死时有个滞空展示时间，被用于解决网络同步延迟。所有端应该在展示时间结束前收到该预约。展示完成后武器将飞向炮台变为附加炮台 )
    struct Weapon : PKG::CatchFish::MoveItem {
        // 配置id
        int32_t cfgId = 0;
        // 开始飞行的帧编号
        int32_t flyFrameNumber = 0;

        typedef Weapon ThisType;
        typedef PKG::CatchFish::MoveItem BaseType;
	    Weapon() = default;
		Weapon(Weapon const&) = delete;
		Weapon& operator=(Weapon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 预约出鱼
    struct FishBorn : xx::Object {
        // 开始 / 生效帧编号
        int32_t beginFrameNumber = 0;
        // 当 currentFrameNumber == beginFrameNumber 时，将 fish 放入 Scene.fishs 并自杀
        PKG::CatchFish::Fish_s fish;

        typedef FishBorn ThisType;
        typedef xx::Object BaseType;
	    FishBorn() = default;
		FishBorn(FishBorn const&) = delete;
		FishBorn& operator=(FishBorn const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 路径. 预约下发安全, 将复制路径完整数据
    struct Way : xx::Object {
        // 路点集合
        xx::List_s<PKG::CatchFish::WayPoint> points;
        // 总距离长度( sum( points[all].distance ). 如果非循环线, 不包含最后一个点的距离值. )
        float distance = 0;
        // 是否循环( 即移动到最后一个点之后又到第 1 个点, 永远走不完
        bool loop = false;

        typedef Way ThisType;
        typedef xx::Object BaseType;
	    Way() = default;
		Way(Way const&) = delete;
		Way& operator=(Way const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 基于路径移动的鱼基类
    struct WayFish : PKG::CatchFish::Fish {
        // 移动路径. 动态生成, 不引用自 cfg. 同步时被复制. 如果该值为空, 则启用 wayTypeIndex / wayIndex
        PKG::CatchFish::Way_s way;
        // cfg.ways[wayTypeIndex]
        int32_t wayTypeIndex = 0;
        // cfg.ways[wayTypeIndex][wayIndex]
        int32_t wayIndex = 0;
        // 当前路点下标
        int32_t wayPointIndex = 0;
        // 当前路点上的已前进距离
        float wayPointDistance = 0;
        // 是否为在路径上倒着移动( 默认否 )
        bool reverse = false;

        typedef WayFish ThisType;
        typedef PKG::CatchFish::Fish BaseType;
	    WayFish() = default;
		WayFish(WayFish const&) = delete;
		WayFish& operator=(WayFish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
#include <PKG_CatchFish_WayFish.h>
    };
    // 围绕目标鱼 圆周 旋转的小鱼( 实现自己的 Move 函数并附加几个计算参数, 被 BigFish Move 调用 )
    struct RoundFish : PKG::CatchFish::Fish {
        // 目标大鱼到当前小鱼的角度
        float tarAngle = 0;

        typedef RoundFish ThisType;
        typedef PKG::CatchFish::Fish BaseType;
	    RoundFish() = default;
		RoundFish(RoundFish const&) = delete;
		RoundFish& operator=(RoundFish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
#include <PKG_CatchFish_RoundFish.h>
    };
    // 一只大鱼, 身边围了几只小鱼. 分摊伤害. 随机直线慢移. 自动再生. 切换关卡时快速逃离
    struct BigFish : PKG::CatchFish::Fish {
        // 围在身边的小鱼( Update, HitCheck 时级联处理 )
        xx::List_s<PKG::CatchFish::RoundFish_s> childs;

        typedef BigFish ThisType;
        typedef PKG::CatchFish::Fish BaseType;
	    BigFish() = default;
		BigFish(BigFish const&) = delete;
		BigFish& operator=(BigFish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
#include <PKG_CatchFish_BigFish.h>
    };
}
namespace CatchFish::Events {
    // 通知: 玩家进入. 大部分字段从 Player 类复制. 添加了部分初始数值, 可还原出玩家类实例.
    struct Enter : PKG::CatchFish::Events::Event {
        // 昵称
        std::string_s nickname;
        // 头像id
        int32_t avatar_id = 0;
        // 破产标识
        bool noMoney = false;
        // 剩余金币值
        int64_t coin = 0;
        // 座位
        PKG::CatchFish::Sits sit = (PKG::CatchFish::Sits)0;
        // 炮台配置id
        int32_t cannonCfgId = 0;
        // 炮台币值
        int64_t cannonCoin = 0;

        typedef Enter ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    Enter() = default;
		Enter(Enter const&) = delete;
		Enter& operator=(Enter const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 通知: 玩家离开
    struct Leave : PKG::CatchFish::Events::Event {

        typedef Leave ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    Leave() = default;
		Leave(Leave const&) = delete;
		Leave& operator=(Leave const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
    // 通知: 玩家破产
    struct NoMoney : PKG::CatchFish::Events::Event {

        typedef NoMoney ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    NoMoney() = default;
		NoMoney(NoMoney const&) = delete;
		NoMoney& operator=(NoMoney const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Configs {
    // 带物理检测区和锁定线等附加数据的鱼移动帧动画
    struct FishSpriteFrame : xx::Object {
        // 指向精灵帧
        PKG::CatchFish::Configs::SpriteFrame_s frame;
        // 指向物理建模
        PKG::CatchFish::Configs::Physics_s physics;
        // 首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )
        ::xx::Pos lockPoint;
        // 锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )
        xx::List_s<::xx::Pos> lockPoints;
        // 本帧动画切到下一帧动画后应该移动的距离( 受 Fish.speedScale 影响 )
        float moveDistance = 0;

        typedef FishSpriteFrame ThisType;
        typedef xx::Object BaseType;
	    FishSpriteFrame() = default;
		FishSpriteFrame(FishSpriteFrame const&) = delete;
		FishSpriteFrame& operator=(FishSpriteFrame const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Events {
    // 通知: 下发已生效 Weapon, 需要判断 flyFrameNumber, 放入 player.weapon 队列
    struct PushWeapon : PKG::CatchFish::Events::Event {
        // 已于 server 端构造好的, 无牵挂的, 能干净下发的实例
        PKG::CatchFish::Weapon_s weapon;

        typedef PushWeapon ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    PushWeapon() = default;
		PushWeapon(PushWeapon const&) = delete;
		PushWeapon& operator=(PushWeapon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
namespace CatchFish::Configs {
    // 小鱼环绕的大鱼的特殊配置
    struct BigFish : PKG::CatchFish::Configs::Fish {
        // 每帧移动距离
        float moveFrameDistance = 0;
        // 小鱼只数
        int32_t numChilds = 0;
        // 小鱼前进角速度
        float childsAngleInc = 0;

        typedef BigFish ThisType;
        typedef PKG::CatchFish::Configs::Fish BaseType;
	    BigFish() = default;
		BigFish(BigFish const&) = delete;
		BigFish& operator=(BigFish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;
    };
}
}
namespace xx {
	template<>
	struct BFuncs<PKG::CatchFish::WayPoint, void> {
		static inline void WriteTo(BBuffer& bb, PKG::CatchFish::WayPoint const& in) noexcept {
			bb.Write(in.pos, in.angle, in.distance);
		}
		static inline int ReadFrom(BBuffer& bb, PKG::CatchFish::WayPoint& out) noexcept {
			return bb.Read(out.pos, out.angle, out.distance);
		}
	};
	template<>
	struct SFuncs<PKG::CatchFish::WayPoint, void> {
		static inline void WriteTo(std::string& s, PKG::CatchFish::WayPoint const& in) noexcept {
			xx::Append(s, "{ \"structTypeName\":\"CatchFish.WayPoint\"", ", \"pos\":", in.pos, ", \"angle\":", in.angle, ", \"distance\":", in.distance, " }");
        }
    };
    template<> struct TypeId<PKG::Generic::Success> { static const uint16_t value = 3; };
    template<> struct TypeId<PKG::Generic::Error> { static const uint16_t value = 4; };
    template<> struct TypeId<PKG::Generic::Ping> { static const uint16_t value = 5; };
    template<> struct TypeId<PKG::Generic::Pong> { static const uint16_t value = 6; };
    template<> struct TypeId<PKG::CatchFish_Client::EnterSuccess> { static const uint16_t value = 7; };
    template<> struct TypeId<PKG::CatchFish::Scene> { static const uint16_t value = 8; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Player_s>> { static const uint16_t value = 9; };
    template<> struct TypeId<PKG::CatchFish::Player> { static const uint16_t value = 10; };
    template<> struct TypeId<PKG::CatchFish_Client::FrameEvents> { static const uint16_t value = 11; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Events::Event_s>> { static const uint16_t value = 12; };
    template<> struct TypeId<PKG::CatchFish::Events::Event> { static const uint16_t value = 13; };
    template<> struct TypeId<PKG::Client_CatchFish::Enter> { static const uint16_t value = 14; };
    template<> struct TypeId<PKG::Client_CatchFish::Bet> { static const uint16_t value = 87; };
    template<> struct TypeId<PKG::Client_CatchFish::Fire> { static const uint16_t value = 15; };
    template<> struct TypeId<PKG::Client_CatchFish::Hit> { static const uint16_t value = 16; };
    template<> struct TypeId<::xx::Random> { static const uint16_t value = 17; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Fish_s>> { static const uint16_t value = 18; };
    template<> struct TypeId<PKG::CatchFish::Fish> { static const uint16_t value = 19; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Item_s>> { static const uint16_t value = 20; };
    template<> struct TypeId<PKG::CatchFish::Item> { static const uint16_t value = 21; };
    template<> struct TypeId<xx::List<PKG::CatchFish::FishBorn_s>> { static const uint16_t value = 22; };
    template<> struct TypeId<PKG::CatchFish::FishBorn> { static const uint16_t value = 23; };
    template<> struct TypeId<PKG::CatchFish::Stages::Stage> { static const uint16_t value = 24; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Sits>> { static const uint16_t value = 25; };
    template<> struct TypeId<xx::List<std::weak_ptr<PKG::CatchFish::Player>>> { static const uint16_t value = 26; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Cannon_s>> { static const uint16_t value = 27; };
    template<> struct TypeId<PKG::CatchFish::Cannon> { static const uint16_t value = 28; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Weapon_s>> { static const uint16_t value = 29; };
    template<> struct TypeId<PKG::CatchFish::Weapon> { static const uint16_t value = 30; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Bullet_s>> { static const uint16_t value = 31; };
    template<> struct TypeId<PKG::CatchFish::Bullet> { static const uint16_t value = 32; };
    template<> struct TypeId<PKG::CatchFish::MoveItem> { static const uint16_t value = 33; };
    template<> struct TypeId<PKG::CatchFish::Way> { static const uint16_t value = 34; };
    template<> struct TypeId<xx::List<PKG::CatchFish::WayPoint>> { static const uint16_t value = 36; };
    template<> struct TypeId<PKG::CatchFish::WayFish> { static const uint16_t value = 80; };
    template<> struct TypeId<PKG::CatchFish::RoundFish> { static const uint16_t value = 82; };
    template<> struct TypeId<PKG::CatchFish::BigFish> { static const uint16_t value = 83; };
    template<> struct TypeId<xx::List<PKG::CatchFish::RoundFish_s>> { static const uint16_t value = 84; };
    template<> struct TypeId<PKG::CatchFish::Events::Enter> { static const uint16_t value = 37; };
    template<> struct TypeId<PKG::CatchFish::Events::Leave> { static const uint16_t value = 38; };
    template<> struct TypeId<PKG::CatchFish::Events::NoMoney> { static const uint16_t value = 39; };
    template<> struct TypeId<PKG::CatchFish::Events::Refund> { static const uint16_t value = 40; };
    template<> struct TypeId<PKG::CatchFish::Events::FishDead> { static const uint16_t value = 41; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Events::FishDead_s>> { static const uint16_t value = 42; };
    template<> struct TypeId<PKG::CatchFish::Events::PushWeapon> { static const uint16_t value = 43; };
    template<> struct TypeId<PKG::CatchFish::Events::PushFish> { static const uint16_t value = 44; };
    template<> struct TypeId<PKG::CatchFish::Events::OpenAutoLock> { static const uint16_t value = 45; };
    template<> struct TypeId<PKG::CatchFish::Events::Aim> { static const uint16_t value = 46; };
    template<> struct TypeId<PKG::CatchFish::Events::CloseAutoLock> { static const uint16_t value = 47; };
    template<> struct TypeId<PKG::CatchFish::Events::OpenAutoFire> { static const uint16_t value = 48; };
    template<> struct TypeId<PKG::CatchFish::Events::CloseAutoFire> { static const uint16_t value = 49; };
    template<> struct TypeId<PKG::CatchFish::Events::Fire> { static const uint16_t value = 50; };
    template<> struct TypeId<PKG::CatchFish::Events::CannonSwitch> { static const uint16_t value = 51; };
    template<> struct TypeId<PKG::CatchFish::Events::CannonCoinChange> { static const uint16_t value = 52; };
    template<> struct TypeId<PKG::CatchFish::Events::DebugInfo> { static const uint16_t value = 53; };
    template<> struct TypeId<xx::List<int32_t>> { static const uint16_t value = 54; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Stages::StageElement_s>> { static const uint16_t value = 74; };
    template<> struct TypeId<PKG::CatchFish::Stages::StageElement> { static const uint16_t value = 75; };
    template<> struct TypeId<PKG::CatchFish::Stages::Emitter_RandomFishs> { static const uint16_t value = 76; };
    template<> struct TypeId<PKG::CatchFish::Stages::Monitor_KeepFatFish> { static const uint16_t value = 77; };
    template<> struct TypeId<PKG::CatchFish::Stages::Monitor_KeepBigFish> { static const uint16_t value = 86; };
    template<> struct TypeId<PKG::CatchFish::Stages::Emitter_RingFishs> { static const uint16_t value = 78; };
    template<> struct TypeId<PKG::CatchFish::Stages::Emitter_CircleFishs> { static const uint16_t value = 81; };
    template<> struct TypeId<PKG::CatchFish::Configs::Config> { static const uint16_t value = 57; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Way_s>> { static const uint16_t value = 58; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::Fish_s>> { static const uint16_t value = 59; };
    template<> struct TypeId<PKG::CatchFish::Configs::Fish> { static const uint16_t value = 60; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::Cannon_s>> { static const uint16_t value = 61; };
    template<> struct TypeId<PKG::CatchFish::Configs::Cannon> { static const uint16_t value = 62; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::Weapon_s>> { static const uint16_t value = 63; };
    template<> struct TypeId<PKG::CatchFish::Configs::Weapon> { static const uint16_t value = 64; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Stages::Stage_s>> { static const uint16_t value = 65; };
    template<> struct TypeId<xx::List<::xx::Pos>> { static const uint16_t value = 66; };
    template<> struct TypeId<PKG::CatchFish::Configs::Item> { static const uint16_t value = 67; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::SpriteFrame_s>> { static const uint16_t value = 68; };
    template<> struct TypeId<PKG::CatchFish::Configs::SpriteFrame> { static const uint16_t value = 69; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::FishSpriteFrame_s>> { static const uint16_t value = 70; };
    template<> struct TypeId<PKG::CatchFish::Configs::FishSpriteFrame> { static const uint16_t value = 71; };
    template<> struct TypeId<PKG::CatchFish::Configs::Physics> { static const uint16_t value = 72; };
    template<> struct TypeId<xx::List<xx::List_s<::xx::Pos>>> { static const uint16_t value = 73; };
    template<> struct TypeId<PKG::CatchFish::Configs::BigFish> { static const uint16_t value = 85; };
}
namespace PKG {
namespace Generic {
    inline uint16_t Success::GetTypeId() const noexcept {
        return 3;
    }
    inline void Success::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Success::FromBBuffer(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline int Success::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Success::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Generic.Success\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Success::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t Error::GetTypeId() const noexcept {
        return 4;
    }
    inline void Error::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->number);
        bb.Write(this->message);
    }
    inline int Error::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->number)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->message)) return r;
        return 0;
    }
    inline int Error::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Error::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Generic.Error\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Error::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"number\":", this->number);
        if (this->message) xx::Append(s, ", \"message\":\"", this->message, "\"");
        else xx::Append(s, ", \"message\":nil");
    }
    inline uint16_t Ping::GetTypeId() const noexcept {
        return 5;
    }
    inline void Ping::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->ticks);
    }
    inline int Ping::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->ticks)) return r;
        return 0;
    }
    inline int Ping::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Ping::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Generic.Ping\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Ping::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"ticks\":", this->ticks);
    }
    inline uint16_t Pong::GetTypeId() const noexcept {
        return 6;
    }
    inline void Pong::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->ticks);
    }
    inline int Pong::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->ticks)) return r;
        return 0;
    }
    inline int Pong::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Pong::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Generic.Pong\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Pong::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"ticks\":", this->ticks);
    }
}
namespace CatchFish_Client {
    inline uint16_t EnterSuccess::GetTypeId() const noexcept {
        return 7;
    }
    inline void EnterSuccess::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->scene);
        bb.Write(this->players);
        bb.Write(this->self);
        bb.Write(this->token);
    }
    inline int EnterSuccess::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->scene)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        if (int r = bb.Read(this->self)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->token)) return r;
        return 0;
    }
    inline int EnterSuccess::InitCascade(void* const& o) noexcept {
        if (this->scene) {
            if (int r = this->scene->InitCascade(o)) return r;
        }
        if (this->players) {
            if (int r = this->players->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void EnterSuccess::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish_Client.EnterSuccess\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void EnterSuccess::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"scene\":", this->scene);
        xx::Append(s, ", \"players\":", this->players);
        xx::Append(s, ", \"self\":", this->self);
        if (this->token) xx::Append(s, ", \"token\":\"", this->token, "\"");
        else xx::Append(s, ", \"token\":nil");
    }
    inline uint16_t FrameEvents::GetTypeId() const noexcept {
        return 11;
    }
    inline void FrameEvents::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->frameNumber);
        bb.Write(this->events);
        bb.Write(this->persionalEvents);
    }
    inline int FrameEvents::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->frameNumber)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->events)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->persionalEvents)) return r;
        return 0;
    }
    inline int FrameEvents::InitCascade(void* const& o) noexcept {
        if (this->events) {
            if (int r = this->events->InitCascade(o)) return r;
        }
        if (this->persionalEvents) {
            if (int r = this->persionalEvents->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void FrameEvents::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish_Client.FrameEvents\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void FrameEvents::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"frameNumber\":", this->frameNumber);
        xx::Append(s, ", \"events\":", this->events);
        xx::Append(s, ", \"persionalEvents\":", this->persionalEvents);
    }
}
namespace Client_CatchFish {
    inline uint16_t Enter::GetTypeId() const noexcept {
        return 14;
    }
    inline void Enter::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->token);
    }
    inline int Enter::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->token)) return r;
        return 0;
    }
    inline int Enter::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Enter::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_CatchFish.Enter\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Enter::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->token) xx::Append(s, ", \"token\":\"", this->token, "\"");
        else xx::Append(s, ", \"token\":nil");
    }
    inline uint16_t Bet::GetTypeId() const noexcept {
        return 87;
    }
    inline void Bet::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->cannonId);
        bb.Write(this->coin);
    }
    inline int Bet::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->cannonId)) return r;
        if (int r = bb.Read(this->coin)) return r;
        return 0;
    }
    inline int Bet::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Bet::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_CatchFish.Bet\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Bet::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cannonId\":", this->cannonId);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t Fire::GetTypeId() const noexcept {
        return 15;
    }
    inline void Fire::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->frameNumber);
        bb.Write(this->cannonId);
        bb.Write(this->bulletId);
        bb.Write(this->pos);
    }
    inline int Fire::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->frameNumber)) return r;
        if (int r = bb.Read(this->cannonId)) return r;
        if (int r = bb.Read(this->bulletId)) return r;
        if (int r = bb.Read(this->pos)) return r;
        return 0;
    }
    inline int Fire::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Fire::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_CatchFish.Fire\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Fire::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"frameNumber\":", this->frameNumber);
        xx::Append(s, ", \"cannonId\":", this->cannonId);
        xx::Append(s, ", \"bulletId\":", this->bulletId);
        xx::Append(s, ", \"pos\":", this->pos);
    }
    inline uint16_t Hit::GetTypeId() const noexcept {
        return 16;
    }
    inline void Hit::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->cannonId);
        bb.Write(this->bulletId);
        bb.Write(this->fishId);
    }
    inline int Hit::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->cannonId)) return r;
        if (int r = bb.Read(this->bulletId)) return r;
        if (int r = bb.Read(this->fishId)) return r;
        return 0;
    }
    inline int Hit::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Hit::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_CatchFish.Hit\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Hit::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cannonId\":", this->cannonId);
        xx::Append(s, ", \"bulletId\":", this->bulletId);
        xx::Append(s, ", \"fishId\":", this->fishId);
    }
}
namespace CatchFish {
    inline uint16_t Scene::GetTypeId() const noexcept {
        return 8;
    }
    inline void Scene::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->gameId);
        bb.Write(this->levelId);
        bb.Write(this->roomId);
        bb.Write(this->minMoney);
        bb.Write(this->minBet);
        bb.Write(this->maxBet);
        bb.Write(this->stepBet);
        bb.Write(this->exchangeCoinRatio);
        bb.Write(this->frameNumber);
        bb.Write(this->rnd);
        bb.Write(this->autoIncId);
        bb.Write(this->fishs);
        bb.Write(this->items);
        bb.Write(this->borns);
        bb.Write(this->stage);
        bb.Write(this->freeSits);
        bb.Write(this->players);
    }
    inline int Scene::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->gameId)) return r;
        if (int r = bb.Read(this->levelId)) return r;
        if (int r = bb.Read(this->roomId)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        if (int r = bb.Read(this->minBet)) return r;
        if (int r = bb.Read(this->maxBet)) return r;
        if (int r = bb.Read(this->stepBet)) return r;
        if (int r = bb.Read(this->exchangeCoinRatio)) return r;
        if (int r = bb.Read(this->frameNumber)) return r;
        if (int r = bb.Read(this->rnd)) return r;
        if (int r = bb.Read(this->autoIncId)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishs)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->items)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->borns)) return r;
        if (int r = bb.Read(this->stage)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->freeSits)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        return 0;
    }
    inline int Scene::InitCascadeCore(void* const& o) noexcept {
        if (this->rnd) {
            if (int r = this->rnd->InitCascade(o)) return r;
        }
        if (this->fishs) {
            if (int r = this->fishs->InitCascade(o)) return r;
        }
        if (this->items) {
            if (int r = this->items->InitCascade(o)) return r;
        }
        if (this->borns) {
            if (int r = this->borns->InitCascade(o)) return r;
        }
        if (this->stage) {
            if (int r = this->stage->InitCascade(o)) return r;
        }
        if (this->freeSits) {
            if (int r = this->freeSits->InitCascade(o)) return r;
        }
        if (this->players) {
            if (int r = this->players->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Scene::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Scene\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Scene::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"gameId\":", this->gameId);
        xx::Append(s, ", \"levelId\":", this->levelId);
        xx::Append(s, ", \"roomId\":", this->roomId);
        xx::Append(s, ", \"minMoney\":", this->minMoney);
        xx::Append(s, ", \"minBet\":", this->minBet);
        xx::Append(s, ", \"maxBet\":", this->maxBet);
        xx::Append(s, ", \"stepBet\":", this->stepBet);
        xx::Append(s, ", \"exchangeCoinRatio\":", this->exchangeCoinRatio);
        xx::Append(s, ", \"frameNumber\":", this->frameNumber);
        xx::Append(s, ", \"rnd\":", this->rnd);
        xx::Append(s, ", \"autoIncId\":", this->autoIncId);
        xx::Append(s, ", \"fishs\":", this->fishs);
        xx::Append(s, ", \"items\":", this->items);
        xx::Append(s, ", \"borns\":", this->borns);
        xx::Append(s, ", \"stage\":", this->stage);
        xx::Append(s, ", \"freeSits\":", this->freeSits);
        xx::Append(s, ", \"players\":", this->players);
    }
    inline uint16_t Item::GetTypeId() const noexcept {
        return 21;
    }
    inline void Item::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->indexAtContainer);
    }
    inline int Item::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->indexAtContainer)) return r;
        return 0;
    }
    inline int Item::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Item::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Item\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Item::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"indexAtContainer\":", this->indexAtContainer);
    }
    inline uint16_t Player::GetTypeId() const noexcept {
        return 10;
    }
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->nickname);
        bb.Write(this->avatar_id);
        bb.Write(this->noMoney);
        bb.Write(this->coin);
        bb.Write(this->sit);
        bb.Write(this->autoLock);
        bb.Write(this->autoFire);
        bb.Write(this->aimFish);
        bb.Write(this->autoIncId);
        bb.Write(this->cannons);
        bb.Write(this->weapons);
    }
    inline int Player::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->nickname)) return r;
        if (int r = bb.Read(this->avatar_id)) return r;
        if (int r = bb.Read(this->noMoney)) return r;
        if (int r = bb.Read(this->coin)) return r;
        if (int r = bb.Read(this->sit)) return r;
        if (int r = bb.Read(this->autoLock)) return r;
        if (int r = bb.Read(this->autoFire)) return r;
        if (int r = bb.Read(this->aimFish)) return r;
        if (int r = bb.Read(this->autoIncId)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->cannons)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->weapons)) return r;
        return 0;
    }
    inline int Player::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->cannons) {
            if (int r = this->cannons->InitCascade(o)) return r;
        }
        if (this->weapons) {
            if (int r = this->weapons->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Player::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Player\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Player::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->nickname) xx::Append(s, ", \"nickname\":\"", this->nickname, "\"");
        else xx::Append(s, ", \"nickname\":nil");
        xx::Append(s, ", \"avatar_id\":", this->avatar_id);
        xx::Append(s, ", \"noMoney\":", this->noMoney);
        xx::Append(s, ", \"coin\":", this->coin);
        xx::Append(s, ", \"sit\":", this->sit);
        xx::Append(s, ", \"autoLock\":", this->autoLock);
        xx::Append(s, ", \"autoFire\":", this->autoFire);
        xx::Append(s, ", \"aimFish\":", this->aimFish);
        xx::Append(s, ", \"autoIncId\":", this->autoIncId);
        xx::Append(s, ", \"cannons\":", this->cannons);
        xx::Append(s, ", \"weapons\":", this->weapons);
    }
    inline uint16_t Cannon::GetTypeId() const noexcept {
        return 28;
    }
    inline void Cannon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfgId);
        bb.Write(this->coin);
        bb.Write(this->angle);
        bb.Write(this->bullets);
    }
    inline int Cannon::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfgId)) return r;
        if (int r = bb.Read(this->coin)) return r;
        if (int r = bb.Read(this->angle)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->bullets)) return r;
        return 0;
    }
    inline int Cannon::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->bullets) {
            if (int r = this->bullets->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Cannon::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Cannon\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Cannon::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfgId\":", this->cfgId);
        xx::Append(s, ", \"coin\":", this->coin);
        xx::Append(s, ", \"angle\":", this->angle);
        xx::Append(s, ", \"bullets\":", this->bullets);
    }
    inline uint16_t MoveItem::GetTypeId() const noexcept {
        return 33;
    }
    inline void MoveItem::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->pos);
        bb.Write(this->angle);
        bb.Write(this->moveInc);
    }
    inline int MoveItem::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->pos)) return r;
        if (int r = bb.Read(this->angle)) return r;
        if (int r = bb.Read(this->moveInc)) return r;
        return 0;
    }
    inline int MoveItem::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void MoveItem::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.MoveItem\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void MoveItem::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"pos\":", this->pos);
        xx::Append(s, ", \"angle\":", this->angle);
        xx::Append(s, ", \"moveInc\":", this->moveInc);
    }
    inline uint16_t Bullet::GetTypeId() const noexcept {
        return 32;
    }
    inline void Bullet::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->coin);
    }
    inline int Bullet::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->coin)) return r;
        return 0;
    }
    inline int Bullet::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Bullet::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Bullet\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Bullet::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t Fish::GetTypeId() const noexcept {
        return 19;
    }
    inline void Fish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfgId);
        bb.Write(this->coin);
        bb.Write(this->speedScale);
        bb.Write(this->scale);
        bb.Write(this->spriteFrameIndex);
        bb.Write(this->frameRatio);
    }
    inline int Fish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfgId)) return r;
        if (int r = bb.Read(this->coin)) return r;
        if (int r = bb.Read(this->speedScale)) return r;
        if (int r = bb.Read(this->scale)) return r;
        if (int r = bb.Read(this->spriteFrameIndex)) return r;
        if (int r = bb.Read(this->frameRatio)) return r;
        return 0;
    }
    inline int Fish::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Fish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Fish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Fish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfgId\":", this->cfgId);
        xx::Append(s, ", \"coin\":", this->coin);
        xx::Append(s, ", \"speedScale\":", this->speedScale);
        xx::Append(s, ", \"scale\":", this->scale);
        xx::Append(s, ", \"spriteFrameIndex\":", this->spriteFrameIndex);
        xx::Append(s, ", \"frameRatio\":", this->frameRatio);
    }
    inline uint16_t Weapon::GetTypeId() const noexcept {
        return 30;
    }
    inline void Weapon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfgId);
        bb.Write(this->flyFrameNumber);
    }
    inline int Weapon::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfgId)) return r;
        if (int r = bb.Read(this->flyFrameNumber)) return r;
        return 0;
    }
    inline int Weapon::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Weapon::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Weapon\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Weapon::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfgId\":", this->cfgId);
        xx::Append(s, ", \"flyFrameNumber\":", this->flyFrameNumber);
    }
    inline uint16_t FishBorn::GetTypeId() const noexcept {
        return 23;
    }
    inline void FishBorn::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->beginFrameNumber);
        bb.Write(this->fish);
    }
    inline int FishBorn::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->beginFrameNumber)) return r;
        if (int r = bb.Read(this->fish)) return r;
        return 0;
    }
    inline int FishBorn::InitCascade(void* const& o) noexcept {
        if (this->fish) {
            if (int r = this->fish->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void FishBorn::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.FishBorn\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void FishBorn::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"beginFrameNumber\":", this->beginFrameNumber);
        xx::Append(s, ", \"fish\":", this->fish);
    }
    inline uint16_t Way::GetTypeId() const noexcept {
        return 34;
    }
    inline void Way::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->points);
        bb.Write(this->distance);
        bb.Write(this->loop);
    }
    inline int Way::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->points)) return r;
        if (int r = bb.Read(this->distance)) return r;
        if (int r = bb.Read(this->loop)) return r;
        return 0;
    }
    inline int Way::InitCascade(void* const& o) noexcept {
        if (this->points) {
            if (int r = this->points->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Way::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Way\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Way::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"points\":", this->points);
        xx::Append(s, ", \"distance\":", this->distance);
        xx::Append(s, ", \"loop\":", this->loop);
    }
    inline uint16_t WayFish::GetTypeId() const noexcept {
        return 80;
    }
    inline void WayFish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->way);
        bb.Write(this->wayTypeIndex);
        bb.Write(this->wayIndex);
        bb.Write(this->wayPointIndex);
        bb.Write(this->wayPointDistance);
        bb.Write(this->reverse);
    }
    inline int WayFish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->way)) return r;
        if (int r = bb.Read(this->wayTypeIndex)) return r;
        if (int r = bb.Read(this->wayIndex)) return r;
        if (int r = bb.Read(this->wayPointIndex)) return r;
        if (int r = bb.Read(this->wayPointDistance)) return r;
        if (int r = bb.Read(this->reverse)) return r;
        return 0;
    }
    inline int WayFish::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->way) {
            if (int r = this->way->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void WayFish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.WayFish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void WayFish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"way\":", this->way);
        xx::Append(s, ", \"wayTypeIndex\":", this->wayTypeIndex);
        xx::Append(s, ", \"wayIndex\":", this->wayIndex);
        xx::Append(s, ", \"wayPointIndex\":", this->wayPointIndex);
        xx::Append(s, ", \"wayPointDistance\":", this->wayPointDistance);
        xx::Append(s, ", \"reverse\":", this->reverse);
    }
    inline uint16_t RoundFish::GetTypeId() const noexcept {
        return 82;
    }
    inline void RoundFish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->tarAngle);
    }
    inline int RoundFish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->tarAngle)) return r;
        return 0;
    }
    inline int RoundFish::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void RoundFish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.RoundFish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void RoundFish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"tarAngle\":", this->tarAngle);
    }
    inline uint16_t BigFish::GetTypeId() const noexcept {
        return 83;
    }
    inline void BigFish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->childs);
    }
    inline int BigFish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->childs)) return r;
        return 0;
    }
    inline int BigFish::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->childs) {
            if (int r = this->childs->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void BigFish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.BigFish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void BigFish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"childs\":", this->childs);
    }
}
namespace CatchFish::Events {
    inline uint16_t Event::GetTypeId() const noexcept {
        return 13;
    }
    inline void Event::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->playerId);
    }
    inline int Event::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->playerId)) return r;
        return 0;
    }
    inline int Event::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Event::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.Event\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Event::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"playerId\":", this->playerId);
    }
    inline uint16_t Enter::GetTypeId() const noexcept {
        return 37;
    }
    inline void Enter::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->nickname);
        bb.Write(this->avatar_id);
        bb.Write(this->noMoney);
        bb.Write(this->coin);
        bb.Write(this->sit);
        bb.Write(this->cannonCfgId);
        bb.Write(this->cannonCoin);
    }
    inline int Enter::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->nickname)) return r;
        if (int r = bb.Read(this->avatar_id)) return r;
        if (int r = bb.Read(this->noMoney)) return r;
        if (int r = bb.Read(this->coin)) return r;
        if (int r = bb.Read(this->sit)) return r;
        if (int r = bb.Read(this->cannonCfgId)) return r;
        if (int r = bb.Read(this->cannonCoin)) return r;
        return 0;
    }
    inline int Enter::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Enter::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.Enter\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Enter::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->nickname) xx::Append(s, ", \"nickname\":\"", this->nickname, "\"");
        else xx::Append(s, ", \"nickname\":nil");
        xx::Append(s, ", \"avatar_id\":", this->avatar_id);
        xx::Append(s, ", \"noMoney\":", this->noMoney);
        xx::Append(s, ", \"coin\":", this->coin);
        xx::Append(s, ", \"sit\":", this->sit);
        xx::Append(s, ", \"cannonCfgId\":", this->cannonCfgId);
        xx::Append(s, ", \"cannonCoin\":", this->cannonCoin);
    }
    inline uint16_t Leave::GetTypeId() const noexcept {
        return 38;
    }
    inline void Leave::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
    }
    inline int Leave::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return 0;
    }
    inline int Leave::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Leave::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.Leave\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Leave::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t NoMoney::GetTypeId() const noexcept {
        return 39;
    }
    inline void NoMoney::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
    }
    inline int NoMoney::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return 0;
    }
    inline int NoMoney::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void NoMoney::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.NoMoney\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void NoMoney::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t Refund::GetTypeId() const noexcept {
        return 40;
    }
    inline void Refund::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->coin);
    }
    inline int Refund::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->coin)) return r;
        return 0;
    }
    inline int Refund::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Refund::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.Refund\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Refund::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t FishDead::GetTypeId() const noexcept {
        return 41;
    }
    inline void FishDead::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->fishId);
        bb.Write(this->bulletId);
        bb.Write(this->coin);
        bb.Write(this->fishDeads);
    }
    inline int FishDead::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->fishId)) return r;
        if (int r = bb.Read(this->bulletId)) return r;
        if (int r = bb.Read(this->coin)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishDeads)) return r;
        return 0;
    }
    inline int FishDead::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->fishDeads) {
            if (int r = this->fishDeads->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void FishDead::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.FishDead\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void FishDead::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"fishId\":", this->fishId);
        xx::Append(s, ", \"bulletId\":", this->bulletId);
        xx::Append(s, ", \"coin\":", this->coin);
        xx::Append(s, ", \"fishDeads\":", this->fishDeads);
    }
    inline uint16_t PushWeapon::GetTypeId() const noexcept {
        return 43;
    }
    inline void PushWeapon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->weapon);
    }
    inline int PushWeapon::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->weapon)) return r;
        return 0;
    }
    inline int PushWeapon::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->weapon) {
            if (int r = this->weapon->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void PushWeapon::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.PushWeapon\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void PushWeapon::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"weapon\":", this->weapon);
    }
    inline uint16_t PushFish::GetTypeId() const noexcept {
        return 44;
    }
    inline void PushFish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->born);
    }
    inline int PushFish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->born)) return r;
        return 0;
    }
    inline int PushFish::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->born) {
            if (int r = this->born->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void PushFish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.PushFish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void PushFish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"born\":", this->born);
    }
    inline uint16_t OpenAutoLock::GetTypeId() const noexcept {
        return 45;
    }
    inline void OpenAutoLock::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
    }
    inline int OpenAutoLock::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return 0;
    }
    inline int OpenAutoLock::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void OpenAutoLock::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.OpenAutoLock\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void OpenAutoLock::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t Aim::GetTypeId() const noexcept {
        return 46;
    }
    inline void Aim::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->fishId);
    }
    inline int Aim::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->fishId)) return r;
        return 0;
    }
    inline int Aim::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Aim::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.Aim\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Aim::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"fishId\":", this->fishId);
    }
    inline uint16_t CloseAutoLock::GetTypeId() const noexcept {
        return 47;
    }
    inline void CloseAutoLock::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
    }
    inline int CloseAutoLock::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return 0;
    }
    inline int CloseAutoLock::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void CloseAutoLock::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.CloseAutoLock\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void CloseAutoLock::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t OpenAutoFire::GetTypeId() const noexcept {
        return 48;
    }
    inline void OpenAutoFire::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
    }
    inline int OpenAutoFire::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return 0;
    }
    inline int OpenAutoFire::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void OpenAutoFire::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.OpenAutoFire\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void OpenAutoFire::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t CloseAutoFire::GetTypeId() const noexcept {
        return 49;
    }
    inline void CloseAutoFire::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
    }
    inline int CloseAutoFire::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return 0;
    }
    inline int CloseAutoFire::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void CloseAutoFire::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.CloseAutoFire\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void CloseAutoFire::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t Fire::GetTypeId() const noexcept {
        return 50;
    }
    inline void Fire::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->frameNumber);
        bb.Write(this->cannonId);
        bb.Write(this->bulletId);
        bb.Write(this->tarAngle);
    }
    inline int Fire::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->frameNumber)) return r;
        if (int r = bb.Read(this->cannonId)) return r;
        if (int r = bb.Read(this->bulletId)) return r;
        if (int r = bb.Read(this->tarAngle)) return r;
        return 0;
    }
    inline int Fire::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Fire::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.Fire\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Fire::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"frameNumber\":", this->frameNumber);
        xx::Append(s, ", \"cannonId\":", this->cannonId);
        xx::Append(s, ", \"bulletId\":", this->bulletId);
        xx::Append(s, ", \"tarAngle\":", this->tarAngle);
    }
    inline uint16_t CannonSwitch::GetTypeId() const noexcept {
        return 51;
    }
    inline void CannonSwitch::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfgId);
    }
    inline int CannonSwitch::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfgId)) return r;
        return 0;
    }
    inline int CannonSwitch::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void CannonSwitch::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.CannonSwitch\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void CannonSwitch::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfgId\":", this->cfgId);
    }
    inline uint16_t CannonCoinChange::GetTypeId() const noexcept {
        return 52;
    }
    inline void CannonCoinChange::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cannonId);
        bb.Write(this->coin);
    }
    inline int CannonCoinChange::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cannonId)) return r;
        if (int r = bb.Read(this->coin)) return r;
        return 0;
    }
    inline int CannonCoinChange::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void CannonCoinChange::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.CannonCoinChange\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void CannonCoinChange::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cannonId\":", this->cannonId);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t DebugInfo::GetTypeId() const noexcept {
        return 53;
    }
    inline void DebugInfo::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->fishIds);
    }
    inline int DebugInfo::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishIds)) return r;
        return 0;
    }
    inline int DebugInfo::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->fishIds) {
            if (int r = this->fishIds->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void DebugInfo::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.DebugInfo\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void DebugInfo::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"fishIds\":", this->fishIds);
    }
}
namespace CatchFish::Stages {
    inline uint16_t Stage::GetTypeId() const noexcept {
        return 24;
    }
    inline void Stage::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->cfg_id);
        bb.Write(this->cfg_endTicks);
        bb.Write(this->ticks);
        bb.Write(this->elements);
        bb.Write(this->monitors);
    }
    inline int Stage::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->cfg_id)) return r;
        if (int r = bb.Read(this->cfg_endTicks)) return r;
        if (int r = bb.Read(this->ticks)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->elements)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->monitors)) return r;
        return 0;
    }
    inline int Stage::InitCascade(void* const& o) noexcept {
        if (this->elements) {
            if (int r = this->elements->InitCascade(o)) return r;
        }
        if (this->monitors) {
            if (int r = this->monitors->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Stage::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.Stage\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Stage::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfg_id\":", this->cfg_id);
        xx::Append(s, ", \"cfg_endTicks\":", this->cfg_endTicks);
        xx::Append(s, ", \"ticks\":", this->ticks);
        xx::Append(s, ", \"elements\":", this->elements);
        xx::Append(s, ", \"monitors\":", this->monitors);
    }
    inline uint16_t StageElement::GetTypeId() const noexcept {
        return 75;
    }
    inline void StageElement::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->cfg_beginTicks);
        bb.Write(this->cfg_endTicks);
    }
    inline int StageElement::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->cfg_beginTicks)) return r;
        if (int r = bb.Read(this->cfg_endTicks)) return r;
        return 0;
    }
    inline int StageElement::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void StageElement::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.StageElement\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void StageElement::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfg_beginTicks\":", this->cfg_beginTicks);
        xx::Append(s, ", \"cfg_endTicks\":", this->cfg_endTicks);
    }
    inline uint16_t Emitter_RandomFishs::GetTypeId() const noexcept {
        return 76;
    }
    inline void Emitter_RandomFishs::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfg_bornTicksInterval);
        bb.Write(this->cfg_coin);
        bb.Write(this->cfg_scaleFrom);
        bb.Write(this->cfg_scaleTo);
        bb.Write(this->bornAvaliableTicks);
    }
    inline int Emitter_RandomFishs::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfg_bornTicksInterval)) return r;
        if (int r = bb.Read(this->cfg_coin)) return r;
        if (int r = bb.Read(this->cfg_scaleFrom)) return r;
        if (int r = bb.Read(this->cfg_scaleTo)) return r;
        if (int r = bb.Read(this->bornAvaliableTicks)) return r;
        return 0;
    }
    inline int Emitter_RandomFishs::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Emitter_RandomFishs::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.Emitter_RandomFishs\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Emitter_RandomFishs::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfg_bornTicksInterval\":", this->cfg_bornTicksInterval);
        xx::Append(s, ", \"cfg_coin\":", this->cfg_coin);
        xx::Append(s, ", \"cfg_scaleFrom\":", this->cfg_scaleFrom);
        xx::Append(s, ", \"cfg_scaleTo\":", this->cfg_scaleTo);
        xx::Append(s, ", \"bornAvaliableTicks\":", this->bornAvaliableTicks);
    }
    inline uint16_t Monitor_KeepFatFish::GetTypeId() const noexcept {
        return 77;
    }
    inline void Monitor_KeepFatFish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfg_numFishsLimit);
        bb.Write(this->cfg_bornDelayFrameNumber);
    }
    inline int Monitor_KeepFatFish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfg_numFishsLimit)) return r;
        if (int r = bb.Read(this->cfg_bornDelayFrameNumber)) return r;
        return 0;
    }
    inline int Monitor_KeepFatFish::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Monitor_KeepFatFish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.Monitor_KeepFatFish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Monitor_KeepFatFish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfg_numFishsLimit\":", this->cfg_numFishsLimit);
        xx::Append(s, ", \"cfg_bornDelayFrameNumber\":", this->cfg_bornDelayFrameNumber);
    }
    inline uint16_t Monitor_KeepBigFish::GetTypeId() const noexcept {
        return 86;
    }
    inline void Monitor_KeepBigFish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfg_bornTicksInterval);
        bb.Write(this->cfg_numFishsLimit);
        bb.Write(this->cfg_bornDelayFrameNumber);
        bb.Write(this->bornAvaliableTicks);
    }
    inline int Monitor_KeepBigFish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfg_bornTicksInterval)) return r;
        if (int r = bb.Read(this->cfg_numFishsLimit)) return r;
        if (int r = bb.Read(this->cfg_bornDelayFrameNumber)) return r;
        if (int r = bb.Read(this->bornAvaliableTicks)) return r;
        return 0;
    }
    inline int Monitor_KeepBigFish::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Monitor_KeepBigFish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.Monitor_KeepBigFish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Monitor_KeepBigFish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfg_bornTicksInterval\":", this->cfg_bornTicksInterval);
        xx::Append(s, ", \"cfg_numFishsLimit\":", this->cfg_numFishsLimit);
        xx::Append(s, ", \"cfg_bornDelayFrameNumber\":", this->cfg_bornDelayFrameNumber);
        xx::Append(s, ", \"bornAvaliableTicks\":", this->bornAvaliableTicks);
    }
    inline uint16_t Emitter_RingFishs::GetTypeId() const noexcept {
        return 78;
    }
    inline void Emitter_RingFishs::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfg_numFishsPerBatch);
        bb.Write(this->cfg_bornTicksInterval);
        bb.Write(this->cfg_coin);
        bb.Write(this->cfg_scale);
        bb.Write(this->cfg_speed);
        bb.Write(this->bornAvaliableTicks);
    }
    inline int Emitter_RingFishs::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfg_numFishsPerBatch)) return r;
        if (int r = bb.Read(this->cfg_bornTicksInterval)) return r;
        if (int r = bb.Read(this->cfg_coin)) return r;
        if (int r = bb.Read(this->cfg_scale)) return r;
        if (int r = bb.Read(this->cfg_speed)) return r;
        if (int r = bb.Read(this->bornAvaliableTicks)) return r;
        return 0;
    }
    inline int Emitter_RingFishs::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Emitter_RingFishs::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.Emitter_RingFishs\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Emitter_RingFishs::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfg_numFishsPerBatch\":", this->cfg_numFishsPerBatch);
        xx::Append(s, ", \"cfg_bornTicksInterval\":", this->cfg_bornTicksInterval);
        xx::Append(s, ", \"cfg_coin\":", this->cfg_coin);
        xx::Append(s, ", \"cfg_scale\":", this->cfg_scale);
        xx::Append(s, ", \"cfg_speed\":", this->cfg_speed);
        xx::Append(s, ", \"bornAvaliableTicks\":", this->bornAvaliableTicks);
    }
    inline uint16_t Emitter_CircleFishs::GetTypeId() const noexcept {
        return 81;
    }
    inline void Emitter_CircleFishs::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfg_angleBegin);
        bb.Write(this->cfg_angleIncrease);
        bb.Write(this->cfg_bornTicksInterval);
        bb.Write(this->cfg_coin);
        bb.Write(this->cfg_scale);
        bb.Write(this->cfg_speed);
        bb.Write(this->bornAvaliableTicks);
        bb.Write(this->angle);
    }
    inline int Emitter_CircleFishs::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfg_angleBegin)) return r;
        if (int r = bb.Read(this->cfg_angleIncrease)) return r;
        if (int r = bb.Read(this->cfg_bornTicksInterval)) return r;
        if (int r = bb.Read(this->cfg_coin)) return r;
        if (int r = bb.Read(this->cfg_scale)) return r;
        if (int r = bb.Read(this->cfg_speed)) return r;
        if (int r = bb.Read(this->bornAvaliableTicks)) return r;
        if (int r = bb.Read(this->angle)) return r;
        return 0;
    }
    inline int Emitter_CircleFishs::InitCascadeCore(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Emitter_CircleFishs::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.Emitter_CircleFishs\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Emitter_CircleFishs::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"cfg_angleBegin\":", this->cfg_angleBegin);
        xx::Append(s, ", \"cfg_angleIncrease\":", this->cfg_angleIncrease);
        xx::Append(s, ", \"cfg_bornTicksInterval\":", this->cfg_bornTicksInterval);
        xx::Append(s, ", \"cfg_coin\":", this->cfg_coin);
        xx::Append(s, ", \"cfg_scale\":", this->cfg_scale);
        xx::Append(s, ", \"cfg_speed\":", this->cfg_speed);
        xx::Append(s, ", \"bornAvaliableTicks\":", this->bornAvaliableTicks);
        xx::Append(s, ", \"angle\":", this->angle);
    }
}
namespace CatchFish::Configs {
    inline uint16_t Config::GetTypeId() const noexcept {
        return 57;
    }
    inline void Config::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->fixedWays);
        bb.Write(this->fishs);
        bb.Write(this->cannons);
        bb.Write(this->weapons);
        bb.Write(this->stages);
        bb.Write(this->sitPositons);
        bb.Write(this->aimTouchRadius);
        bb.Write(this->normalFishMaxRadius);
        bb.Write(this->enableBulletFastForward);
    }
    inline int Config::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fixedWays)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishs)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->cannons)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->weapons)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->stages)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->sitPositons)) return r;
        if (int r = bb.Read(this->aimTouchRadius)) return r;
        if (int r = bb.Read(this->normalFishMaxRadius)) return r;
        if (int r = bb.Read(this->enableBulletFastForward)) return r;
        return 0;
    }
    inline int Config::InitCascadeCore(void* const& o) noexcept {
        if (this->fixedWays) {
            if (int r = this->fixedWays->InitCascade(o)) return r;
        }
        if (this->fishs) {
            if (int r = this->fishs->InitCascade(o)) return r;
        }
        if (this->cannons) {
            if (int r = this->cannons->InitCascade(o)) return r;
        }
        if (this->weapons) {
            if (int r = this->weapons->InitCascade(o)) return r;
        }
        if (this->stages) {
            if (int r = this->stages->InitCascade(o)) return r;
        }
        if (this->sitPositons) {
            if (int r = this->sitPositons->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Config::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.Config\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Config::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"fixedWays\":", this->fixedWays);
        xx::Append(s, ", \"fishs\":", this->fishs);
        xx::Append(s, ", \"cannons\":", this->cannons);
        xx::Append(s, ", \"weapons\":", this->weapons);
        xx::Append(s, ", \"stages\":", this->stages);
        xx::Append(s, ", \"sitPositons\":", this->sitPositons);
        xx::Append(s, ", \"aimTouchRadius\":", this->aimTouchRadius);
        xx::Append(s, ", \"normalFishMaxRadius\":", this->normalFishMaxRadius);
        xx::Append(s, ", \"enableBulletFastForward\":", this->enableBulletFastForward);
    }
    inline uint16_t Item::GetTypeId() const noexcept {
        return 67;
    }
    inline void Item::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->scale);
        bb.Write(this->zOrder);
        bb.Write(this->frames);
    }
    inline int Item::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->scale)) return r;
        if (int r = bb.Read(this->zOrder)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->frames)) return r;
        return 0;
    }
    inline int Item::InitCascade(void* const& o) noexcept {
        if (this->frames) {
            if (int r = this->frames->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Item::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.Item\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Item::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"scale\":", this->scale);
        xx::Append(s, ", \"zOrder\":", this->zOrder);
        xx::Append(s, ", \"frames\":", this->frames);
    }
    inline uint16_t Fish::GetTypeId() const noexcept {
        return 60;
    }
    inline void Fish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->minCoin);
        bb.Write(this->maxCoin);
        bb.Write(this->maxDetectRadius);
        bb.Write(this->minDetectRadius);
        bb.Write(this->moveFrames);
        bb.Write(this->dieFrames);
        bb.Write(this->touchRank);
        bb.Write(this->shadowScale);
        bb.Write(this->shadowOffset);
    }
    inline int Fish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->minCoin)) return r;
        if (int r = bb.Read(this->maxCoin)) return r;
        if (int r = bb.Read(this->maxDetectRadius)) return r;
        if (int r = bb.Read(this->minDetectRadius)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->moveFrames)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->dieFrames)) return r;
        if (int r = bb.Read(this->touchRank)) return r;
        if (int r = bb.Read(this->shadowScale)) return r;
        if (int r = bb.Read(this->shadowOffset)) return r;
        return 0;
    }
    inline int Fish::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->moveFrames) {
            if (int r = this->moveFrames->InitCascade(o)) return r;
        }
        if (this->dieFrames) {
            if (int r = this->dieFrames->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Fish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.Fish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Fish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"minCoin\":", this->minCoin);
        xx::Append(s, ", \"maxCoin\":", this->maxCoin);
        xx::Append(s, ", \"maxDetectRadius\":", this->maxDetectRadius);
        xx::Append(s, ", \"minDetectRadius\":", this->minDetectRadius);
        xx::Append(s, ", \"moveFrames\":", this->moveFrames);
        xx::Append(s, ", \"dieFrames\":", this->dieFrames);
        xx::Append(s, ", \"touchRank\":", this->touchRank);
        xx::Append(s, ", \"shadowScale\":", this->shadowScale);
        xx::Append(s, ", \"shadowOffset\":", this->shadowOffset);
    }
    inline uint16_t Cannon::GetTypeId() const noexcept {
        return 62;
    }
    inline void Cannon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->angle);
        bb.Write(this->muzzleLen);
        bb.Write(this->quantity);
        bb.Write(this->numLimit);
        bb.Write(this->fireCD);
        bb.Write(this->radius);
        bb.Write(this->maxRadius);
        bb.Write(this->distance);
    }
    inline int Cannon::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->angle)) return r;
        if (int r = bb.Read(this->muzzleLen)) return r;
        if (int r = bb.Read(this->quantity)) return r;
        if (int r = bb.Read(this->numLimit)) return r;
        if (int r = bb.Read(this->fireCD)) return r;
        if (int r = bb.Read(this->radius)) return r;
        if (int r = bb.Read(this->maxRadius)) return r;
        if (int r = bb.Read(this->distance)) return r;
        return 0;
    }
    inline int Cannon::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Cannon::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.Cannon\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Cannon::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"angle\":", this->angle);
        xx::Append(s, ", \"muzzleLen\":", this->muzzleLen);
        xx::Append(s, ", \"quantity\":", this->quantity);
        xx::Append(s, ", \"numLimit\":", this->numLimit);
        xx::Append(s, ", \"fireCD\":", this->fireCD);
        xx::Append(s, ", \"radius\":", this->radius);
        xx::Append(s, ", \"maxRadius\":", this->maxRadius);
        xx::Append(s, ", \"distance\":", this->distance);
    }
    inline uint16_t Weapon::GetTypeId() const noexcept {
        return 64;
    }
    inline void Weapon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->distance);
        bb.Write(this->showNumFrames);
        bb.Write(this->cannon);
    }
    inline int Weapon::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->distance)) return r;
        if (int r = bb.Read(this->showNumFrames)) return r;
        if (int r = bb.Read(this->cannon)) return r;
        return 0;
    }
    inline int Weapon::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->cannon) {
            if (int r = this->cannon->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Weapon::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.Weapon\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Weapon::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"distance\":", this->distance);
        xx::Append(s, ", \"showNumFrames\":", this->showNumFrames);
        xx::Append(s, ", \"cannon\":", this->cannon);
    }
    inline uint16_t SpriteFrame::GetTypeId() const noexcept {
        return 69;
    }
    inline void SpriteFrame::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->plistName);
        bb.Write(this->frameName);
    }
    inline int SpriteFrame::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->plistName)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->frameName)) return r;
        return 0;
    }
    inline int SpriteFrame::InitCascadeCore(void* const& o) noexcept {
        return 0;
    }
    inline void SpriteFrame::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.SpriteFrame\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void SpriteFrame::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->plistName) xx::Append(s, ", \"plistName\":\"", this->plistName, "\"");
        else xx::Append(s, ", \"plistName\":nil");
        if (this->frameName) xx::Append(s, ", \"frameName\":\"", this->frameName, "\"");
        else xx::Append(s, ", \"frameName\":nil");
    }
    inline uint16_t Physics::GetTypeId() const noexcept {
        return 72;
    }
    inline void Physics::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->polygons);
    }
    inline int Physics::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->polygons)) return r;
        return 0;
    }
    inline int Physics::InitCascadeCore(void* const& o) noexcept {
        if (this->polygons) {
            if (int r = this->polygons->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void Physics::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.Physics\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Physics::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"polygons\":", this->polygons);
    }
    inline uint16_t FishSpriteFrame::GetTypeId() const noexcept {
        return 71;
    }
    inline void FishSpriteFrame::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->frame);
        bb.Write(this->physics);
        bb.Write(this->lockPoint);
        bb.Write(this->lockPoints);
        bb.Write(this->moveDistance);
    }
    inline int FishSpriteFrame::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->frame)) return r;
        if (int r = bb.Read(this->physics)) return r;
        if (int r = bb.Read(this->lockPoint)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->lockPoints)) return r;
        if (int r = bb.Read(this->moveDistance)) return r;
        return 0;
    }
    inline int FishSpriteFrame::InitCascade(void* const& o) noexcept {
        if (this->frame) {
            if (int r = this->frame->InitCascade(o)) return r;
        }
        if (this->physics) {
            if (int r = this->physics->InitCascade(o)) return r;
        }
        if (this->lockPoints) {
            if (int r = this->lockPoints->InitCascade(o)) return r;
        }
        return 0;
    }
    inline void FishSpriteFrame::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.FishSpriteFrame\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void FishSpriteFrame::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"frame\":", this->frame);
        xx::Append(s, ", \"physics\":", this->physics);
        xx::Append(s, ", \"lockPoint\":", this->lockPoint);
        xx::Append(s, ", \"lockPoints\":", this->lockPoints);
        xx::Append(s, ", \"moveDistance\":", this->moveDistance);
    }
    inline uint16_t BigFish::GetTypeId() const noexcept {
        return 85;
    }
    inline void BigFish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->moveFrameDistance);
        bb.Write(this->numChilds);
        bb.Write(this->childsAngleInc);
    }
    inline int BigFish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->moveFrameDistance)) return r;
        if (int r = bb.Read(this->numChilds)) return r;
        if (int r = bb.Read(this->childsAngleInc)) return r;
        return 0;
    }
    inline int BigFish::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void BigFish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Configs.BigFish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void BigFish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"moveFrameDistance\":", this->moveFrameDistance);
        xx::Append(s, ", \"numChilds\":", this->numChilds);
        xx::Append(s, ", \"childsAngleInc\":", this->childsAngleInc);
    }
}
}
namespace PKG {
	struct AllTypesRegister {
        AllTypesRegister() {
	        xx::BBuffer::Register<PKG::Generic::Success>(3);
	        xx::BBuffer::Register<PKG::Generic::Error>(4);
	        xx::BBuffer::Register<PKG::Generic::Ping>(5);
	        xx::BBuffer::Register<PKG::Generic::Pong>(6);
	        xx::BBuffer::Register<PKG::CatchFish_Client::EnterSuccess>(7);
	        xx::BBuffer::Register<PKG::CatchFish::Scene>(8);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Player_s>>(9);
	        xx::BBuffer::Register<PKG::CatchFish::Player>(10);
	        xx::BBuffer::Register<PKG::CatchFish_Client::FrameEvents>(11);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Events::Event_s>>(12);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Event>(13);
	        xx::BBuffer::Register<PKG::Client_CatchFish::Enter>(14);
	        xx::BBuffer::Register<PKG::Client_CatchFish::Bet>(87);
	        xx::BBuffer::Register<PKG::Client_CatchFish::Fire>(15);
	        xx::BBuffer::Register<PKG::Client_CatchFish::Hit>(16);
	        xx::BBuffer::Register<::xx::Random>(17);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Fish_s>>(18);
	        xx::BBuffer::Register<PKG::CatchFish::Fish>(19);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Item_s>>(20);
	        xx::BBuffer::Register<PKG::CatchFish::Item>(21);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::FishBorn_s>>(22);
	        xx::BBuffer::Register<PKG::CatchFish::FishBorn>(23);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Stage>(24);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Sits>>(25);
	        xx::BBuffer::Register<xx::List<std::weak_ptr<PKG::CatchFish::Player>>>(26);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Cannon_s>>(27);
	        xx::BBuffer::Register<PKG::CatchFish::Cannon>(28);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Weapon_s>>(29);
	        xx::BBuffer::Register<PKG::CatchFish::Weapon>(30);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Bullet_s>>(31);
	        xx::BBuffer::Register<PKG::CatchFish::Bullet>(32);
	        xx::BBuffer::Register<PKG::CatchFish::MoveItem>(33);
	        xx::BBuffer::Register<PKG::CatchFish::Way>(34);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::WayPoint>>(36);
	        xx::BBuffer::Register<PKG::CatchFish::WayFish>(80);
	        xx::BBuffer::Register<PKG::CatchFish::RoundFish>(82);
	        xx::BBuffer::Register<PKG::CatchFish::BigFish>(83);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::RoundFish_s>>(84);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Enter>(37);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Leave>(38);
	        xx::BBuffer::Register<PKG::CatchFish::Events::NoMoney>(39);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Refund>(40);
	        xx::BBuffer::Register<PKG::CatchFish::Events::FishDead>(41);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Events::FishDead_s>>(42);
	        xx::BBuffer::Register<PKG::CatchFish::Events::PushWeapon>(43);
	        xx::BBuffer::Register<PKG::CatchFish::Events::PushFish>(44);
	        xx::BBuffer::Register<PKG::CatchFish::Events::OpenAutoLock>(45);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Aim>(46);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CloseAutoLock>(47);
	        xx::BBuffer::Register<PKG::CatchFish::Events::OpenAutoFire>(48);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CloseAutoFire>(49);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Fire>(50);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CannonSwitch>(51);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CannonCoinChange>(52);
	        xx::BBuffer::Register<PKG::CatchFish::Events::DebugInfo>(53);
	        xx::BBuffer::Register<xx::List<int32_t>>(54);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Stages::StageElement_s>>(74);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::StageElement>(75);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Emitter_RandomFishs>(76);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Monitor_KeepFatFish>(77);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Monitor_KeepBigFish>(86);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Emitter_RingFishs>(78);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Emitter_CircleFishs>(81);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Config>(57);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Way_s>>(58);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::Fish_s>>(59);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Fish>(60);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::Cannon_s>>(61);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Cannon>(62);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::Weapon_s>>(63);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Weapon>(64);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Stages::Stage_s>>(65);
	        xx::BBuffer::Register<xx::List<::xx::Pos>>(66);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Item>(67);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::SpriteFrame_s>>(68);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::SpriteFrame>(69);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::FishSpriteFrame_s>>(70);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::FishSpriteFrame>(71);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Physics>(72);
	        xx::BBuffer::Register<xx::List<xx::List_s<::xx::Pos>>>(73);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::BigFish>(85);
        }
	};
	inline AllTypesRegister AllTypesRegisterInstance;   // for auto register at program startup
}
