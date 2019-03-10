#pragma once
#include "xx_list.h"

namespace PKG {
	struct PkgGenMd5 {
		inline static const std::string value = "29d1a8b88b58ab22d8325d53f9c143b2";
    };

    // 操作成功( 默认 response 结果 )
    struct Success;
    using Success_s = std::shared_ptr<Success>;
    using Success_w = std::weak_ptr<Success>;

    // 出错( 通用 response 结果 )
    struct Error;
    using Error_s = std::shared_ptr<Error>;
    using Error_w = std::weak_ptr<Error>;

    // 服务连接信息
    struct ConnInfo;
    using ConnInfo_s = std::shared_ptr<ConnInfo>;
    using ConnInfo_w = std::weak_ptr<ConnInfo>;

    // 并非一般的数据包. 仅用于声明各式 List<T>
    struct Collections;
    using Collections_s = std::shared_ptr<Collections>;
    using Collections_w = std::weak_ptr<Collections>;

namespace Client_Login {
    // 校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T
    struct Auth;
    using Auth_s = std::shared_ptr<Auth>;
    using Auth_w = std::weak_ptr<Auth>;

}
namespace Server {
    // 服务间互表身份的首包
    struct Info;
    using Info_s = std::shared_ptr<Info>;
    using Info_w = std::weak_ptr<Info>;

}
namespace Client_Lobby {
    // 首包. 进入大厅. 成功返回 Self( 含 Root 以及个人信息 ). 如果已经位于具体游戏中, 返回 ConnInfo. 失败立即被 T
    struct Enter;
    using Enter_s = std::shared_ptr<Enter>;
    using Enter_w = std::weak_ptr<Enter>;

    // 进入 Game1, 位于 Root 时可发送, 返回 Game1. 失败立即被 T
    struct Enter_Game1;
    using Enter_Game1_s = std::shared_ptr<Enter_Game1>;
    using Enter_Game1_w = std::weak_ptr<Enter_Game1>;

    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    struct Enter_Game1_Level;
    using Enter_Game1_Level_s = std::shared_ptr<Enter_Game1_Level>;
    using Enter_Game1_Level_w = std::weak_ptr<Enter_Game1_Level>;

    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    struct Enter_Game1_Level_Desk;
    using Enter_Game1_Level_Desk_s = std::shared_ptr<Enter_Game1_Level_Desk>;
    using Enter_Game1_Level_Desk_w = std::weak_ptr<Enter_Game1_Level_Desk>;

    // 退回上一层. 失败立即被 T
    struct Back;
    using Back_s = std::shared_ptr<Back>;
    using Back_w = std::weak_ptr<Back>;

}
namespace Lobby_Client {
    // 玩家自己的数据
    struct Self;
    using Self_s = std::shared_ptr<Self>;
    using Self_w = std::weak_ptr<Self>;

    // 其他玩家的数据
    struct Player;
    using Player_s = std::shared_ptr<Player>;
    using Player_w = std::weak_ptr<Player>;

    // 大厅根部
    struct Root;
    using Root_s = std::shared_ptr<Root>;
    using Root_w = std::weak_ptr<Root>;

    // Game 特化: Game1 具体配置信息
    struct Game1;
    using Game1_s = std::shared_ptr<Game1>;
    using Game1_w = std::weak_ptr<Game1>;

    // Game1 级别的详细数据
    struct Game1_Level_Info;
    using Game1_Level_Info_s = std::shared_ptr<Game1_Level_Info>;
    using Game1_Level_Info_w = std::weak_ptr<Game1_Level_Info>;

    // Game1 级别的详细数据
    struct Game1_Level;
    using Game1_Level_s = std::shared_ptr<Game1_Level>;
    using Game1_Level_w = std::weak_ptr<Game1_Level>;

    // Game1 级别 下的 桌子 的详细数据
    struct Game1_Level_Desk;
    using Game1_Level_Desk_s = std::shared_ptr<Game1_Level_Desk>;
    using Game1_Level_Desk_w = std::weak_ptr<Game1_Level_Desk>;

}
namespace Lobby {
    // 玩家明细
    struct Player;
    using Player_s = std::shared_ptr<Player>;
    using Player_w = std::weak_ptr<Player>;

    // 玩家容器基类
    struct Place;
    using Place_s = std::shared_ptr<Place>;
    using Place_w = std::weak_ptr<Place>;

    // 大厅根部
    struct Root;
    using Root_s = std::shared_ptr<Root>;
    using Root_w = std::weak_ptr<Root>;

    // 游戏基类
    struct Game;
    using Game_s = std::shared_ptr<Game>;
    using Game_w = std::weak_ptr<Game>;

    // Game 特化: Game1 具体配置信息
    struct Game1;
    using Game1_s = std::shared_ptr<Game1>;
    using Game1_w = std::weak_ptr<Game1>;

    // Game1 级别的详细数据
    struct Game1_Level;
    using Game1_Level_s = std::shared_ptr<Game1_Level>;
    using Game1_Level_w = std::weak_ptr<Game1_Level>;

