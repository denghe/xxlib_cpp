#pragma once
namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "8a0380cef9eda62800e6c66e92235f9f";
    };

namespace Test {
    struct Foo;
    using Foo_s = std::shared_ptr<Foo>;
    using Foo_w = std::weak_ptr<Foo>;

    struct Player;
    using Player_s = std::shared_ptr<Player>;
    using Player_w = std::weak_ptr<Player>;

    struct Scene;
    using Scene_s = std::shared_ptr<Scene>;
    using Scene_w = std::weak_ptr<Scene>;

    struct Fish;
    using Fish_s = std::shared_ptr<Fish>;
    using Fish_w = std::weak_ptr<Fish>;

    struct EnterSuccess;
    using EnterSuccess_s = std::shared_ptr<EnterSuccess>;
    using EnterSuccess_w = std::weak_ptr<EnterSuccess>;

}
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
namespace CatchFishConfig {
    // 帧动画基本信息
    struct SpriteFrameBase;
    using SpriteFrameBase_s = std::shared_ptr<SpriteFrameBase>;
    using SpriteFrameBase_w = std::weak_ptr<SpriteFrameBase>;

    // 帧动画
    struct SpriteFrame;
    using SpriteFrame_s = std::shared_ptr<SpriteFrame>;
    using SpriteFrame_w = std::weak_ptr<SpriteFrame>;

    // 鱼配置信息基类 ( 部分 boss 可能具有更多 frame, state 等配置参数 )
    struct Fish;
    using Fish_s = std::shared_ptr<Fish>;
    using Fish_w = std::weak_ptr<Fish>;

    struct Cannon;
    using Cannon_s = std::shared_ptr<Cannon>;
    using Cannon_w = std::weak_ptr<Cannon>;

    struct Bullet;
    using Bullet_s = std::shared_ptr<Bullet>;
    using Bullet_w = std::weak_ptr<Bullet>;

    // 游戏配置信息( 配置信息并不会随着网络同步而下发, 反序列化后需要手工还原 )
    struct Config;
    using Config_s = std::shared_ptr<Config>;
    using Config_w = std::weak_ptr<Config>;

}
namespace CatchFish {
    // 场景基础配置参数 ( 主要来自 db )
    struct SceneConfig;
    using SceneConfig_s = std::shared_ptr<SceneConfig>;
    using SceneConfig_w = std::weak_ptr<SceneConfig>;

    // 场景
    struct Scene;
    using Scene_s = std::shared_ptr<Scene>;
    using Scene_w = std::weak_ptr<Scene>;

    // 炮台 ( 基类 )
    struct Cannon;
    using Cannon_s = std::shared_ptr<Cannon>;
    using Cannon_w = std::weak_ptr<Cannon>;

    // 玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )
    struct Player;
    using Player_s = std::shared_ptr<Player>;
    using Player_w = std::weak_ptr<Player>;

    // 子弹 & 鱼 & 武器 的基类
    struct MoveObject;
    using MoveObject_s = std::shared_ptr<MoveObject>;
    using MoveObject_w = std::weak_ptr<MoveObject>;

    // 子弹基类
    struct Bullet;
    using Bullet_s = std::shared_ptr<Bullet>;
    using Bullet_w = std::weak_ptr<Bullet>;

    // 鱼基类
    struct Fish;
    using Fish_s = std::shared_ptr<Fish>;
    using Fish_w = std::weak_ptr<Fish>;

    // 武器 ( 有一些特殊鱼死后会变做 某种武器的长相，并花一段世家飞向玩家炮台 )
    struct Weapon;
    using Weapon_s = std::shared_ptr<Weapon>;
    using Weapon_w = std::weak_ptr<Weapon>;

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
namespace CatchFishConfig {
    // 帧动画基本信息
    struct SpriteFrameBase : xx::Object {
        // 贴图名. 通过遍历扫描去重之后, 结合关卡数据, 可以针对即将出现的鱼以及短期内不再出现的鱼做异步加载/卸载
        std::string_s textureName;
        // 帧名
        std::string_s frameName;

