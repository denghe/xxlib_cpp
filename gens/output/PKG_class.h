#pragma once
namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "00b260547a113cca5a55d79acbbef0ad";
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
namespace CatchFish {
    // 场景
    struct Scene;
    using Scene_s = std::shared_ptr<Scene>;
    using Scene_w = std::weak_ptr<Scene>;

    // 玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
    struct Player;
    using Player_s = std::shared_ptr<Player>;
    using Player_w = std::weak_ptr<Player>;

    // 场景元素的共通基类
    struct Item;
    using Item_s = std::shared_ptr<Item>;
    using Item_w = std::weak_ptr<Item>;

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

    // 鱼基类 ( 下列属性适合大多数鱼, 不一定适合部分 boss )
    struct Fish;
    using Fish_s = std::shared_ptr<Fish>;
    using Fish_w = std::weak_ptr<Fish>;

    // 武器基类 ( 有一些特殊鱼死后会变做 某种武器 / 炮台，死时有个滞空展示时间，被用于解决网络同步延迟。所有端应该在展示时间结束前收到该预约。展示完成后武器将飞向炮台变为附加炮台 )
    struct Weapon;
    using Weapon_s = std::shared_ptr<Weapon>;
    using Weapon_w = std::weak_ptr<Weapon>;

    // 定时器基类
    struct Timer;
    using Timer_s = std::shared_ptr<Timer>;
    using Timer_w = std::weak_ptr<Timer>;

    // 预约出鱼
    struct FishBorn;
    using FishBorn_s = std::shared_ptr<FishBorn>;
    using FishBorn_w = std::weak_ptr<FishBorn>;

    // 轨迹点
    struct WayPoint;
    using WayPoint_s = std::shared_ptr<WayPoint>;
    using WayPoint_w = std::weak_ptr<WayPoint>;

    // 轨迹. 预约下发安全, 将复制轨迹完整数据
    struct Way;
    using Way_s = std::shared_ptr<Way>;
    using Way_w = std::weak_ptr<Way>;

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

    // 通知: 玩家充值( 解除破产? )
    struct Charge;
    using Charge_s = std::shared_ptr<Charge>;
    using Charge_w = std::weak_ptr<Charge>;

    // 通知: 鱼被打死
    struct FishDead;
    using FishDead_s = std::shared_ptr<FishDead>;
    using FishDead_w = std::weak_ptr<FishDead>;

    // 通知: 下发已生效 Weapon, 需要判断 flyFrameNumber, 放入 player.weapon 队列
    struct PushWeapon;
    using PushWeapon_s = std::shared_ptr<PushWeapon>;
    using PushWeapon_w = std::weak_ptr<PushWeapon>;

    // 预约: 出鱼( 需判定 beginFrameNumber ), 放入 scene.timers 队列
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

}
namespace CatchFish::Stages {
    // 游戏关卡. 位于 Stage.timers 中的 timer, 使用 stageFrameNumber 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化
    struct Stage;
    using Stage_s = std::shared_ptr<Stage>;
    using Stage_w = std::weak_ptr<Stage>;