    // Game1 级别 下的 桌子 的详细数据
    struct Game1_Level_Desk;
    using Game1_Level_Desk_s = std::shared_ptr<Game1_Level_Desk>;
    using Game1_Level_Desk_w = std::weak_ptr<Game1_Level_Desk>;

}
namespace Login_DB {
    // 根据用户名获取用户信息. 找到就返回 DB.Account. 找不到就返回 Error
    struct GetAccount;
    using GetAccount_s = std::shared_ptr<GetAccount>;
    using GetAccount_w = std::weak_ptr<GetAccount>;

}
namespace DB {
    struct Account;
    using Account_s = std::shared_ptr<Account>;
    using Account_w = std::weak_ptr<Account>;

}
namespace Server {
    enum class Types : int32_t {
        Unknown = 0,
        Login = 1,
        Lobby = 2,
        DB = 3,
        MAX = 4,
    };
}
namespace Lobby {
    // 玩家容器基类
    struct Place : xx::Object {
        // 指向上层容器( Root 没有上层容器 )
        PKG::Lobby::Place_s parent;
        // 玩家容器基类
        xx::List_s<PKG::Lobby::Player_s> players;

        typedef Place ThisType;
        typedef xx::Object BaseType;
	    Place() = default;
		Place(Place const&) = delete;
		Place& operator=(Place const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Place> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 游戏基类
    struct Game : PKG::Lobby::Place {
        // 游戏id
        int32_t id = 0;

        typedef Game ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Game() = default;
		Game(Game const&) = delete;
		Game& operator=(Game const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
    // 操作成功( 默认 response 结果 )
    struct Success : xx::Object {

        typedef Success ThisType;
        typedef xx::Object BaseType;
	    Success() = default;
		Success(Success const&) = delete;
		Success& operator=(Success const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Success> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
namespace Lobby {
    // Game1 级别 下的 桌子 的详细数据
    struct Game1_Level_Desk : PKG::Lobby::Place {
        // 桌子编号
        int32_t id = 0;

        typedef Game1_Level_Desk ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Game1_Level_Desk() = default;
		Game1_Level_Desk(Game1_Level_Desk const&) = delete;
		Game1_Level_Desk& operator=(Game1_Level_Desk const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game1_Level_Desk> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // Game1 级别的详细数据
    struct Game1_Level : PKG::Lobby::Place {
        // 级别编号
        int32_t id = 0;
        // 准入门槛
        double minMoney = 0;
        // 该级别下所有桌子列表
        xx::List_s<PKG::Lobby::Game1_Level_Desk_s> desks;

        typedef Game1_Level ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Game1_Level() = default;
		Game1_Level(Game1_Level const&) = delete;
		Game1_Level& operator=(Game1_Level const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game1_Level> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // Game 特化: Game1 具体配置信息
    struct Game1 : PKG::Lobby::Game {
        // Game1 级别列表
        xx::List_s<PKG::Lobby::Game1_Level_s> levels;

        typedef Game1 ThisType;
        typedef PKG::Lobby::Game BaseType;
	    Game1() = default;
		Game1(Game1 const&) = delete;
		Game1& operator=(Game1 const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game1> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 大厅根部
    struct Root : PKG::Lobby::Place {
        // 所有游戏列表
        xx::List_s<PKG::Lobby::Game_s> games;

        typedef Root ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Root() = default;
		Root(Root const&) = delete;
		Root& operator=(Root const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Root> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 玩家明细
    struct Player : xx::Object {
        // 玩家id
        int32_t id = 0;
        // 名字
        std::string_s username;
        // 有多少钱
        double money = 0;
        // 当前位置
        PKG::Lobby::Place_s place;
        // 位于 players 数组中的下标( 便于交换删除 )
        int32_t indexAtContainer = 0;
        // 特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息
        int32_t game1_Level_Desk_SeatIndex = 0;

        typedef Player ThisType;
        typedef xx::Object BaseType;
	    Player() = default;
		Player(Player const&) = delete;
		Player& operator=(Player const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Player> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Lobby_Client {
    // Game1 级别 下的 桌子 的详细数据
    struct Game1_Level_Desk : xx::Object {
        // 桌子编号
        int32_t id = 0;
        // 玩家列表
        xx::List_s<PKG::Lobby_Client::Player_s> players;

        typedef Game1_Level_Desk ThisType;
        typedef xx::Object BaseType;
	    Game1_Level_Desk() = default;
		Game1_Level_Desk(Game1_Level_Desk const&) = delete;
		Game1_Level_Desk& operator=(Game1_Level_Desk const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game1_Level_Desk> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // Game1 级别的详细数据
    struct Game1_Level : xx::Object {
        // 级别编号
        int32_t id = 0;
        // 准入门槛
        double minMoney = 0;
        // 该级别下所有桌子列表
        xx::List_s<PKG::Lobby_Client::Game1_Level_Desk_s> desks;

        typedef Game1_Level ThisType;
        typedef xx::Object BaseType;
	    Game1_Level() = default;
		Game1_Level(Game1_Level const&) = delete;
		Game1_Level& operator=(Game1_Level const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game1_Level> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // Game1 级别的详细数据
    struct Game1_Level_Info : xx::Object {
        // 级别编号
        int32_t id = 0;
        // 准入门槛
        double minMoney = 0;

        typedef Game1_Level_Info ThisType;
        typedef xx::Object BaseType;
	    Game1_Level_Info() = default;
		Game1_Level_Info(Game1_Level_Info const&) = delete;
		Game1_Level_Info& operator=(Game1_Level_Info const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game1_Level_Info> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // Game 特化: Game1 具体配置信息
    struct Game1 : xx::Object {
        // Game1 级别列表
        xx::List_s<PKG::Lobby_Client::Game1_Level_Info_s> levels;

        typedef Game1 ThisType;
        typedef xx::Object BaseType;
	    Game1() = default;
		Game1(Game1 const&) = delete;
		Game1& operator=(Game1 const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Game1> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Login_DB {
    // 根据用户名获取用户信息. 找到就返回 DB.Account. 找不到就返回 Error
    struct GetAccount : xx::Object {
        std::string_s username;

        typedef GetAccount ThisType;
        typedef xx::Object BaseType;
	    GetAccount() = default;
		GetAccount(GetAccount const&) = delete;
		GetAccount& operator=(GetAccount const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<GetAccount> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Lobby_Client {
    // 大厅根部
    struct Root : xx::Object {
        // 当前玩家可见的游戏列表
        xx::List_s<int32_t> gameIds;
        // 玩家自己的数据
        PKG::Lobby_Client::Self_s self;

        typedef Root ThisType;
        typedef xx::Object BaseType;
	    Root() = default;
		Root(Root const&) = delete;
		Root& operator=(Root const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Root> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 玩家自己的数据
    struct Self : xx::Object {
        // 玩家id
        int32_t id = 0;
        // 有多少钱
        double money = 0;

        typedef Self ThisType;
        typedef xx::Object BaseType;
	    Self() = default;
		Self(Self const&) = delete;
		Self& operator=(Self const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Self> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Client_Lobby {
    // 退回上一层. 失败立即被 T
    struct Back : xx::Object {

        typedef Back ThisType;
        typedef xx::Object BaseType;
	    Back() = default;
		Back(Back const&) = delete;
		Back& operator=(Back const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Back> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    struct Enter_Game1_Level_Desk : xx::Object {
        // 指定 Desk id
        int32_t id = 0;
        // 指定座次
        int32_t seatIndex = 0;

        typedef Enter_Game1_Level_Desk ThisType;
        typedef xx::Object BaseType;
	    Enter_Game1_Level_Desk() = default;
		Enter_Game1_Level_Desk(Enter_Game1_Level_Desk const&) = delete;
		Enter_Game1_Level_Desk& operator=(Enter_Game1_Level_Desk const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Enter_Game1_Level_Desk> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    struct Enter_Game1_Level : xx::Object {
        // 指定 Level id
        int32_t id = 0;

        typedef Enter_Game1_Level ThisType;
        typedef xx::Object BaseType;
	    Enter_Game1_Level() = default;
		Enter_Game1_Level(Enter_Game1_Level const&) = delete;
		Enter_Game1_Level& operator=(Enter_Game1_Level const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Enter_Game1_Level> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 进入 Game1, 位于 Root 时可发送, 返回 Game1. 失败立即被 T
    struct Enter_Game1 : xx::Object {

        typedef Enter_Game1 ThisType;
        typedef xx::Object BaseType;
	    Enter_Game1() = default;
		Enter_Game1(Enter_Game1 const&) = delete;
		Enter_Game1& operator=(Enter_Game1 const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Enter_Game1> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 首包. 进入大厅. 成功返回 Self( 含 Root 以及个人信息 ). 如果已经位于具体游戏中, 返回 ConnInfo. 失败立即被 T
    struct Enter : xx::Object {
        std::string_s token;

        typedef Enter ThisType;
        typedef xx::Object BaseType;
	    Enter() = default;
		Enter(Enter const&) = delete;
		Enter& operator=(Enter const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Enter> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Server {
    // 服务间互表身份的首包
    struct Info : xx::Object {
        PKG::Server::Types type = (PKG::Server::Types)0;

        typedef Info ThisType;
        typedef xx::Object BaseType;
	    Info() = default;
		Info(Info const&) = delete;
		Info& operator=(Info const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Info> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace Client_Login {
    // 校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T
    struct Auth : xx::Object {
        // 包版本校验
        std::string_s pkgMD5;
        // 用户名
        std::string_s username;
        // 密码
        std::string_s password;

        typedef Auth ThisType;
        typedef xx::Object BaseType;
	    Auth() = default;
		Auth(Auth const&) = delete;
		Auth& operator=(Auth const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Auth> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
    // 并非一般的数据包. 仅用于声明各式 List<T>
    struct Collections : xx::Object {
        xx::List_s<int32_t> ints;
        xx::List_s<int64_t> longs;
        xx::List_s<std::string_s> strings;
        xx::List_s<xx::Object_s> objects;

        typedef Collections ThisType;
        typedef xx::Object BaseType;
	    Collections() = default;
		Collections(Collections const&) = delete;
		Collections& operator=(Collections const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Collections> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 服务连接信息
    struct ConnInfo : xx::Object {
        // 服务类型
        PKG::Server::Types type = (PKG::Server::Types)0;
        // ipv4/6 地址
        std::string_s ip;
        // 端口
        int32_t port = 0;
        // 令牌
        std::string_s token;

        typedef ConnInfo ThisType;
        typedef xx::Object BaseType;
	    ConnInfo() = default;
		ConnInfo(ConnInfo const&) = delete;
		ConnInfo& operator=(ConnInfo const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<ConnInfo> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
    // 出错( 通用 response 结果 )
    struct Error : xx::Object {
        int32_t id = 0;
        std::string_s txt;

        typedef Error ThisType;
        typedef xx::Object BaseType;
	    Error() = default;
		Error(Error const&) = delete;
		Error& operator=(Error const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Error> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
namespace Lobby_Client {
    // 其他玩家的数据
    struct Player : xx::Object {
        // 玩家id
        int32_t id = 0;
        // 名字
        std::string_s username;
        // 特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息
        int32_t game1_Level_Desk_SeatIndex = 0;

        typedef Player ThisType;
        typedef xx::Object BaseType;
	    Player() = default;
		Player(Player const&) = delete;
		Player& operator=(Player const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Player> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
namespace DB {
    struct Account : xx::Object {
        int32_t id = 0;
        std::string_s username;
        std::string_s password;

        typedef Account ThisType;
        typedef xx::Object BaseType;
	    Account() = default;
		Account(Account const&) = delete;
		Account& operator=(Account const&) = delete;

        void ToString(std::string& s) const noexcept override;
        void ToStringCore(std::string& s) const noexcept override;

        virtual uint16_t GetTypeId() const noexcept;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;

        static std::shared_ptr<Account> MakeShared() noexcept;
        inline static std::shared_ptr<ThisType> defaultInstance;
    };
}
}
namespace xx {
    template<> struct TypeId<PKG::Success> { static const uint16_t value = 3; };
    template<> struct TypeId<PKG::Error> { static const uint16_t value = 4; };
    template<> struct TypeId<PKG::ConnInfo> { static const uint16_t value = 6; };
    template<> struct TypeId<PKG::Collections> { static const uint16_t value = 7; };
    template<> struct TypeId<xx::List<int32_t>> { static const uint16_t value = 8; };
    template<> struct TypeId<xx::List<int64_t>> { static const uint16_t value = 9; };
    template<> struct TypeId<xx::List<std::string_s>> { static const uint16_t value = 10; };
    template<> struct TypeId<xx::List<xx::Object_s>> { static const uint16_t value = 11; };
    template<> struct TypeId<PKG::Client_Login::Auth> { static const uint16_t value = 12; };
    template<> struct TypeId<PKG::Server::Info> { static const uint16_t value = 5; };
    template<> struct TypeId<PKG::Client_Lobby::Enter> { static const uint16_t value = 13; };
    template<> struct TypeId<PKG::Client_Lobby::Enter_Game1> { static const uint16_t value = 14; };
    template<> struct TypeId<PKG::Client_Lobby::Enter_Game1_Level> { static const uint16_t value = 15; };
    template<> struct TypeId<PKG::Client_Lobby::Enter_Game1_Level_Desk> { static const uint16_t value = 16; };
    template<> struct TypeId<PKG::Client_Lobby::Back> { static const uint16_t value = 17; };
    template<> struct TypeId<PKG::Lobby_Client::Self> { static const uint16_t value = 18; };
    template<> struct TypeId<PKG::Lobby_Client::Player> { static const uint16_t value = 19; };
    template<> struct TypeId<PKG::Lobby_Client::Root> { static const uint16_t value = 20; };
    template<> struct TypeId<PKG::Lobby_Client::Game1> { static const uint16_t value = 21; };
    template<> struct TypeId<xx::List<PKG::Lobby_Client::Game1_Level_Info_s>> { static const uint16_t value = 22; };
    template<> struct TypeId<PKG::Lobby_Client::Game1_Level_Info> { static const uint16_t value = 23; };
    template<> struct TypeId<PKG::Lobby_Client::Game1_Level> { static const uint16_t value = 24; };
    template<> struct TypeId<xx::List<PKG::Lobby_Client::Game1_Level_Desk_s>> { static const uint16_t value = 25; };
    template<> struct TypeId<PKG::Lobby_Client::Game1_Level_Desk> { static const uint16_t value = 26; };
    template<> struct TypeId<xx::List<PKG::Lobby_Client::Player_s>> { static const uint16_t value = 27; };
    template<> struct TypeId<PKG::Lobby::Player> { static const uint16_t value = 28; };
    template<> struct TypeId<PKG::Lobby::Place> { static const uint16_t value = 29; };
    template<> struct TypeId<xx::List<PKG::Lobby::Player_s>> { static const uint16_t value = 30; };
    template<> struct TypeId<PKG::Lobby::Root> { static const uint16_t value = 31; };
    template<> struct TypeId<xx::List<PKG::Lobby::Game_s>> { static const uint16_t value = 32; };
    template<> struct TypeId<PKG::Lobby::Game> { static const uint16_t value = 33; };
    template<> struct TypeId<PKG::Lobby::Game1> { static const uint16_t value = 34; };
    template<> struct TypeId<xx::List<PKG::Lobby::Game1_Level_s>> { static const uint16_t value = 35; };
    template<> struct TypeId<PKG::Lobby::Game1_Level> { static const uint16_t value = 36; };
    template<> struct TypeId<xx::List<PKG::Lobby::Game1_Level_Desk_s>> { static const uint16_t value = 37; };
    template<> struct TypeId<PKG::Lobby::Game1_Level_Desk> { static const uint16_t value = 38; };
    template<> struct TypeId<PKG::Login_DB::GetAccount> { static const uint16_t value = 39; };
    template<> struct TypeId<PKG::DB::Account> { static const uint16_t value = 40; };
}
namespace PKG {
    inline uint16_t Success::GetTypeId() const noexcept {
        return 3;
    }
    inline void Success::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Success::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Success::FromBBufferCore(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline void Success::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Success\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Success::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline std::shared_ptr<Success> Success::MakeShared() noexcept {
        return std::make_shared<Success>();
    }
    inline uint16_t Error::GetTypeId() const noexcept {
        return 4;
    }
    inline void Error::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->txt);
    }
    inline int Error::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Error::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->txt)) return r;
        return 0;
    }
    inline void Error::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Error\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Error::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        if (this->txt) xx::Append(s, ", \"txt\":\"", this->txt, "\"");
        else xx::Append(s, ", \"txt\":nil");
    }
    inline std::shared_ptr<Error> Error::MakeShared() noexcept {
        return std::make_shared<Error>();
    }
    inline uint16_t ConnInfo::GetTypeId() const noexcept {
        return 6;
    }
    inline void ConnInfo::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->type);
        bb.Write(this->ip);
        bb.Write(this->port);
        bb.Write(this->token);
    }
    inline int ConnInfo::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int ConnInfo::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->type)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->ip)) return r;
        if (int r = bb.Read(this->port)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->token)) return r;
        return 0;
    }
    inline void ConnInfo::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"ConnInfo\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void ConnInfo::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"type\":", this->type);
        if (this->ip) xx::Append(s, ", \"ip\":\"", this->ip, "\"");
        else xx::Append(s, ", \"ip\":nil");
        xx::Append(s, ", \"port\":", this->port);
        if (this->token) xx::Append(s, ", \"token\":\"", this->token, "\"");
        else xx::Append(s, ", \"token\":nil");
    }
    inline std::shared_ptr<ConnInfo> ConnInfo::MakeShared() noexcept {
        return std::make_shared<ConnInfo>();
    }
    inline uint16_t Collections::GetTypeId() const noexcept {
        return 7;
    }
    inline void Collections::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->ints);
        bb.Write(this->longs);
        bb.Write(this->strings);
        bb.Write(this->objects);
    }
    inline int Collections::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Collections::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->ints)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->longs)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->strings)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->objects)) return r;
        return 0;
    }
    inline void Collections::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Collections\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Collections::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"ints\":", this->ints);
        xx::Append(s, ", \"longs\":", this->longs);
        xx::Append(s, ", \"strings\":", this->strings);
        xx::Append(s, ", \"objects\":", this->objects);
    }
    inline std::shared_ptr<Collections> Collections::MakeShared() noexcept {
        return std::make_shared<Collections>();
    }
namespace Client_Login {
    inline uint16_t Auth::GetTypeId() const noexcept {
        return 12;
    }
    inline void Auth::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->pkgMD5);
        bb.Write(this->username);
        bb.Write(this->password);
    }
    inline int Auth::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Auth::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->pkgMD5)) return r;
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->username)) return r;
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->password)) return r;
        return 0;
    }
    inline void Auth::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_Login.Auth\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Auth::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->pkgMD5) xx::Append(s, ", \"pkgMD5\":\"", this->pkgMD5, "\"");
        else xx::Append(s, ", \"pkgMD5\":nil");
        if (this->username) xx::Append(s, ", \"username\":\"", this->username, "\"");
        else xx::Append(s, ", \"username\":nil");
        if (this->password) xx::Append(s, ", \"password\":\"", this->password, "\"");
        else xx::Append(s, ", \"password\":nil");
    }
    inline std::shared_ptr<Auth> Auth::MakeShared() noexcept {
        return std::make_shared<Auth>();
    }
}
namespace Server {
    inline uint16_t Info::GetTypeId() const noexcept {
        return 5;
    }
    inline void Info::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->type);
    }
    inline int Info::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Info::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->type)) return r;
        return 0;
    }
    inline void Info::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Server.Info\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Info::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"type\":", this->type);
    }
    inline std::shared_ptr<Info> Info::MakeShared() noexcept {
        return std::make_shared<Info>();
    }
}
namespace Client_Lobby {
    inline uint16_t Enter::GetTypeId() const noexcept {
        return 13;
    }
    inline void Enter::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->token);
    }
    inline int Enter::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Enter::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->token)) return r;
        return 0;
    }
    inline void Enter::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_Lobby.Enter\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Enter::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->token) xx::Append(s, ", \"token\":\"", this->token, "\"");
        else xx::Append(s, ", \"token\":nil");
    }
    inline std::shared_ptr<Enter> Enter::MakeShared() noexcept {
        return std::make_shared<Enter>();
    }
    inline uint16_t Enter_Game1::GetTypeId() const noexcept {
        return 14;
    }
    inline void Enter_Game1::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Enter_Game1::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Enter_Game1::FromBBufferCore(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline void Enter_Game1::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Enter_Game1::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline std::shared_ptr<Enter_Game1> Enter_Game1::MakeShared() noexcept {
        return std::make_shared<Enter_Game1>();
    }
    inline uint16_t Enter_Game1_Level::GetTypeId() const noexcept {
        return 15;
    }
    inline void Enter_Game1_Level::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
    }
    inline int Enter_Game1_Level::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Enter_Game1_Level::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        return 0;
    }
    inline void Enter_Game1_Level::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1_Level\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Enter_Game1_Level::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
    }
    inline std::shared_ptr<Enter_Game1_Level> Enter_Game1_Level::MakeShared() noexcept {
        return std::make_shared<Enter_Game1_Level>();
    }
    inline uint16_t Enter_Game1_Level_Desk::GetTypeId() const noexcept {
        return 16;
    }
    inline void Enter_Game1_Level_Desk::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->seatIndex);
    }
    inline int Enter_Game1_Level_Desk::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Enter_Game1_Level_Desk::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->seatIndex)) return r;
        return 0;
    }
    inline void Enter_Game1_Level_Desk::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1_Level_Desk\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Enter_Game1_Level_Desk::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"seatIndex\":", this->seatIndex);
    }
    inline std::shared_ptr<Enter_Game1_Level_Desk> Enter_Game1_Level_Desk::MakeShared() noexcept {
        return std::make_shared<Enter_Game1_Level_Desk>();
    }
    inline uint16_t Back::GetTypeId() const noexcept {
        return 17;
    }
    inline void Back::ToBBuffer(xx::BBuffer& bb) const noexcept {
    }
    inline int Back::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Back::FromBBufferCore(xx::BBuffer& bb) noexcept {
        return 0;
    }
    inline void Back::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Client_Lobby.Back\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Back::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
    }
    inline std::shared_ptr<Back> Back::MakeShared() noexcept {
        return std::make_shared<Back>();
    }
}
namespace Lobby_Client {
    inline uint16_t Self::GetTypeId() const noexcept {
        return 18;
    }
    inline void Self::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->money);
    }
    inline int Self::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Self::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->money)) return r;
        return 0;
    }
    inline void Self::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby_Client.Self\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Self::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"money\":", this->money);
    }
    inline std::shared_ptr<Self> Self::MakeShared() noexcept {
        return std::make_shared<Self>();
    }
    inline uint16_t Player::GetTypeId() const noexcept {
        return 19;
    }
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->username);
        bb.Write(this->game1_Level_Desk_SeatIndex);
    }
    inline int Player::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Player::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        if (int r = bb.Read(this->game1_Level_Desk_SeatIndex)) return r;
        return 0;
    }
    inline void Player::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby_Client.Player\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Player::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        if (this->username) xx::Append(s, ", \"username\":\"", this->username, "\"");
        else xx::Append(s, ", \"username\":nil");
        xx::Append(s, ", \"game1_Level_Desk_SeatIndex\":", this->game1_Level_Desk_SeatIndex);
    }
    inline std::shared_ptr<Player> Player::MakeShared() noexcept {
        return std::make_shared<Player>();
    }
    inline uint16_t Root::GetTypeId() const noexcept {
        return 20;
    }
    inline void Root::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->gameIds);
        bb.Write(this->self);
    }
    inline int Root::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Root::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->gameIds)) return r;
        if (int r = bb.Read(this->self)) return r;
        return 0;
    }
    inline void Root::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby_Client.Root\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Root::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"gameIds\":", this->gameIds);
        xx::Append(s, ", \"self\":", this->self);
    }
    inline std::shared_ptr<Root> Root::MakeShared() noexcept {
        return std::make_shared<Root>();
    }
    inline uint16_t Game1::GetTypeId() const noexcept {
        return 21;
    }
    inline void Game1::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->levels);
    }
    inline int Game1::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Game1::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->levels)) return r;
        return 0;
    }
    inline void Game1::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby_Client.Game1\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game1::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"levels\":", this->levels);
    }
    inline std::shared_ptr<Game1> Game1::MakeShared() noexcept {
        return std::make_shared<Game1>();
    }
    inline uint16_t Game1_Level_Info::GetTypeId() const noexcept {
        return 23;
    }
    inline void Game1_Level_Info::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->minMoney);
    }
    inline int Game1_Level_Info::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level_Info::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        return 0;
    }
    inline void Game1_Level_Info::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby_Client.Game1_Level_Info\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game1_Level_Info::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"minMoney\":", this->minMoney);
    }
    inline std::shared_ptr<Game1_Level_Info> Game1_Level_Info::MakeShared() noexcept {
        return std::make_shared<Game1_Level_Info>();
    }
    inline uint16_t Game1_Level::GetTypeId() const noexcept {
        return 24;
    }
    inline void Game1_Level::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->minMoney);
        bb.Write(this->desks);
    }
    inline int Game1_Level::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->desks)) return r;
        return 0;
    }
    inline void Game1_Level::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby_Client.Game1_Level\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game1_Level::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"minMoney\":", this->minMoney);
        xx::Append(s, ", \"desks\":", this->desks);
    }
    inline std::shared_ptr<Game1_Level> Game1_Level::MakeShared() noexcept {
        return std::make_shared<Game1_Level>();
    }
    inline uint16_t Game1_Level_Desk::GetTypeId() const noexcept {
        return 26;
    }
    inline void Game1_Level_Desk::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->players);
    }
    inline int Game1_Level_Desk::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level_Desk::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        return 0;
    }
    inline void Game1_Level_Desk::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby_Client.Game1_Level_Desk\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game1_Level_Desk::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"players\":", this->players);
    }
    inline std::shared_ptr<Game1_Level_Desk> Game1_Level_Desk::MakeShared() noexcept {
        return std::make_shared<Game1_Level_Desk>();
    }
}
namespace Lobby {
    inline uint16_t Player::GetTypeId() const noexcept {
        return 28;
    }
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->username);
        bb.Write(this->money);
        bb.Write(this->place);
        bb.Write(this->indexAtContainer);
        bb.Write(this->game1_Level_Desk_SeatIndex);
    }
    inline int Player::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Player::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        if (int r = bb.Read(this->money)) return r;
        if (int r = bb.Read(this->place)) return r;
        if (int r = bb.Read(this->indexAtContainer)) return r;
        if (int r = bb.Read(this->game1_Level_Desk_SeatIndex)) return r;
        return 0;
    }
    inline void Player::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby.Player\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Player::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        if (this->username) xx::Append(s, ", \"username\":\"", this->username, "\"");
        else xx::Append(s, ", \"username\":nil");
        xx::Append(s, ", \"money\":", this->money);
        xx::Append(s, ", \"place\":", this->place);
        xx::Append(s, ", \"indexAtContainer\":", this->indexAtContainer);
        xx::Append(s, ", \"game1_Level_Desk_SeatIndex\":", this->game1_Level_Desk_SeatIndex);
    }
    inline std::shared_ptr<Player> Player::MakeShared() noexcept {
        return std::make_shared<Player>();
    }
    inline uint16_t Place::GetTypeId() const noexcept {
        return 29;
    }
    inline void Place::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->parent);
        bb.Write(this->players);
    }
    inline int Place::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Place::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->parent)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        return 0;
    }
    inline void Place::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby.Place\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Place::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"parent\":", this->parent);
        xx::Append(s, ", \"players\":", this->players);
    }
    inline std::shared_ptr<Place> Place::MakeShared() noexcept {
        return std::make_shared<Place>();
    }
    inline uint16_t Root::GetTypeId() const noexcept {
        return 31;
    }
    inline void Root::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->games);
    }
    inline int Root::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Root::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->games)) return r;
        return 0;
    }
    inline void Root::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby.Root\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Root::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"games\":", this->games);
    }
    inline std::shared_ptr<Root> Root::MakeShared() noexcept {
        return std::make_shared<Root>();
    }
    inline uint16_t Game::GetTypeId() const noexcept {
        return 33;
    }
    inline void Game::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->id);
    }
    inline int Game::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        return 0;
    }
    inline void Game::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby.Game\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
    }
    inline std::shared_ptr<Game> Game::MakeShared() noexcept {
        return std::make_shared<Game>();
    }
    inline uint16_t Game1::GetTypeId() const noexcept {
        return 34;
    }
    inline void Game1::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->levels);
    }
    inline int Game1::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game1::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->levels)) return r;
        return 0;
    }
    inline void Game1::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby.Game1\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game1::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"levels\":", this->levels);
    }
    inline std::shared_ptr<Game1> Game1::MakeShared() noexcept {
        return std::make_shared<Game1>();
    }
    inline uint16_t Game1_Level::GetTypeId() const noexcept {
        return 36;
    }
    inline void Game1_Level::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->id);
        bb.Write(this->minMoney);
        bb.Write(this->desks);
    }
    inline int Game1_Level::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->desks)) return r;
        return 0;
    }
    inline void Game1_Level::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby.Game1_Level\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game1_Level::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        xx::Append(s, ", \"minMoney\":", this->minMoney);
        xx::Append(s, ", \"desks\":", this->desks);
    }
    inline std::shared_ptr<Game1_Level> Game1_Level::MakeShared() noexcept {
        return std::make_shared<Game1_Level>();
    }
    inline uint16_t Game1_Level_Desk::GetTypeId() const noexcept {
        return 38;
    }
    inline void Game1_Level_Desk::ToBBuffer(xx::BBuffer& bb) const noexcept {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->id);
    }
    inline int Game1_Level_Desk::FromBBuffer(xx::BBuffer& bb) noexcept {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level_Desk::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        return 0;
    }
    inline void Game1_Level_Desk::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Lobby.Game1_Level_Desk\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Game1_Level_Desk::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
    }
    inline std::shared_ptr<Game1_Level_Desk> Game1_Level_Desk::MakeShared() noexcept {
        return std::make_shared<Game1_Level_Desk>();
    }
}
namespace Login_DB {
    inline uint16_t GetAccount::GetTypeId() const noexcept {
        return 39;
    }
    inline void GetAccount::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->username);
    }
    inline int GetAccount::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int GetAccount::FromBBufferCore(xx::BBuffer& bb) noexcept {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        return 0;
    }
    inline void GetAccount::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"Login_DB.GetAccount\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void GetAccount::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        if (this->username) xx::Append(s, ", \"username\":\"", this->username, "\"");
        else xx::Append(s, ", \"username\":nil");
    }
    inline std::shared_ptr<GetAccount> GetAccount::MakeShared() noexcept {
        return std::make_shared<GetAccount>();
    }
}
namespace DB {
    inline uint16_t Account::GetTypeId() const noexcept {
        return 40;
    }
    inline void Account::ToBBuffer(xx::BBuffer& bb) const noexcept {
        bb.Write(this->id);
        bb.Write(this->username);
        bb.Write(this->password);
    }
    inline int Account::FromBBuffer(xx::BBuffer& bb) noexcept {
        return this->FromBBufferCore(bb);
    }
    inline int Account::FromBBufferCore(xx::BBuffer& bb) noexcept {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->password)) return r;
        return 0;
    }
    inline void Account::ToString(std::string& s) const noexcept {
        if (this->toStringFlag)
        {
        	xx::Append(s, "[ \"***** recursived *****\" ]");
        	return;
        }
        else this->SetToStringFlag();

        xx::Append(s, "{ \"pkgTypeName\":\"DB.Account\", \"pkgTypeId\":", GetTypeId());
        ToStringCore(s);
        xx::Append(s, " }");
        
        this->SetToStringFlag(false);
    }
    inline void Account::ToStringCore(std::string& s) const noexcept {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ", \"id\":", this->id);
        if (this->username) xx::Append(s, ", \"username\":\"", this->username, "\"");
        else xx::Append(s, ", \"username\":nil");
        if (this->password) xx::Append(s, ", \"password\":\"", this->password, "\"");
        else xx::Append(s, ", \"password\":nil");
    }
    inline std::shared_ptr<Account> Account::MakeShared() noexcept {
        return std::make_shared<Account>();
    }
}
}
namespace PKG {
	inline void AllTypesRegister() noexcept {
	    xx::BBuffer::Register<PKG::Success>(3);
	    xx::BBuffer::Register<PKG::Error>(4);
	    xx::BBuffer::Register<PKG::ConnInfo>(6);
	    xx::BBuffer::Register<PKG::Collections>(7);
	    xx::BBuffer::Register<xx::List<int32_t>>(8);
	    xx::BBuffer::Register<xx::List<int64_t>>(9);
	    xx::BBuffer::Register<xx::List<std::string_s>>(10);
	    xx::BBuffer::Register<xx::List<xx::Object_s>>(11);
	    xx::BBuffer::Register<PKG::Client_Login::Auth>(12);
	    xx::BBuffer::Register<PKG::Server::Info>(5);
	    xx::BBuffer::Register<PKG::Client_Lobby::Enter>(13);
	    xx::BBuffer::Register<PKG::Client_Lobby::Enter_Game1>(14);
	    xx::BBuffer::Register<PKG::Client_Lobby::Enter_Game1_Level>(15);
	    xx::BBuffer::Register<PKG::Client_Lobby::Enter_Game1_Level_Desk>(16);
	    xx::BBuffer::Register<PKG::Client_Lobby::Back>(17);
	    xx::BBuffer::Register<PKG::Lobby_Client::Self>(18);
	    xx::BBuffer::Register<PKG::Lobby_Client::Player>(19);
	    xx::BBuffer::Register<PKG::Lobby_Client::Root>(20);
	    xx::BBuffer::Register<PKG::Lobby_Client::Game1>(21);
	    xx::BBuffer::Register<xx::List<PKG::Lobby_Client::Game1_Level_Info_s>>(22);
	    xx::BBuffer::Register<PKG::Lobby_Client::Game1_Level_Info>(23);
	    xx::BBuffer::Register<PKG::Lobby_Client::Game1_Level>(24);
	    xx::BBuffer::Register<xx::List<PKG::Lobby_Client::Game1_Level_Desk_s>>(25);
	    xx::BBuffer::Register<PKG::Lobby_Client::Game1_Level_Desk>(26);
	    xx::BBuffer::Register<xx::List<PKG::Lobby_Client::Player_s>>(27);
	    xx::BBuffer::Register<PKG::Lobby::Player>(28);
	    xx::BBuffer::Register<PKG::Lobby::Place>(29);
	    xx::BBuffer::Register<xx::List<PKG::Lobby::Player_s>>(30);
	    xx::BBuffer::Register<PKG::Lobby::Root>(31);
	    xx::BBuffer::Register<xx::List<PKG::Lobby::Game_s>>(32);
	    xx::BBuffer::Register<PKG::Lobby::Game>(33);
	    xx::BBuffer::Register<PKG::Lobby::Game1>(34);
	    xx::BBuffer::Register<xx::List<PKG::Lobby::Game1_Level_s>>(35);
	    xx::BBuffer::Register<PKG::Lobby::Game1_Level>(36);
	    xx::BBuffer::Register<xx::List<PKG::Lobby::Game1_Level_Desk_s>>(37);
	    xx::BBuffer::Register<PKG::Lobby::Game1_Level_Desk>(38);
	    xx::BBuffer::Register<PKG::Login_DB::GetAccount>(39);
	    xx::BBuffer::Register<PKG::DB::Account>(40);
	}
}