        typedef SpriteFrameBase ThisType;
        typedef xx::Object BaseType;
	    SpriteFrameBase() = default;
		SpriteFrameBase(SpriteFrameBase const&) = delete;
		SpriteFrameBase& operator=(SpriteFrameBase const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Test {
    struct Foo : xx::Object {

        typedef Foo ThisType;
        typedef xx::Object BaseType;
	    Foo() = default;
		Foo(Foo const&) = delete;
		Foo& operator=(Foo const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish {
    // 子弹基类
    struct Bullet : xx::Object {
        // 自增id
        int32_t id = 0;
        // 位于容器时的下标 ( 用于快速交换删除 )
        int32_t indexAtContainer = 0;
        // 中心点坐标
        ::xx::Pos pos;
        // 每帧的直线移动坐标增量( 60fps )
        ::xx::Pos moveInc;
        // 金币 / 倍率( 记录炮台开火时的 Bet 值 )
        int64_t coin = 0;

        typedef Bullet ThisType;
        typedef xx::Object BaseType;
	    Bullet() = default;
		Bullet(Bullet const&) = delete;
		Bullet& operator=(Bullet const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 子弹 & 鱼 & 武器 的基类
    struct MoveObject : xx::Object {
        // 自增id
        int32_t id = 0;
        // 位于容器时的下标 ( 用于快速交换删除 )
        int32_t indexAtContainer = 0;
        // 中心点坐标
        ::xx::Pos pos;

        typedef MoveObject ThisType;
        typedef xx::Object BaseType;
	    MoveObject() = default;
		MoveObject(MoveObject const&) = delete;
		MoveObject& operator=(MoveObject const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

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
        // 当 Client 通过 Lobby 服务到 Game 发 Enter 时, Game 需要生成一个 token 以便 Client Enter 时传入以校验身份
        std::string_s token;
        // 开炮等行为花掉的金币数汇总 ( 统计 )
        int64_t consumeCoin = 0;
        // 当前显示游戏币数( 不代表玩家总资产 ). 当玩家进入到游戏时, 该值填充 money * exchangeCoinRatio. 玩家退出时, 做除法还原为 money.
        int64_t coin = 0;
        // 所在场景
        std::weak_ptr<PKG::CatchFish::Scene> scene;
        // 座位
        PKG::CatchFish::Sits sit = (PKG::CatchFish::Sits)0;
        // 破产标识 ( 每帧开始检测一次是否破产, 是就标记之 )
        bool isBankRuptcy = false;
        // 开火锁定状态
        bool fireLocking = false;
        // 自动开火状态
        bool automating = false;
        // 开火锁定瞄准的鱼
        std::weak_ptr<PKG::CatchFish::Fish> aimFish;
        // 自增id ( 从 1 开始, 用于 子弹或炮台 id 填充 )
        int32_t autoIncId = 0;
        // 炮台集合 ( 常规炮 打到 钻头, 钻头飞向玩家变为 钻头炮, 覆盖在常规炮上 )
        xx::List_s<PKG::CatchFish::Cannon_s> cannons;

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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 炮台 ( 基类 )
    struct Cannon : xx::Object {
        // 炮倍率 ( 初始填充自 db. 范围限制为 Scene.minBet ~ maxBet )
        int64_t cannonPower = 0;
        // 炮管角度 ( 每次发射时都填充一下 )
        float cannonAngle = 0;
        // 子弹的自增流水号
        int32_t autoIncId = 0;
        // 所有子弹
        xx::List_s<PKG::CatchFish::Bullet_s> bullets;

        typedef Cannon ThisType;
        typedef xx::Object BaseType;
	    Cannon() = default;
		Cannon(Cannon const&) = delete;
		Cannon& operator=(Cannon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 场景
    struct Scene : xx::Object {
        // 场景基础配置参数 ( 主要来自 db )
        PKG::CatchFish::SceneConfig_s cfg;
        // 帧编号, 每帧 + 1. 用于同步
        int32_t frameNumber = 0;
        // 本地鱼生成专用随机数发生器
        ::xx::Random_s rnd;
        // 自增id ( 从 1 开始, 用于本地鱼生成 id 填充 )
        int32_t autoIncId = 0;
        // 自减id ( 从 -1 开始, 用于服务器下发鱼生成 id 填充 )
        int32_t autoDecId = 0;
        // 所有鱼 ( 乱序 )
        xx::List_s<PKG::CatchFish::Fish_s> fishss;
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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 场景基础配置参数 ( 主要来自 db )
    struct SceneConfig : xx::Object {
        // 游戏id
        int32_t gameId = 0;
        // 级别id
        int32_t levelId = 0;
        // 房间id
        int32_t roomId = 0;
        // 准入金
        double minMoney = 0;
        // 最低炮注( coin )
        int64_t minBet = 0;
        // 最高炮注( coin )
        int64_t maxBet = 0;
        // 进出游戏时 money 自动兑换成 coin 要 乘除 的系数
        int32_t exchangeCoinRatio = 0;
        // 子弹颗数限制 ( 分别针对每个炮台 )
        int64_t maxBulletsPerCannon = 0;

        typedef SceneConfig ThisType;
        typedef xx::Object BaseType;
	    SceneConfig() = default;
		SceneConfig(SceneConfig const&) = delete;
		SceneConfig& operator=(SceneConfig const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFishConfig {
    // 游戏配置信息( 配置信息并不会随着网络同步而下发, 反序列化后需要手工还原 )
    struct Config : xx::Object {
        // 所有鱼的配置信息
        xx::List_s<PKG::CatchFishConfig::Fish_s> fishs;
        // 所有炮台的配置信息
        xx::List_s<PKG::CatchFishConfig::Cannon_s> cannons;
        // 基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )
        xx::List_s<::xx::Pos> sitPositons;

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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    struct Bullet : xx::Object {
        // 配置id
        int32_t id = 0;
        // 开火火焰帧集合
        xx::List_s<PKG::CatchFishConfig::SpriteFrameBase_s> fireFrames;
        // 子弹移动帧集合
        xx::List_s<PKG::CatchFishConfig::SpriteFrameBase_s> moveFrames;
        // 子弹爆炸帧集合
        xx::List_s<PKG::CatchFishConfig::SpriteFrameBase_s> boomFrames;
        // 渔网帧集合
        xx::List_s<PKG::CatchFishConfig::SpriteFrameBase_s> fishingNetsFrames;

        typedef Bullet ThisType;
        typedef xx::Object BaseType;
	    Bullet() = default;
		Bullet(Bullet const&) = delete;
		Bullet& operator=(Bullet const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    struct Cannon : xx::Object {
        // 配置id
        int32_t id = 0;
        // 对应子弹的配置
        PKG::CatchFishConfig::Bullet_s bullet;

        typedef Cannon ThisType;
        typedef xx::Object BaseType;
	    Cannon() = default;
		Cannon(Cannon const&) = delete;
		Cannon& operator=(Cannon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 鱼配置信息基类 ( 部分 boss 可能具有更多 frame, state 等配置参数 )
    struct Fish : xx::Object {
        // 配置id
        int32_t id = 0;
        // 鱼名
        std::string_s name;
        // 金币 / 倍率 ( 最小值 )
        int64_t minCoin = 0;
        // 金币 / 倍率 ( 最大值 )
        int64_t maxCoin = 0;
        // 基于整个鱼的最大晃动范围的圆形碰撞检测半径( 粗判 )
        float maxDetectRadius = 0;
        // 鱼移动帧集合
        xx::List_s<PKG::CatchFishConfig::SpriteFrame_s> moveFrames;
        // 鱼死帧集合
        xx::List_s<PKG::CatchFishConfig::SpriteFrameBase_s> dieFrames;
        // 显示放大系数. 创建精灵时先设起. 后面不用反复改
        float scale = 0;
        // 屏幕显示 z 轴( 决定显示覆盖顺序 )
        int32_t zOrder = 0;
        // 点选优先级说明参数, 越大越优先
        int32_t hitRank = 0;
        // 影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.
        float shadowScale = 0;
        // 影子的偏移坐标
        ::xx::Pos shadowOffset;

        typedef Fish ThisType;
        typedef xx::Object BaseType;
	    Fish() = default;
		Fish(Fish const&) = delete;
		Fish& operator=(Fish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 帧动画
    struct SpriteFrame : PKG::CatchFishConfig::SpriteFrameBase {
        // 基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模 )
        xx::List_s<xx::List_s<::xx::Pos>> polygons;
        // 首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )
        ::xx::Pos lockPoint;
        // 锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )
        xx::List_s<::xx::Pos> lockPoints;
        // 这一帧切到下一帧后应该移动的距离( 受 Fish.speedScale 影响 )
        float distance = 0;

        typedef SpriteFrame ThisType;
        typedef PKG::CatchFishConfig::SpriteFrameBase BaseType;
	    SpriteFrame() = default;
		SpriteFrame(SpriteFrame const&) = delete;
		SpriteFrame& operator=(SpriteFrame const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Generic {
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
        void InitCascade() noexcept override;

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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Test {
    struct EnterSuccess : xx::Object {
        xx::List_s<PKG::Test::Player_s> players;
        PKG::Test::Scene_s scene;

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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    struct Fish : xx::Object {
        int32_t id = 0;

        typedef Fish ThisType;
        typedef xx::Object BaseType;
	    Fish() = default;
		Fish(Fish const&) = delete;
		Fish& operator=(Fish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    struct Scene : xx::Object {
        xx::List_s<PKG::Test::Fish_s> fishs;
        xx::List_s<std::weak_ptr<PKG::Test::Player>> players;

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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    struct Player : xx::Object {
        int32_t id = 0;
        std::weak_ptr<PKG::Test::Scene> owner;

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
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace CatchFish {
    // 鱼基类
    struct Fish : xx::Object {
        // 自增id
        int32_t id = 0;
        // 位于容器时的下标 ( 用于快速交换删除 )
        int32_t indexAtContainer = 0;
        // 中心点坐标
        ::xx::Pos pos;
        // 配置id( 用来还原配置 )
        int32_t cfgId = 0;
        // 移动速度系数 ( 默认为 1 )
        float speedScale = 0;
        // 碰撞 | 显示 体积系数 ( 默认为 1 )
        float sizeScale = 0;

        typedef Fish ThisType;
        typedef xx::Object BaseType;
	    Fish() = default;
		Fish(Fish const&) = delete;
		Fish& operator=(Fish const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 武器 ( 有一些特殊鱼死后会变做 某种武器的长相，并花一段世家飞向玩家炮台 )
    struct Weapon : xx::Object {

        typedef Weapon ThisType;
        typedef xx::Object BaseType;
	    Weapon() = default;
		Weapon(Weapon const&) = delete;
		Weapon& operator=(Weapon const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;
        uint16_t GetTypeId() const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        void InitCascade() noexcept override;

        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
}
namespace xx {
    template<> struct TypeId<PKG::Test::Foo> { static const uint16_t value = 3; };
    template<> struct TypeId<PKG::Test::Player> { static const uint16_t value = 4; };
    template<> struct TypeId<PKG::Test::Scene> { static const uint16_t value = 5; };
    template<> struct TypeId<xx::List<PKG::Test::Fish_s>> { static const uint16_t value = 6; };
    template<> struct TypeId<PKG::Test::Fish> { static const uint16_t value = 7; };
    template<> struct TypeId<xx::List<std::weak_ptr<PKG::Test::Player>>> { static const uint16_t value = 8; };
    template<> struct TypeId<PKG::Test::EnterSuccess> { static const uint16_t value = 9; };
    template<> struct TypeId<xx::List<PKG::Test::Player_s>> { static const uint16_t value = 10; };
    template<> struct TypeId<PKG::Generic::Success> { static const uint16_t value = 11; };
    template<> struct TypeId<PKG::Generic::Error> { static const uint16_t value = 12; };
    template<> struct TypeId<PKG::Generic::Ping> { static const uint16_t value = 13; };
    template<> struct TypeId<PKG::Generic::Pong> { static const uint16_t value = 14; };
    template<> struct TypeId<PKG::CatchFishConfig::SpriteFrameBase> { static const uint16_t value = 15; };
    template<> struct TypeId<PKG::CatchFishConfig::SpriteFrame> { static const uint16_t value = 16; };
    template<> struct TypeId<xx::List<xx::List_s<::xx::Pos>>> { static const uint16_t value = 17; };
    template<> struct TypeId<xx::List<::xx::Pos>> { static const uint16_t value = 18; };
    template<> struct TypeId<PKG::CatchFishConfig::Fish> { static const uint16_t value = 19; };
    template<> struct TypeId<xx::List<PKG::CatchFishConfig::SpriteFrame_s>> { static const uint16_t value = 20; };
    template<> struct TypeId<xx::List<PKG::CatchFishConfig::SpriteFrameBase_s>> { static const uint16_t value = 21; };
    template<> struct TypeId<PKG::CatchFishConfig::Cannon> { static const uint16_t value = 22; };
    template<> struct TypeId<PKG::CatchFishConfig::Bullet> { static const uint16_t value = 23; };
    template<> struct TypeId<PKG::CatchFishConfig::Config> { static const uint16_t value = 24; };
    template<> struct TypeId<xx::List<PKG::CatchFishConfig::Fish_s>> { static const uint16_t value = 25; };
    template<> struct TypeId<xx::List<PKG::CatchFishConfig::Cannon_s>> { static const uint16_t value = 26; };
    template<> struct TypeId<PKG::CatchFish::SceneConfig> { static const uint16_t value = 27; };
    template<> struct TypeId<PKG::CatchFish::Scene> { static const uint16_t value = 28; };
    template<> struct TypeId<::xx::Random> { static const uint16_t value = 29; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Fish_s>> { static const uint16_t value = 30; };
    template<> struct TypeId<PKG::CatchFish::Fish> { static const uint16_t value = 31; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Sits>> { static const uint16_t value = 32; };
    template<> struct TypeId<xx::List<std::weak_ptr<PKG::CatchFish::Player>>> { static const uint16_t value = 33; };
    template<> struct TypeId<PKG::CatchFish::Cannon> { static const uint16_t value = 34; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Bullet_s>> { static const uint16_t value = 35; };
    template<> struct TypeId<PKG::CatchFish::Bullet> { static const uint16_t value = 36; };
    template<> struct TypeId<PKG::CatchFish::Player> { static const uint16_t value = 37; };
    template<> struct TypeId<xx::List<PKG::CatchFish::Cannon_s>> { static const uint16_t value = 38; };
    template<> struct TypeId<PKG::CatchFish::MoveObject> { static const uint16_t value = 39; };
    template<> struct TypeId<PKG::CatchFish::Weapon> { static const uint16_t value = 40; };
}
namespace PKG {
namespace Test {
    inline uint16_t Foo::GetTypeId() const noexcept {
        return 3;
    }
    inline void Foo::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Foo::FromBBuffer(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline void Foo::InitCascade() noexcept {
    }
    inline void Foo::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Test.Foo\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Foo::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline uint16_t Player::GetTypeId() const noexcept {
        return 4;
    }
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->owner);
    }
    inline int Player::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->owner)) return r;
        return 0;
    }
    inline void Player::InitCascade() noexcept {
    }
    inline void Player::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Test.Player\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Player::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"owner\":", this->owner);
    }
    inline uint16_t Scene::GetTypeId() const noexcept {
        return 5;
    }
    inline void Scene::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->fishs);
        bb.Write(this->players);
    }
    inline int Scene::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishs)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        return 0;
    }
    inline void Scene::InitCascade() noexcept {
        if (this->fishs) {
            this->fishs->InitCascade();
        }
        if (this->players) {
            this->players->InitCascade();
        }
    }
    inline void Scene::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Test.Scene\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Scene::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"fishs\":", this->fishs);
        xx::Append(s, ", \"players\":", this->players);
    }
    inline uint16_t Fish::GetTypeId() const noexcept {
        return 7;
    }
    inline void Fish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
    }
    inline int Fish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        return 0;
    }
    inline void Fish::InitCascade() noexcept {
    }
    inline void Fish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Test.Fish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Fish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
    }
    inline uint16_t EnterSuccess::GetTypeId() const noexcept {
        return 9;
    }
    inline void EnterSuccess::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->players);
        bb.Write(this->scene);
    }
    inline int EnterSuccess::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        if (int r = bb.Read(this->scene)) return r;
        return 0;
    }
    inline void EnterSuccess::InitCascade() noexcept {
        if (this->players) {
            this->players->InitCascade();
        }
        if (this->scene) {
            this->scene->InitCascade();
        }
    }
    inline void EnterSuccess::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Test.EnterSuccess\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void EnterSuccess::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"players\":", this->players);
        xx::Append(s, ", \"scene\":", this->scene);
    }
}
namespace Generic {
    inline uint16_t Success::GetTypeId() const noexcept {
        return 11;
    }
    inline void Success::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Success::FromBBuffer(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline void Success::InitCascade() noexcept {
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
        return 12;
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
    inline void Error::InitCascade() noexcept {
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
        return 13;
    }
    inline void Ping::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->ticks);
    }
    inline int Ping::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->ticks)) return r;
        return 0;
    }
    inline void Ping::InitCascade() noexcept {
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
        return 14;
    }
    inline void Pong::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->ticks);
    }
    inline int Pong::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->ticks)) return r;
        return 0;
    }
    inline void Pong::InitCascade() noexcept {
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
namespace CatchFishConfig {
    inline uint16_t SpriteFrameBase::GetTypeId() const noexcept {
        return 15;
    }
    inline void SpriteFrameBase::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->textureName);
        bb.Write(this->frameName);
    }
    inline int SpriteFrameBase::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->textureName)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->frameName)) return r;
        return 0;
    }
    inline void SpriteFrameBase::InitCascade() noexcept {
    }
    inline void SpriteFrameBase::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFishConfig.SpriteFrameBase\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void SpriteFrameBase::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->textureName) xx::Append(s, ", \"textureName\":\"", this->textureName, "\"");
        else xx::Append(s, ", \"textureName\":nil");
        if (this->frameName) xx::Append(s, ", \"frameName\":\"", this->frameName, "\"");
        else xx::Append(s, ", \"frameName\":nil");
    }
    inline uint16_t SpriteFrame::GetTypeId() const noexcept {
        return 16;
    }
    inline void SpriteFrame::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->polygons);
        bb.Write(this->lockPoint);
        bb.Write(this->lockPoints);
        bb.Write(this->distance);
    }
    inline int SpriteFrame::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->polygons)) return r;
        if (int r = bb.Read(this->lockPoint)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->lockPoints)) return r;
        if (int r = bb.Read(this->distance)) return r;
        return 0;
    }
    inline void SpriteFrame::InitCascade() noexcept {
        this->BaseType::InitCascade();
        if (this->polygons) {
            this->polygons->InitCascade();
        }
        if (this->lockPoints) {
            this->lockPoints->InitCascade();
        }
    }
    inline void SpriteFrame::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFishConfig.SpriteFrame\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void SpriteFrame::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"polygons\":", this->polygons);
        xx::Append(s, ", \"lockPoint\":", this->lockPoint);
        xx::Append(s, ", \"lockPoints\":", this->lockPoints);
        xx::Append(s, ", \"distance\":", this->distance);
    }
    inline uint16_t Fish::GetTypeId() const noexcept {
        return 19;
    }
    inline void Fish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->name);
        bb.Write(this->minCoin);
        bb.Write(this->maxCoin);
        bb.Write(this->maxDetectRadius);
        bb.Write(this->moveFrames);
        bb.Write(this->dieFrames);
        bb.Write(this->scale);
        bb.Write(this->zOrder);
        bb.Write(this->hitRank);
        bb.Write(this->shadowScale);
        bb.Write(this->shadowOffset);
    }
    inline int Fish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->name)) return r;
        if (int r = bb.Read(this->minCoin)) return r;
        if (int r = bb.Read(this->maxCoin)) return r;
        if (int r = bb.Read(this->maxDetectRadius)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->moveFrames)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->dieFrames)) return r;
        if (int r = bb.Read(this->scale)) return r;
        if (int r = bb.Read(this->zOrder)) return r;
        if (int r = bb.Read(this->hitRank)) return r;
        if (int r = bb.Read(this->shadowScale)) return r;
        if (int r = bb.Read(this->shadowOffset)) return r;
        return 0;
    }
    inline void Fish::InitCascade() noexcept {
        if (this->moveFrames) {
            this->moveFrames->InitCascade();
        }
        if (this->dieFrames) {
            this->dieFrames->InitCascade();
        }
    }
    inline void Fish::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFishConfig.Fish\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Fish::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        if (this->name) xx::Append(s, ", \"name\":\"", this->name, "\"");
        else xx::Append(s, ", \"name\":nil");
        xx::Append(s, ", \"minCoin\":", this->minCoin);
        xx::Append(s, ", \"maxCoin\":", this->maxCoin);
        xx::Append(s, ", \"maxDetectRadius\":", this->maxDetectRadius);
        xx::Append(s, ", \"moveFrames\":", this->moveFrames);
        xx::Append(s, ", \"dieFrames\":", this->dieFrames);
        xx::Append(s, ", \"scale\":", this->scale);
        xx::Append(s, ", \"zOrder\":", this->zOrder);
        xx::Append(s, ", \"hitRank\":", this->hitRank);
        xx::Append(s, ", \"shadowScale\":", this->shadowScale);
        xx::Append(s, ", \"shadowOffset\":", this->shadowOffset);
    }
    inline uint16_t Cannon::GetTypeId() const noexcept {
        return 22;
    }
    inline void Cannon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->bullet);
    }
    inline int Cannon::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->bullet)) return r;
        return 0;
    }
    inline void Cannon::InitCascade() noexcept {
        if (this->bullet) {
            this->bullet->InitCascade();
        }
    }
    inline void Cannon::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFishConfig.Cannon\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Cannon::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"bullet\":", this->bullet);
    }
    inline uint16_t Bullet::GetTypeId() const noexcept {
        return 23;
    }
    inline void Bullet::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->fireFrames);
        bb.Write(this->moveFrames);
        bb.Write(this->boomFrames);
        bb.Write(this->fishingNetsFrames);
    }
    inline int Bullet::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fireFrames)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->moveFrames)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->boomFrames)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishingNetsFrames)) return r;
        return 0;
    }
    inline void Bullet::InitCascade() noexcept {
        if (this->fireFrames) {
            this->fireFrames->InitCascade();
        }
        if (this->moveFrames) {
            this->moveFrames->InitCascade();
        }
        if (this->boomFrames) {
            this->boomFrames->InitCascade();
        }
        if (this->fishingNetsFrames) {
            this->fishingNetsFrames->InitCascade();
        }
    }
    inline void Bullet::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFishConfig.Bullet\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Bullet::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"fireFrames\":", this->fireFrames);
        xx::Append(s, ", \"moveFrames\":", this->moveFrames);
        xx::Append(s, ", \"boomFrames\":", this->boomFrames);
        xx::Append(s, ", \"fishingNetsFrames\":", this->fishingNetsFrames);
    }
    inline uint16_t Config::GetTypeId() const noexcept {
        return 24;
    }
    inline void Config::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->fishs);
        bb.Write(this->cannons);
        bb.Write(this->sitPositons);
    }
    inline int Config::FromBBuffer(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishs)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->cannons)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->sitPositons)) return r;
        return 0;
    }
    inline void Config::InitCascade() noexcept {
        if (this->fishs) {
            this->fishs->InitCascade();
        }
        if (this->cannons) {
            this->cannons->InitCascade();
        }
        if (this->sitPositons) {
            this->sitPositons->InitCascade();
        }
    }
    inline void Config::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFishConfig.Config\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Config::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"fishs\":", this->fishs);
        xx::Append(s, ", \"cannons\":", this->cannons);
        xx::Append(s, ", \"sitPositons\":", this->sitPositons);
    }
}
namespace CatchFish {
    inline uint16_t SceneConfig::GetTypeId() const noexcept {
        return 27;
    }
    inline void SceneConfig::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->gameId);
        bb.Write(this->levelId);
        bb.Write(this->roomId);
        bb.Write(this->minMoney);
        bb.Write(this->minBet);
        bb.Write(this->maxBet);
        bb.Write(this->exchangeCoinRatio);
        bb.Write(this->maxBulletsPerCannon);
    }
    inline int SceneConfig::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->gameId)) return r;
        if (int r = bb.Read(this->levelId)) return r;
        if (int r = bb.Read(this->roomId)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        if (int r = bb.Read(this->minBet)) return r;
        if (int r = bb.Read(this->maxBet)) return r;
        if (int r = bb.Read(this->exchangeCoinRatio)) return r;
        if (int r = bb.Read(this->maxBulletsPerCannon)) return r;
        return 0;
    }
    inline void SceneConfig::InitCascade() noexcept {
    }
    inline void SceneConfig::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.SceneConfig\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void SceneConfig::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"gameId\":", this->gameId);
        xx::Append(s, ", \"levelId\":", this->levelId);
        xx::Append(s, ", \"roomId\":", this->roomId);
        xx::Append(s, ", \"minMoney\":", this->minMoney);
        xx::Append(s, ", \"minBet\":", this->minBet);
        xx::Append(s, ", \"maxBet\":", this->maxBet);
        xx::Append(s, ", \"exchangeCoinRatio\":", this->exchangeCoinRatio);
        xx::Append(s, ", \"maxBulletsPerCannon\":", this->maxBulletsPerCannon);
    }
    inline uint16_t Scene::GetTypeId() const noexcept {
        return 28;
    }
    inline void Scene::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->cfg);
        bb.Write(this->frameNumber);
        bb.Write(this->rnd);
        bb.Write(this->autoIncId);
        bb.Write(this->autoDecId);
        bb.Write(this->fishss);
        bb.Write(this->freeSits);
        bb.Write(this->players);
    }
    inline int Scene::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->cfg)) return r;
        if (int r = bb.Read(this->frameNumber)) return r;
        if (int r = bb.Read(this->rnd)) return r;
        if (int r = bb.Read(this->autoIncId)) return r;
        if (int r = bb.Read(this->autoDecId)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->fishss)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->freeSits)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        return 0;
    }
    inline void Scene::InitCascade() noexcept {
        if (this->cfg) {
            this->cfg->InitCascade();
        }
        if (this->rnd) {
            this->rnd->InitCascade();
        }
        if (this->fishss) {
            this->fishss->InitCascade();
        }
        if (this->freeSits) {
            this->freeSits->InitCascade();
        }
        if (this->players) {
            this->players->InitCascade();
        }
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
        xx::Append(s, ", \"cfg\":", this->cfg);
        xx::Append(s, ", \"frameNumber\":", this->frameNumber);
        xx::Append(s, ", \"rnd\":", this->rnd);
        xx::Append(s, ", \"autoIncId\":", this->autoIncId);
        xx::Append(s, ", \"autoDecId\":", this->autoDecId);
        xx::Append(s, ", \"fishss\":", this->fishss);
        xx::Append(s, ", \"freeSits\":", this->freeSits);
        xx::Append(s, ", \"players\":", this->players);
    }
    inline uint16_t Cannon::GetTypeId() const noexcept {
        return 34;
    }
    inline void Cannon::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->cannonPower);
        bb.Write(this->cannonAngle);
        bb.Write(this->autoIncId);
        bb.Write(this->bullets);
    }
    inline int Cannon::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->cannonPower)) return r;
        if (int r = bb.Read(this->cannonAngle)) return r;
        if (int r = bb.Read(this->autoIncId)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->bullets)) return r;
        return 0;
    }
    inline void Cannon::InitCascade() noexcept {
        if (this->bullets) {
            this->bullets->InitCascade();
        }
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
        xx::Append(s, ", \"cannonPower\":", this->cannonPower);
        xx::Append(s, ", \"cannonAngle\":", this->cannonAngle);
        xx::Append(s, ", \"autoIncId\":", this->autoIncId);
        xx::Append(s, ", \"bullets\":", this->bullets);
    }
    inline uint16_t Player::GetTypeId() const noexcept {
        return 37;
    }
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->nickname);
        bb.Write(this->avatar_id);
        bb.Write(this->token);
        bb.Write(this->consumeCoin);
        bb.Write(this->coin);
        bb.Write(this->scene);
        bb.Write(this->sit);
        bb.Write(this->isBankRuptcy);
        bb.Write(this->fireLocking);
        bb.Write(this->automating);
        bb.Write(this->aimFish);
        bb.Write(this->autoIncId);
        bb.Write(this->cannons);
    }
    inline int Player::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->nickname)) return r;
        if (int r = bb.Read(this->avatar_id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->token)) return r;
        if (int r = bb.Read(this->consumeCoin)) return r;
        if (int r = bb.Read(this->coin)) return r;
        if (int r = bb.Read(this->scene)) return r;
        if (int r = bb.Read(this->sit)) return r;
        if (int r = bb.Read(this->isBankRuptcy)) return r;
        if (int r = bb.Read(this->fireLocking)) return r;
        if (int r = bb.Read(this->automating)) return r;
        if (int r = bb.Read(this->aimFish)) return r;
        if (int r = bb.Read(this->autoIncId)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->cannons)) return r;
        return 0;
    }
    inline void Player::InitCascade() noexcept {
        if (this->cannons) {
            this->cannons->InitCascade();
        }
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
        if (this->token) xx::Append(s, ", \"token\":\"", this->token, "\"");
        else xx::Append(s, ", \"token\":nil");
        xx::Append(s, ", \"consumeCoin\":", this->consumeCoin);
        xx::Append(s, ", \"coin\":", this->coin);
        xx::Append(s, ", \"scene\":", this->scene);
        xx::Append(s, ", \"sit\":", this->sit);
        xx::Append(s, ", \"isBankRuptcy\":", this->isBankRuptcy);
        xx::Append(s, ", \"fireLocking\":", this->fireLocking);
        xx::Append(s, ", \"automating\":", this->automating);
        xx::Append(s, ", \"aimFish\":", this->aimFish);
        xx::Append(s, ", \"autoIncId\":", this->autoIncId);
        xx::Append(s, ", \"cannons\":", this->cannons);
    }
    inline uint16_t MoveObject::GetTypeId() const noexcept {
        return 39;
    }
    inline void MoveObject::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->indexAtContainer);
        bb.Write(this->pos);
    }
    inline int MoveObject::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->indexAtContainer)) return r;
        if (int r = bb.Read(this->pos)) return r;
        return 0;
    }
    inline void MoveObject::InitCascade() noexcept {
    }
    inline void MoveObject::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"CatchFish.MoveObject\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void MoveObject::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"indexAtContainer\":", this->indexAtContainer);
        xx::Append(s, ", \"pos\":", this->pos);
    }
    inline uint16_t Bullet::GetTypeId() const noexcept {
        return 36;
    }
    inline void Bullet::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->indexAtContainer);
        bb.Write(this->pos);
        bb.Write(this->moveInc);
        bb.Write(this->coin);
    }
    inline int Bullet::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->indexAtContainer)) return r;
        if (int r = bb.Read(this->pos)) return r;
        if (int r = bb.Read(this->moveInc)) return r;
        if (int r = bb.Read(this->coin)) return r;
        return 0;
    }
    inline void Bullet::InitCascade() noexcept {
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
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"indexAtContainer\":", this->indexAtContainer);
        xx::Append(s, ", \"pos\":", this->pos);
        xx::Append(s, ", \"moveInc\":", this->moveInc);
        xx::Append(s, ", \"coin\":", this->coin);
    }
    inline uint16_t Fish::GetTypeId() const noexcept {
        return 31;
    }
    inline void Fish::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->indexAtContainer);
        bb.Write(this->pos);
        bb.Write(this->cfgId);
        bb.Write(this->speedScale);
        bb.Write(this->sizeScale);
    }
    inline int Fish::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->indexAtContainer)) return r;
        if (int r = bb.Read(this->pos)) return r;
        if (int r = bb.Read(this->cfgId)) return r;
        if (int r = bb.Read(this->speedScale)) return r;
        if (int r = bb.Read(this->sizeScale)) return r;
        return 0;
    }
    inline void Fish::InitCascade() noexcept {
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
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"indexAtContainer\":", this->indexAtContainer);
        xx::Append(s, ", \"pos\":", this->pos);
        xx::Append(s, ", \"cfgId\":", this->cfgId);
        xx::Append(s, ", \"speedScale\":", this->speedScale);
        xx::Append(s, ", \"sizeScale\":", this->sizeScale);
    }
    inline uint16_t Weapon::GetTypeId() const noexcept {
        return 40;
    }
    inline void Weapon::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Weapon::FromBBuffer(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline void Weapon::InitCascade() noexcept {
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
    }
}
}
namespace PKG {
	struct AllTypesRegister {
        AllTypesRegister() {
	        xx::BBuffer::Register<PKG::Test::Foo>(3);
	        xx::BBuffer::Register<PKG::Test::Player>(4);
	        xx::BBuffer::Register<PKG::Test::Scene>(5);
	        xx::BBuffer::Register<xx::List<PKG::Test::Fish_s>>(6);
	        xx::BBuffer::Register<PKG::Test::Fish>(7);
	        xx::BBuffer::Register<xx::List<std::weak_ptr<PKG::Test::Player>>>(8);
	        xx::BBuffer::Register<PKG::Test::EnterSuccess>(9);
	        xx::BBuffer::Register<xx::List<PKG::Test::Player_s>>(10);
	        xx::BBuffer::Register<PKG::Generic::Success>(11);
	        xx::BBuffer::Register<PKG::Generic::Error>(12);
	        xx::BBuffer::Register<PKG::Generic::Ping>(13);
	        xx::BBuffer::Register<PKG::Generic::Pong>(14);
	        xx::BBuffer::Register<PKG::CatchFishConfig::SpriteFrameBase>(15);
	        xx::BBuffer::Register<PKG::CatchFishConfig::SpriteFrame>(16);
	        xx::BBuffer::Register<xx::List<xx::List_s<::xx::Pos>>>(17);
	        xx::BBuffer::Register<xx::List<::xx::Pos>>(18);
	        xx::BBuffer::Register<PKG::CatchFishConfig::Fish>(19);
	        xx::BBuffer::Register<xx::List<PKG::CatchFishConfig::SpriteFrame_s>>(20);
	        xx::BBuffer::Register<xx::List<PKG::CatchFishConfig::SpriteFrameBase_s>>(21);
	        xx::BBuffer::Register<PKG::CatchFishConfig::Cannon>(22);
	        xx::BBuffer::Register<PKG::CatchFishConfig::Bullet>(23);
	        xx::BBuffer::Register<PKG::CatchFishConfig::Config>(24);
	        xx::BBuffer::Register<xx::List<PKG::CatchFishConfig::Fish_s>>(25);
	        xx::BBuffer::Register<xx::List<PKG::CatchFishConfig::Cannon_s>>(26);
	        xx::BBuffer::Register<PKG::CatchFish::SceneConfig>(27);
	        xx::BBuffer::Register<PKG::CatchFish::Scene>(28);
	        xx::BBuffer::Register<::xx::Random>(29);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Fish_s>>(30);
	        xx::BBuffer::Register<PKG::CatchFish::Fish>(31);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Sits>>(32);
	        xx::BBuffer::Register<xx::List<std::weak_ptr<PKG::CatchFish::Player>>>(33);
	        xx::BBuffer::Register<PKG::CatchFish::Cannon>(34);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Bullet_s>>(35);
	        xx::BBuffer::Register<PKG::CatchFish::Bullet>(36);
	        xx::BBuffer::Register<PKG::CatchFish::Player>(37);
	        xx::BBuffer::Register<xx::List<PKG::CatchFish::Cannon_s>>(38);
	        xx::BBuffer::Register<PKG::CatchFish::MoveObject>(39);
	        xx::BBuffer::Register<PKG::CatchFish::Weapon>(40);
        }
	};
	inline AllTypesRegister AllTypesRegisterInstance;   // for auto register at program startup
}