    // 服务器本地脚本( 关卡元素 )
    struct Script;
    using Script_s = std::shared_ptr<Script>;
    using Script_w = std::weak_ptr<Script>;

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
    // 轨迹点
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
        int32_t id = 0;

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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish {
    // 场景元素的共通基类
    struct Item : xx::Object {
        // 自增id ( 服务器实时下发的id为负 )
        int32_t id = 0;
        // 位于容器时的下标 ( 用于快速交换删除 )
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish {
    // 子弹 & 鱼 & 武器 的基类
    struct MoveItem : PKG::CatchFish::Item {
        // 中心点坐标
        ::xx::Pos pos;
        // 当前角度
        float angle = 0;

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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 定时器基类
    struct Timer : xx::Object {
        // 开始 / 生效帧编号
        int32_t beginFrameNumber = 0;

        typedef Timer ThisType;
        typedef xx::Object BaseType;
	    Timer() = default;
		Timer(Timer const&) = delete;
		Timer& operator=(Timer const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish::Events {
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 转发: 发子弹( 单次 ). 非特殊子弹, 只可能是 cannons[0] 原始炮台发射
    struct Fire : PKG::CatchFish::Events::Event {
        // 起始帧编号 ( 来自客户端 )
        int32_t frameNumber = 0;
        // 子弹id
        int32_t bulletId = 0;
        // 子弹的发射目标坐标
        ::xx::Pos tarPos;
        // 子弹的发射角度
        float tarAngle = 0;
        // 币值 / 倍率
        int64_t coin = 0;

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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish::Stages {
    // 游戏关卡. 位于 Stage.timers 中的 timer, 使用 stageFrameNumber 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化
    struct Stage : xx::Object {
        // 同下标
        int32_t id = 0;
        // 关卡帧编号( clone 后需清0. 每帧 +1 )
        int32_t stageFrameNumber = 0;
        // 当前阶段结束时间点( clone 后需修正 )
        int32_t endFrameNumber = 0;
        // 关卡元素集合
        xx::List_s<PKG::CatchFish::Timer_s> timers;

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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish::Stages {
    // 服务器本地脚本( 关卡元素 )
    struct Script : PKG::CatchFish::Timer {
        int32_t lineNumber = 0;

        typedef Script ThisType;
        typedef PKG::CatchFish::Timer BaseType;
	    Script() = default;
		Script(Script const&) = delete;
		Script& operator=(Script const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish::Configs {
    // 游戏配置主体
    struct Config : xx::Object {
        // 所有预生成轨迹( 轨迹创建后先填充到这, 再与具体的鱼 bind, 以达到重用的目的 )
        xx::List_s<PKG::CatchFish::Way_s> ways;
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
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 鱼配置基类 ( 派生类中不再包含 sprite frame 相关, 以便于资源加载管理扫描 )
    struct Fish : PKG::CatchFish::Configs::Item {
        // 金币 / 倍率随机范围 ( 最小值 )
        int64_t minCoin = 0;
        // 金币 / 倍率随机范围 ( 最大值 )
        int64_t maxCoin = 0;
        // 基于整个鱼的最大晃动范围的圆形碰撞检测半径( 粗判. <= 0 则直接进行细判 )
        float maxDetectRadius = 0;
        // 与该鱼绑定的默认路径集合( 不含鱼阵的路径 ), 为随机路径创造便利
        xx::List_s<PKG::CatchFish::Way_s> ways;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 炮台 & 子弹配置基类
    struct Cannon : PKG::CatchFish::Configs::Item {
        // 初始角度
        int32_t angle = 0;
        // 炮管长度
        float muzzleLen = 0;
        // 拥有的数量( -1: 无限 )
        int32_t quantity = 0;
        // 同屏颗数限制 ( 到达上限就不允许继续发射 )
        int32_t numLimit = 0;
        // 发射间隔帧数
        int32_t shootCD = 0;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish::Events {
    // 转发: 切换炮台倍率
    struct CannonCoinChange : PKG::CatchFish::Events::Event {
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 预约: 出鱼( 需判定 beginFrameNumber ), 放入 scene.timers 队列
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish::Configs {
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
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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
        // 所有玩家
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
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
    struct Player : xx::Object {
        // 账号id. 用于定位玩家 ( 填充自 db )
        int32_t id = 0;
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
        typedef xx::Object BaseType;
	    Player() = default;
		Player(Player const&) = delete;
		Player& operator=(Player const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
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
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 子弹基类
    struct Bullet : PKG::CatchFish::MoveItem {
        // 每帧的直线移动坐标增量( 60fps )
        ::xx::Pos moveInc;
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
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 鱼基类 ( 下列属性适合大多数鱼, 不一定适合部分 boss )
    struct Fish : PKG::CatchFish::MoveItem {
        // 配置id
        int32_t cfgId = 0;
        // 币值 / 倍率
        int64_t coin = 0;
        // 移动速度系数 ( 默认为 1 )
        float speedScale = 0;
        // 运行时缩放比例( 通常为 1 )
        float scale = 0;
        // 移动轨迹. 动态生成, 不引用自 cfg. 同步时被复制. 如果该值为空, 则启用 wayIndex ( 常见于非直线鱼 )
        PKG::CatchFish::Way_s way;
        // 移动轨迹 于 cfg.ways 的下标. 启用优先级低于 way
        int32_t wayIndex = 0;
        // 当前轨迹点下标
        int32_t wayPointIndex = 0;
        // 当前轨迹点上的已前进距离
        float wayPointDistance = 0;
        // 当前帧下标( 每帧循环累加 )
        int32_t spriteFrameIndex = 0;
        // 帧比值, 平时为 1, 如果为 0 则表示鱼不动( 比如实现冰冻效果 ), 帧图也不更新. 如果大于 1, 则需要在 1 帧内多次驱动该鱼( 比如实现快速离场的效果 )
        int32_t frameRatio = 0;
        // 是否为在鱼线上倒着移动( 默认否 )
        bool reverse = false;

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
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 预约出鱼
    struct FishBorn : PKG::CatchFish::Timer {
        // 当 currentFrameNumber == beginFrameNumber 时，将 fish 放入 Scene.fishs 并自杀
        PKG::CatchFish::Fish_s fish;

        typedef FishBorn ThisType;
        typedef PKG::CatchFish::Timer BaseType;
	    FishBorn() = default;
		FishBorn(FishBorn const&) = delete;
		FishBorn& operator=(FishBorn const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 轨迹. 预约下发安全, 将复制轨迹完整数据
    struct Way : xx::Object {
        // 轨迹点集合
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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
        bool dead = false;
        // 剩余金币值
        int64_t coin = 0;
        // 座位
        PKG::CatchFish::Sits sit = (PKG::CatchFish::Sits)0;
        // 炮台id
        int32_t cannonId = 0;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 通知: 玩家充值( 解除破产? )
    struct Charge : PKG::CatchFish::Events::Event {
        // 新增币值
        int64_t coin = 0;

        typedef Charge ThisType;
        typedef PKG::CatchFish::Events::Event BaseType;
	    Charge() = default;
		Charge(Charge const&) = delete;
		Charge& operator=(Charge const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int InitCascade(void* const& o = nullptr) noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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

        inline static std::shared_ptr<ThisType> defaultInstance;
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
    template<> struct TypeId<PKG::CatchFish::Scene> { static const uint16_t value = 8; };
    template<> struct TypeId<::xx::Random> { static const uint16_t value = 9; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Fish_s>> { static const uint16_t value = 10; };
    template<> struct TypeId<PKG::CatchFish::Fish> { static const uint16_t value = 11; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Item_s>> { static const uint16_t value = 12; };
    template<> struct TypeId<PKG::CatchFish::Item> { static const uint16_t value = 13; };
    template<> struct TypeId<xx::List<PKG::CatchFish::FishBorn_s>> { static const uint16_t value = 14; };
    template<> struct TypeId<PKG::CatchFish::FishBorn> { static const uint16_t value = 15; };
    template<> struct TypeId<PKG::CatchFish::Stages::Stage> { static const uint16_t value = 16; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Sits>> { static const uint16_t value = 17; };
    template<> struct TypeId<xx::List<std::weak_ptr<PKG::CatchFish::Player>>> { static const uint16_t value = 18; };
    template<> struct TypeId<PKG::CatchFish::Player> { static const uint16_t value = 22; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Cannon_s>> { static const uint16_t value = 23; };
    template<> struct TypeId<PKG::CatchFish::Cannon> { static const uint16_t value = 19; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Weapon_s>> { static const uint16_t value = 24; };
    template<> struct TypeId<PKG::CatchFish::Weapon> { static const uint16_t value = 25; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Bullet_s>> { static const uint16_t value = 20; };
    template<> struct TypeId<PKG::CatchFish::Bullet> { static const uint16_t value = 21; };
    template<> struct TypeId<PKG::CatchFish::MoveItem> { static const uint16_t value = 26; };
    template<> struct TypeId<PKG::CatchFish::Way> { static const uint16_t value = 49; };
    template<> struct TypeId<PKG::CatchFish::Timer> { static const uint16_t value = 27; };
    template<> struct TypeId<xx::List<PKG::CatchFish::WayPoint>> { static const uint16_t value = 64; };
    template<> struct TypeId<PKG::CatchFish::Events::Event> { static const uint16_t value = 28; };
    template<> struct TypeId<PKG::CatchFish::Events::Enter> { static const uint16_t value = 29; };
    template<> struct TypeId<PKG::CatchFish::Events::Leave> { static const uint16_t value = 30; };
    template<> struct TypeId<PKG::CatchFish::Events::NoMoney> { static const uint16_t value = 31; };
    template<> struct TypeId<PKG::CatchFish::Events::Charge> { static const uint16_t value = 32; };
    template<> struct TypeId<PKG::CatchFish::Events::FishDead> { static const uint16_t value = 33; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Events::FishDead_s>> { static const uint16_t value = 34; };
    template<> struct TypeId<PKG::CatchFish::Events::PushWeapon> { static const uint16_t value = 35; };
    template<> struct TypeId<PKG::CatchFish::Events::PushFish> { static const uint16_t value = 36; };
    template<> struct TypeId<PKG::CatchFish::Events::OpenAutoLock> { static const uint16_t value = 37; };
    template<> struct TypeId<PKG::CatchFish::Events::Aim> { static const uint16_t value = 38; };
    template<> struct TypeId<PKG::CatchFish::Events::CloseAutoLock> { static const uint16_t value = 39; };
    template<> struct TypeId<PKG::CatchFish::Events::OpenAutoFire> { static const uint16_t value = 40; };
    template<> struct TypeId<PKG::CatchFish::Events::CloseAutoFire> { static const uint16_t value = 41; };
    template<> struct TypeId<PKG::CatchFish::Events::Fire> { static const uint16_t value = 42; };
    template<> struct TypeId<PKG::CatchFish::Events::CannonSwitch> { static const uint16_t value = 43; };
    template<> struct TypeId<PKG::CatchFish::Events::CannonCoinChange> { static const uint16_t value = 44; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Timer_s>> { static const uint16_t value = 45; };
    template<> struct TypeId<PKG::CatchFish::Stages::Script> { static const uint16_t value = 46; };
    template<> struct TypeId<PKG::CatchFish::Configs::Config> { static const uint16_t value = 47; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Way_s>> { static const uint16_t value = 48; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::Fish_s>> { static const uint16_t value = 50; };
    template<> struct TypeId<PKG::CatchFish::Configs::Fish> { static const uint16_t value = 51; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::Cannon_s>> { static const uint16_t value = 52; };
    template<> struct TypeId<PKG::CatchFish::Configs::Cannon> { static const uint16_t value = 53; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::Weapon_s>> { static const uint16_t value = 54; };
    template<> struct TypeId<PKG::CatchFish::Configs::Weapon> { static const uint16_t value = 55; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Stages::Stage_s>> { static const uint16_t value = 56; };
    template<> struct TypeId<xx::List<::xx::Pos>> { static const uint16_t value = 57; };
    template<> struct TypeId<PKG::CatchFish::Configs::Item> { static const uint16_t value = 58; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::SpriteFrame_s>> { static const uint16_t value = 59; };
    template<> struct TypeId<PKG::CatchFish::Configs::SpriteFrame> { static const uint16_t value = 60; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Configs::FishSpriteFrame_s>> { static const uint16_t value = 61; };
    template<> struct TypeId<PKG::CatchFish::Configs::FishSpriteFrame> { static const uint16_t value = 62; };
    template<> struct TypeId<PKG::CatchFish::Configs::Physics> { static const uint16_t value = 65; };
    template<> struct TypeId<xx::List<xx::List_s<::xx::Pos>>> { static const uint16_t value = 63; };
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
    inline int Scene::InitCascade(void* const& o) noexcept {
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
    inline uint16_t Player::GetTypeId() const noexcept {
        return 22;
    }
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
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
        if (int r = bb.Read(this->id)) return r;
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
    inline int Player::InitCascade(void* const& o) noexcept {
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
        xx::Append(s, ", \"id\":", this->id);
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
    inline uint16_t Item::GetTypeId() const noexcept {
        return 13;
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
    inline uint16_t Cannon::GetTypeId() const noexcept {
        return 19;
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
    inline int Cannon::InitCascade(void* const& o) noexcept {
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
        return 26;
    }
    inline void MoveItem::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->pos);
        bb.Write(this->angle);
    }
    inline int MoveItem::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->pos)) return r;
        if (int r = bb.Read(this->angle)) return r;
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
    }
    inline uint16_t Bullet::GetTypeId() const noexcept {
        return 21;
    }
    inline void Bullet::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->moveInc);
        bb.Write(this->coin);
    }
    inline int Bullet::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->moveInc)) return r;
        if (int r = bb.Read(this->coin)) return r;
        return 0;
    }
    inline int Bullet::InitCascade(void* const& o) noexcept {
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
        xx::Append(s, ", \"moveInc\":", this->moveInc);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t Fish::GetTypeId() const noexcept {
        return 11;
    }
    inline void Fish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->cfgId);
        bb.Write(this->coin);
        bb.Write(this->speedScale);
        bb.Write(this->scale);
        bb.Write(this->way);
        bb.Write(this->wayIndex);
        bb.Write(this->wayPointIndex);
        bb.Write(this->wayPointDistance);
        bb.Write(this->spriteFrameIndex);
        bb.Write(this->frameRatio);
        bb.Write(this->reverse);
    }
    inline int Fish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->cfgId)) return r;
        if (int r = bb.Read(this->coin)) return r;
        if (int r = bb.Read(this->speedScale)) return r;
        if (int r = bb.Read(this->scale)) return r;
        if (int r = bb.Read(this->way)) return r;
        if (int r = bb.Read(this->wayIndex)) return r;
        if (int r = bb.Read(this->wayPointIndex)) return r;
        if (int r = bb.Read(this->wayPointDistance)) return r;
        if (int r = bb.Read(this->spriteFrameIndex)) return r;
        if (int r = bb.Read(this->frameRatio)) return r;
        if (int r = bb.Read(this->reverse)) return r;
        return 0;
    }
    inline int Fish::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        if (this->way) {
            if (int r = this->way->InitCascade(o)) return r;
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
        xx::Append(s, ", \"way\":", this->way);
        xx::Append(s, ", \"wayIndex\":", this->wayIndex);
        xx::Append(s, ", \"wayPointIndex\":", this->wayPointIndex);
        xx::Append(s, ", \"wayPointDistance\":", this->wayPointDistance);
        xx::Append(s, ", \"spriteFrameIndex\":", this->spriteFrameIndex);
        xx::Append(s, ", \"frameRatio\":", this->frameRatio);
        xx::Append(s, ", \"reverse\":", this->reverse);
    }
    inline uint16_t Weapon::GetTypeId() const noexcept {
        return 25;
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
    inline uint16_t Timer::GetTypeId() const noexcept {
        return 27;
    }
    inline void Timer::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->beginFrameNumber);
    }
    inline int Timer::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->beginFrameNumber)) return r;
        return 0;
    }
    inline int Timer::InitCascade(void* const& o) noexcept {
        return 0;
    }
    inline void Timer::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Timer\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Timer::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"beginFrameNumber\":", this->beginFrameNumber);
    }
    inline uint16_t FishBorn::GetTypeId() const noexcept {
        return 15;
    }
    inline void FishBorn::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->fish);
    }
    inline int FishBorn::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->fish)) return r;
        return 0;
    }
    inline int FishBorn::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
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
        xx::Append(s, ", \"fish\":", this->fish);
    }
    inline uint16_t Way::GetTypeId() const noexcept {
        return 49;
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
}
namespace CatchFish::Events {
    inline uint16_t Event::GetTypeId() const noexcept {
        return 28;
    }
    inline void Event::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
    }
    inline int Event::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
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
        xx::Append(s, ", \"id\":", this->id);
    }
    inline uint16_t Enter::GetTypeId() const noexcept {
        return 29;
    }
    inline void Enter::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->nickname);
        bb.Write(this->avatar_id);
        bb.Write(this->dead);
        bb.Write(this->coin);
        bb.Write(this->sit);
        bb.Write(this->cannonId);
        bb.Write(this->cannonCfgId);
        bb.Write(this->cannonCoin);
    }
    inline int Enter::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->nickname)) return r;
        if (int r = bb.Read(this->avatar_id)) return r;
        if (int r = bb.Read(this->dead)) return r;
        if (int r = bb.Read(this->coin)) return r;
        if (int r = bb.Read(this->sit)) return r;
        if (int r = bb.Read(this->cannonId)) return r;
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
        xx::Append(s, ", \"dead\":", this->dead);
        xx::Append(s, ", \"coin\":", this->coin);
        xx::Append(s, ", \"sit\":", this->sit);
        xx::Append(s, ", \"cannonId\":", this->cannonId);
        xx::Append(s, ", \"cannonCfgId\":", this->cannonCfgId);
        xx::Append(s, ", \"cannonCoin\":", this->cannonCoin);
    }
    inline uint16_t Leave::GetTypeId() const noexcept {
        return 30;
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
        return 31;
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
    inline uint16_t Charge::GetTypeId() const noexcept {
        return 32;
    }
    inline void Charge::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->coin);
    }
    inline int Charge::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->coin)) return r;
        return 0;
    }
    inline int Charge::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Charge::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Events.Charge\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Charge::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t FishDead::GetTypeId() const noexcept {
        return 33;
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
        return 35;
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
        return 36;
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
        return 37;
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
        return 38;
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
        return 39;
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
        return 40;
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
        return 41;
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
        return 42;
    }
    inline void Fire::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->frameNumber);
        bb.Write(this->bulletId);
        bb.Write(this->tarPos);
        bb.Write(this->tarAngle);
        bb.Write(this->coin);
    }
    inline int Fire::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->frameNumber)) return r;
        if (int r = bb.Read(this->bulletId)) return r;
        if (int r = bb.Read(this->tarPos)) return r;
        if (int r = bb.Read(this->tarAngle)) return r;
        if (int r = bb.Read(this->coin)) return r;
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
        xx::Append(s, ", \"bulletId\":", this->bulletId);
        xx::Append(s, ", \"tarPos\":", this->tarPos);
        xx::Append(s, ", \"tarAngle\":", this->tarAngle);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t CannonSwitch::GetTypeId() const noexcept {
        return 43;
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
        return 44;
    }
    inline void CannonCoinChange::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->coin);
    }
    inline int CannonCoinChange::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
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
        xx::Append(s, ", \"coin\":", this->coin);
    }
}
namespace CatchFish::Stages {
    inline uint16_t Stage::GetTypeId() const noexcept {
        return 16;
    }
    inline void Stage::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->stageFrameNumber);
        bb.Write(this->endFrameNumber);
        bb.Write(this->timers);
    }
    inline int Stage::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->stageFrameNumber)) return r;
        if (int r = bb.Read(this->endFrameNumber)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->timers)) return r;
        return 0;
    }
    inline int Stage::InitCascade(void* const& o) noexcept {
        if (this->timers) {
            if (int r = this->timers->InitCascade(o)) return r;
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
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"stageFrameNumber\":", this->stageFrameNumber);
        xx::Append(s, ", \"endFrameNumber\":", this->endFrameNumber);
        xx::Append(s, ", \"timers\":", this->timers);
    }
    inline uint16_t Script::GetTypeId() const noexcept {
        return 46;
    }
    inline void Script::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->lineNumber);
    }
    inline int Script::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        if (int r = bb.Read(this->lineNumber)) return r;
        return 0;
    }
    inline int Script::InitCascade(void* const& o) noexcept {
        if (int r = this->BaseType::InitCascade(o)) return r;
        return 0;
    }
    inline void Script::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.Stages.Script\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Script::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"lineNumber\":", this->lineNumber);
    }
}
namespace CatchFish::Configs {
    inline uint16_t Config::GetTypeId() const noexcept {
        return 47;
    }
    inline void Config::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->ways);
        bb.Write(this->fishs);
        bb.Write(this->cannons);
        bb.Write(this->weapons);
        bb.Write(this->stages);
        bb.Write(this->sitPositons);
        bb.Write(this->aimTouchRadius);
    }
    inline int Config::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->ways)) return r;
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
        return 0;
    }
    inline int Config::InitCascade(void* const& o) noexcept {
        if (this->ways) {
            if (int r = this->ways->InitCascade(o)) return r;
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
        xx::Append(s, ", \"ways\":", this->ways);
        xx::Append(s, ", \"fishs\":", this->fishs);
        xx::Append(s, ", \"cannons\":", this->cannons);
        xx::Append(s, ", \"weapons\":", this->weapons);
        xx::Append(s, ", \"stages\":", this->stages);
        xx::Append(s, ", \"sitPositons\":", this->sitPositons);
        xx::Append(s, ", \"aimTouchRadius\":", this->aimTouchRadius);
    }
    inline uint16_t Item::GetTypeId() const noexcept {
        return 58;
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
        return 51;
    }
    inline void Fish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->minCoin);
        bb.Write(this->maxCoin);
        bb.Write(this->maxDetectRadius);
        bb.Write(this->ways);
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
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->ways)) return r;
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
        if (this->ways) {
            if (int r = this->ways->InitCascade(o)) return r;
        }
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
        xx::Append(s, ", \"ways\":", this->ways);
        xx::Append(s, ", \"moveFrames\":", this->moveFrames);
        xx::Append(s, ", \"dieFrames\":", this->dieFrames);
        xx::Append(s, ", \"touchRank\":", this->touchRank);
        xx::Append(s, ", \"shadowScale\":", this->shadowScale);
        xx::Append(s, ", \"shadowOffset\":", this->shadowOffset);
    }
    inline uint16_t Cannon::GetTypeId() const noexcept {
        return 53;
    }
    inline void Cannon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->angle);
        bb.Write(this->muzzleLen);
        bb.Write(this->quantity);
        bb.Write(this->numLimit);
        bb.Write(this->shootCD);
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
        if (int r = bb.Read(this->shootCD)) return r;
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
        xx::Append(s, ", \"shootCD\":", this->shootCD);
        xx::Append(s, ", \"radius\":", this->radius);
        xx::Append(s, ", \"maxRadius\":", this->maxRadius);
        xx::Append(s, ", \"distance\":", this->distance);
    }
    inline uint16_t Weapon::GetTypeId() const noexcept {
        return 55;
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
        return 60;
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
    inline int SpriteFrame::InitCascade(void* const& o) noexcept {
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
        return 65;
    }
    inline void Physics::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->polygons);
    }
    inline int Physics::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->polygons)) return r;
        return 0;
    }
    inline int Physics::InitCascade(void* const& o) noexcept {
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
        return 62;
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
}
}
namespace PKG {
	struct AllTypesRegister {
        AllTypesRegister() {
	        xx::BBuffer::Register<PKG::Generic::Success>(3);
	        xx::BBuffer::Register<PKG::Generic::Error>(4);
	        xx::BBuffer::Register<PKG::Generic::Ping>(5);
	        xx::BBuffer::Register<PKG::Generic::Pong>(6);
	        xx::BBuffer::Register<PKG::CatchFish::Scene>(8);
	        xx::BBuffer::Register<::xx::Random>(9);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Fish_s>>(10);
	        xx::BBuffer::Register<PKG::CatchFish::Fish>(11);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Item_s>>(12);
	        xx::BBuffer::Register<PKG::CatchFish::Item>(13);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::FishBorn_s>>(14);
	        xx::BBuffer::Register<PKG::CatchFish::FishBorn>(15);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Stage>(16);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Sits>>(17);
	        xx::BBuffer::Register<xx::List<std::weak_ptr<PKG::CatchFish::Player>>>(18);
	        xx::BBuffer::Register<PKG::CatchFish::Player>(22);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Cannon_s>>(23);
	        xx::BBuffer::Register<PKG::CatchFish::Cannon>(19);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Weapon_s>>(24);
	        xx::BBuffer::Register<PKG::CatchFish::Weapon>(25);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Bullet_s>>(20);
	        xx::BBuffer::Register<PKG::CatchFish::Bullet>(21);
	        xx::BBuffer::Register<PKG::CatchFish::MoveItem>(26);
	        xx::BBuffer::Register<PKG::CatchFish::Way>(49);
	        xx::BBuffer::Register<PKG::CatchFish::Timer>(27);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::WayPoint>>(64);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Event>(28);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Enter>(29);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Leave>(30);
	        xx::BBuffer::Register<PKG::CatchFish::Events::NoMoney>(31);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Charge>(32);
	        xx::BBuffer::Register<PKG::CatchFish::Events::FishDead>(33);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Events::FishDead_s>>(34);
	        xx::BBuffer::Register<PKG::CatchFish::Events::PushWeapon>(35);
	        xx::BBuffer::Register<PKG::CatchFish::Events::PushFish>(36);
	        xx::BBuffer::Register<PKG::CatchFish::Events::OpenAutoLock>(37);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Aim>(38);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CloseAutoLock>(39);
	        xx::BBuffer::Register<PKG::CatchFish::Events::OpenAutoFire>(40);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CloseAutoFire>(41);
	        xx::BBuffer::Register<PKG::CatchFish::Events::Fire>(42);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CannonSwitch>(43);
	        xx::BBuffer::Register<PKG::CatchFish::Events::CannonCoinChange>(44);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Timer_s>>(45);
	        xx::BBuffer::Register<PKG::CatchFish::Stages::Script>(46);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Config>(47);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Way_s>>(48);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::Fish_s>>(50);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Fish>(51);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::Cannon_s>>(52);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Cannon>(53);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::Weapon_s>>(54);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Weapon>(55);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Stages::Stage_s>>(56);
	        xx::BBuffer::Register<xx::List<::xx::Pos>>(57);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Item>(58);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::SpriteFrame_s>>(59);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::SpriteFrame>(60);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Configs::FishSpriteFrame_s>>(61);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::FishSpriteFrame>(62);
	        xx::BBuffer::Register<PKG::CatchFish::Configs::Physics>(65);
	        xx::BBuffer::Register<xx::List<xx::List_s<::xx::Pos>>>(63);
        }
	};
	inline AllTypesRegister AllTypesRegisterInstance;   // for auto register at program startup
}
