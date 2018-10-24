#pragma once
#include "xx.h"

namespace PKG
{
	struct PkgGenMd5
	{
		static constexpr char const* value = "b3a4eeaf69586aa565e1b9d5415e07b1";
    };

    // 操作成功( 默认 response 结果 )
    class Success;
    using Success_p = xx::Ptr<Success>;
    using Success_r = xx::Ref<Success>;

    // 出错( 通用 response 结果 )
    class Error;
    using Error_p = xx::Ptr<Error>;
    using Error_r = xx::Ref<Error>;

    // 服务连接信息
    class ConnInfo;
    using ConnInfo_p = xx::Ptr<ConnInfo>;
    using ConnInfo_r = xx::Ref<ConnInfo>;

    // 并非一般的数据包. 仅用于声明各式 List<T>
    class Collections;
    using Collections_p = xx::Ptr<Collections>;
    using Collections_r = xx::Ref<Collections>;

namespace Client_Login
{
    // 校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T
    class Auth;
    using Auth_p = xx::Ptr<Auth>;
    using Auth_r = xx::Ref<Auth>;

}
namespace Server
{
    // 服务间互表身份的首包
    class Info;
    using Info_p = xx::Ptr<Info>;
    using Info_r = xx::Ref<Info>;

}
namespace Client_Lobby
{
    // 首包. 进入大厅. 成功返回 Self( 含 Root 以及个人信息 ). 如果已经位于具体游戏中, 返回 ConnInfo. 失败立即被 T
    class Enter;
    using Enter_p = xx::Ptr<Enter>;
    using Enter_r = xx::Ref<Enter>;

    // 进入 Game1, 位于 Root 时可发送, 返回 Game1. 失败立即被 T
    class Enter_Game1;
    using Enter_Game1_p = xx::Ptr<Enter_Game1>;
    using Enter_Game1_r = xx::Ref<Enter_Game1>;

    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    class Enter_Game1_Level;
    using Enter_Game1_Level_p = xx::Ptr<Enter_Game1_Level>;
    using Enter_Game1_Level_r = xx::Ref<Enter_Game1_Level>;

    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    class Enter_Game1_Level_Desk;
    using Enter_Game1_Level_Desk_p = xx::Ptr<Enter_Game1_Level_Desk>;
    using Enter_Game1_Level_Desk_r = xx::Ref<Enter_Game1_Level_Desk>;

    // 退回上一层. 失败立即被 T
    class Back;
    using Back_p = xx::Ptr<Back>;
    using Back_r = xx::Ref<Back>;

}
namespace Lobby_Client
{
    // 玩家自己的数据
    class Self;
    using Self_p = xx::Ptr<Self>;
    using Self_r = xx::Ref<Self>;

    // 其他玩家的数据
    class Player;
    using Player_p = xx::Ptr<Player>;
    using Player_r = xx::Ref<Player>;

    // 大厅根部
    class Root;
    using Root_p = xx::Ptr<Root>;
    using Root_r = xx::Ref<Root>;

    // Game 特化: Game1 具体配置信息
    class Game1;
    using Game1_p = xx::Ptr<Game1>;
    using Game1_r = xx::Ref<Game1>;

    // Game1 级别的详细数据
    class Game1_Level_Info;
    using Game1_Level_Info_p = xx::Ptr<Game1_Level_Info>;
    using Game1_Level_Info_r = xx::Ref<Game1_Level_Info>;

    // Game1 级别的详细数据
    class Game1_Level;
    using Game1_Level_p = xx::Ptr<Game1_Level>;
    using Game1_Level_r = xx::Ref<Game1_Level>;

    // Game1 级别 下的 桌子 的详细数据
    class Game1_Level_Desk;
    using Game1_Level_Desk_p = xx::Ptr<Game1_Level_Desk>;
    using Game1_Level_Desk_r = xx::Ref<Game1_Level_Desk>;

}
namespace Lobby
{
    // 玩家明细
    class Player;
    using Player_p = xx::Ptr<Player>;
    using Player_r = xx::Ref<Player>;

    // 玩家容器基类
    class Place;
    using Place_p = xx::Ptr<Place>;
    using Place_r = xx::Ref<Place>;

    // 大厅根部
    class Root;
    using Root_p = xx::Ptr<Root>;
    using Root_r = xx::Ref<Root>;

    // 游戏基类
    class Game;
    using Game_p = xx::Ptr<Game>;
    using Game_r = xx::Ref<Game>;

    // Game 特化: Game1 具体配置信息
    class Game1;
    using Game1_p = xx::Ptr<Game1>;
    using Game1_r = xx::Ref<Game1>;

    // Game1 级别的详细数据
    class Game1_Level;
    using Game1_Level_p = xx::Ptr<Game1_Level>;
    using Game1_Level_r = xx::Ref<Game1_Level>;

    // Game1 级别 下的 桌子 的详细数据
    class Game1_Level_Desk;
    using Game1_Level_Desk_p = xx::Ptr<Game1_Level_Desk>;
    using Game1_Level_Desk_r = xx::Ref<Game1_Level_Desk>;

}
namespace Login_DB
{
    // 根据用户名获取用户信息. 找到就返回 DB.Account. 找不到就返回 Error
    class GetAccount;
    using GetAccount_p = xx::Ptr<GetAccount>;
    using GetAccount_r = xx::Ref<GetAccount>;

}
namespace DB
{
    class Account;
    using Account_p = xx::Ptr<Account>;
    using Account_r = xx::Ref<Account>;

}
namespace Server
{
    enum class Types : int32_t
    {
        Unknown = 0,
        Login = 1,
        Lobby = 2,
        DB = 3,
        MAX = 4,
    };
}
namespace Lobby
{
    // 玩家容器基类
    class Place : public xx::Object
    {
    public:
        // 指向上层容器( Root 没有上层容器 )
        PKG::Lobby::Place_p parent;
        // 玩家容器基类
        xx::List_p<PKG::Lobby::Player_p> players;

        typedef Place ThisType;
        typedef xx::Object BaseType;
	    Place(xx::MemPool* const& mempool) noexcept;
	    Place(xx::BBuffer* const& bb);
		Place(Place const&) = delete;
		Place& operator=(Place const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Place* const& o) const noexcept;
        Place* MakeCopy() const noexcept;
        Place_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 游戏基类
    class Game : public PKG::Lobby::Place
    {
    public:
        // 游戏id
        int32_t id = 0;

        typedef Game ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Game(xx::MemPool* const& mempool) noexcept;
	    Game(xx::BBuffer* const& bb);
		Game(Game const&) = delete;
		Game& operator=(Game const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game* const& o) const noexcept;
        Game* MakeCopy() const noexcept;
        Game_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
    // 操作成功( 默认 response 结果 )
    class Success : public xx::Object
    {
    public:

        typedef Success ThisType;
        typedef xx::Object BaseType;
	    Success(xx::MemPool* const& mempool) noexcept;
	    Success(xx::BBuffer* const& bb);
		Success(Success const&) = delete;
		Success& operator=(Success const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Success* const& o) const noexcept;
        Success* MakeCopy() const noexcept;
        Success_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
namespace Lobby
{
    // Game1 级别 下的 桌子 的详细数据
    class Game1_Level_Desk : public PKG::Lobby::Place
    {
    public:
        // 桌子编号
        int32_t id = 0;

        typedef Game1_Level_Desk ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Game1_Level_Desk(xx::MemPool* const& mempool) noexcept;
	    Game1_Level_Desk(xx::BBuffer* const& bb);
		Game1_Level_Desk(Game1_Level_Desk const&) = delete;
		Game1_Level_Desk& operator=(Game1_Level_Desk const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game1_Level_Desk* const& o) const noexcept;
        Game1_Level_Desk* MakeCopy() const noexcept;
        Game1_Level_Desk_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // Game1 级别的详细数据
    class Game1_Level : public PKG::Lobby::Place
    {
    public:
        // 级别编号
        int32_t id = 0;
        // 准入门槛
        double minMoney = 0;
        // 该级别下所有桌子列表
        xx::List_p<PKG::Lobby::Game1_Level_Desk_p> desks;

        typedef Game1_Level ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Game1_Level(xx::MemPool* const& mempool) noexcept;
	    Game1_Level(xx::BBuffer* const& bb);
		Game1_Level(Game1_Level const&) = delete;
		Game1_Level& operator=(Game1_Level const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game1_Level* const& o) const noexcept;
        Game1_Level* MakeCopy() const noexcept;
        Game1_Level_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // Game 特化: Game1 具体配置信息
    class Game1 : public PKG::Lobby::Game
    {
    public:
        // Game1 级别列表
        xx::List_p<PKG::Lobby::Game1_Level_p> levels;

        typedef Game1 ThisType;
        typedef PKG::Lobby::Game BaseType;
	    Game1(xx::MemPool* const& mempool) noexcept;
	    Game1(xx::BBuffer* const& bb);
		Game1(Game1 const&) = delete;
		Game1& operator=(Game1 const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game1* const& o) const noexcept;
        Game1* MakeCopy() const noexcept;
        Game1_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 大厅根部
    class Root : public PKG::Lobby::Place
    {
    public:
        // 所有游戏列表
        xx::List_p<PKG::Lobby::Game_p> games;

        typedef Root ThisType;
        typedef PKG::Lobby::Place BaseType;
	    Root(xx::MemPool* const& mempool) noexcept;
	    Root(xx::BBuffer* const& bb);
		Root(Root const&) = delete;
		Root& operator=(Root const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Root* const& o) const noexcept;
        Root* MakeCopy() const noexcept;
        Root_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 玩家明细
    class Player : public xx::Object
    {
    public:
        // 玩家id
        int32_t id = 0;
        // 名字
        xx::String_p username;
        // 有多少钱
        double money = 0;
        // 当前位置
        PKG::Lobby::Place_p place;
        // 位于 players 数组中的下标( 便于交换删除 )
        int32_t indexAtContainer = 0;
        // 特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息
        int32_t game1_Level_Desk_SeatIndex = 0;

        typedef Player ThisType;
        typedef xx::Object BaseType;
	    Player(xx::MemPool* const& mempool) noexcept;
	    Player(xx::BBuffer* const& bb);
		Player(Player const&) = delete;
		Player& operator=(Player const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Player* const& o) const noexcept;
        Player* MakeCopy() const noexcept;
        Player_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
namespace Lobby_Client
{
    // Game1 级别 下的 桌子 的详细数据
    class Game1_Level_Desk : public xx::Object
    {
    public:
        // 桌子编号
        int32_t id = 0;
        // 玩家列表
        xx::List_p<PKG::Lobby_Client::Player_p> players;

        typedef Game1_Level_Desk ThisType;
        typedef xx::Object BaseType;
	    Game1_Level_Desk(xx::MemPool* const& mempool) noexcept;
	    Game1_Level_Desk(xx::BBuffer* const& bb);
		Game1_Level_Desk(Game1_Level_Desk const&) = delete;
		Game1_Level_Desk& operator=(Game1_Level_Desk const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game1_Level_Desk* const& o) const noexcept;
        Game1_Level_Desk* MakeCopy() const noexcept;
        Game1_Level_Desk_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // Game1 级别的详细数据
    class Game1_Level : public xx::Object
    {
    public:
        // 级别编号
        int32_t id = 0;
        // 准入门槛
        double minMoney = 0;
        // 该级别下所有桌子列表
        xx::List_p<PKG::Lobby_Client::Game1_Level_Desk_p> desks;

        typedef Game1_Level ThisType;
        typedef xx::Object BaseType;
	    Game1_Level(xx::MemPool* const& mempool) noexcept;
	    Game1_Level(xx::BBuffer* const& bb);
		Game1_Level(Game1_Level const&) = delete;
		Game1_Level& operator=(Game1_Level const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game1_Level* const& o) const noexcept;
        Game1_Level* MakeCopy() const noexcept;
        Game1_Level_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // Game1 级别的详细数据
    class Game1_Level_Info : public xx::Object
    {
    public:
        // 级别编号
        int32_t id = 0;
        // 准入门槛
        double minMoney = 0;

        typedef Game1_Level_Info ThisType;
        typedef xx::Object BaseType;
	    Game1_Level_Info(xx::MemPool* const& mempool) noexcept;
	    Game1_Level_Info(xx::BBuffer* const& bb);
		Game1_Level_Info(Game1_Level_Info const&) = delete;
		Game1_Level_Info& operator=(Game1_Level_Info const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game1_Level_Info* const& o) const noexcept;
        Game1_Level_Info* MakeCopy() const noexcept;
        Game1_Level_Info_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // Game 特化: Game1 具体配置信息
    class Game1 : public xx::Object
    {
    public:
        // Game1 级别列表
        xx::List_p<PKG::Lobby_Client::Game1_Level_Info_p> levels;

        typedef Game1 ThisType;
        typedef xx::Object BaseType;
	    Game1(xx::MemPool* const& mempool) noexcept;
	    Game1(xx::BBuffer* const& bb);
		Game1(Game1 const&) = delete;
		Game1& operator=(Game1 const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Game1* const& o) const noexcept;
        Game1* MakeCopy() const noexcept;
        Game1_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
namespace Login_DB
{
    // 根据用户名获取用户信息. 找到就返回 DB.Account. 找不到就返回 Error
    class GetAccount : public xx::Object
    {
    public:
        xx::String_p username;

        typedef GetAccount ThisType;
        typedef xx::Object BaseType;
	    GetAccount(xx::MemPool* const& mempool) noexcept;
	    GetAccount(xx::BBuffer* const& bb);
		GetAccount(GetAccount const&) = delete;
		GetAccount& operator=(GetAccount const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(GetAccount* const& o) const noexcept;
        GetAccount* MakeCopy() const noexcept;
        GetAccount_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
namespace Lobby_Client
{
    // 大厅根部
    class Root : public xx::Object
    {
    public:
        // 当前玩家可见的游戏列表
        xx::List_p<int32_t> gameIds;
        // 玩家自己的数据
        PKG::Lobby_Client::Self_p self;

        typedef Root ThisType;
        typedef xx::Object BaseType;
	    Root(xx::MemPool* const& mempool) noexcept;
	    Root(xx::BBuffer* const& bb);
		Root(Root const&) = delete;
		Root& operator=(Root const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Root* const& o) const noexcept;
        Root* MakeCopy() const noexcept;
        Root_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 玩家自己的数据
    class Self : public xx::Object
    {
    public:
        // 玩家id
        int32_t id = 0;
        // 有多少钱
        double money = 0;

        typedef Self ThisType;
        typedef xx::Object BaseType;
	    Self(xx::MemPool* const& mempool) noexcept;
	    Self(xx::BBuffer* const& bb);
		Self(Self const&) = delete;
		Self& operator=(Self const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Self* const& o) const noexcept;
        Self* MakeCopy() const noexcept;
        Self_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
namespace Client_Lobby
{
    // 退回上一层. 失败立即被 T
    class Back : public xx::Object
    {
    public:

        typedef Back ThisType;
        typedef xx::Object BaseType;
	    Back(xx::MemPool* const& mempool) noexcept;
	    Back(xx::BBuffer* const& bb);
		Back(Back const&) = delete;
		Back& operator=(Back const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Back* const& o) const noexcept;
        Back* MakeCopy() const noexcept;
        Back_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    class Enter_Game1_Level_Desk : public xx::Object
    {
    public:
        // 指定 Desk id
        int32_t id = 0;
        // 指定座次
        int32_t seatIndex = 0;

        typedef Enter_Game1_Level_Desk ThisType;
        typedef xx::Object BaseType;
	    Enter_Game1_Level_Desk(xx::MemPool* const& mempool) noexcept;
	    Enter_Game1_Level_Desk(xx::BBuffer* const& bb);
		Enter_Game1_Level_Desk(Enter_Game1_Level_Desk const&) = delete;
		Enter_Game1_Level_Desk& operator=(Enter_Game1_Level_Desk const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Enter_Game1_Level_Desk* const& o) const noexcept;
        Enter_Game1_Level_Desk* MakeCopy() const noexcept;
        Enter_Game1_Level_Desk_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T
    class Enter_Game1_Level : public xx::Object
    {
    public:
        // 指定 Level id
        int32_t id = 0;

        typedef Enter_Game1_Level ThisType;
        typedef xx::Object BaseType;
	    Enter_Game1_Level(xx::MemPool* const& mempool) noexcept;
	    Enter_Game1_Level(xx::BBuffer* const& bb);
		Enter_Game1_Level(Enter_Game1_Level const&) = delete;
		Enter_Game1_Level& operator=(Enter_Game1_Level const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Enter_Game1_Level* const& o) const noexcept;
        Enter_Game1_Level* MakeCopy() const noexcept;
        Enter_Game1_Level_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 进入 Game1, 位于 Root 时可发送, 返回 Game1. 失败立即被 T
    class Enter_Game1 : public xx::Object
    {
    public:

        typedef Enter_Game1 ThisType;
        typedef xx::Object BaseType;
	    Enter_Game1(xx::MemPool* const& mempool) noexcept;
	    Enter_Game1(xx::BBuffer* const& bb);
		Enter_Game1(Enter_Game1 const&) = delete;
		Enter_Game1& operator=(Enter_Game1 const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Enter_Game1* const& o) const noexcept;
        Enter_Game1* MakeCopy() const noexcept;
        Enter_Game1_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 首包. 进入大厅. 成功返回 Self( 含 Root 以及个人信息 ). 如果已经位于具体游戏中, 返回 ConnInfo. 失败立即被 T
    class Enter : public xx::Object
    {
    public:
        xx::String_p token;

        typedef Enter ThisType;
        typedef xx::Object BaseType;
	    Enter(xx::MemPool* const& mempool) noexcept;
	    Enter(xx::BBuffer* const& bb);
		Enter(Enter const&) = delete;
		Enter& operator=(Enter const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Enter* const& o) const noexcept;
        Enter* MakeCopy() const noexcept;
        Enter_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
namespace Server
{
    // 服务间互表身份的首包
    class Info : public xx::Object
    {
    public:
        PKG::Server::Types type = (PKG::Server::Types)0;

        typedef Info ThisType;
        typedef xx::Object BaseType;
	    Info(xx::MemPool* const& mempool) noexcept;
	    Info(xx::BBuffer* const& bb);
		Info(Info const&) = delete;
		Info& operator=(Info const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Info* const& o) const noexcept;
        Info* MakeCopy() const noexcept;
        Info_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
namespace Client_Login
{
    // 校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T
    class Auth : public xx::Object
    {
    public:
        // 包版本校验
        xx::String_p pkgMD5;
        // 用户名
        xx::String_p username;
        // 密码
        xx::String_p password;

        typedef Auth ThisType;
        typedef xx::Object BaseType;
	    Auth(xx::MemPool* const& mempool) noexcept;
	    Auth(xx::BBuffer* const& bb);
		Auth(Auth const&) = delete;
		Auth& operator=(Auth const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Auth* const& o) const noexcept;
        Auth* MakeCopy() const noexcept;
        Auth_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
    // 并非一般的数据包. 仅用于声明各式 List<T>
    class Collections : public xx::Object
    {
    public:
        xx::List_p<int32_t> ints;
        xx::List_p<int64_t> longs;
        xx::List_p<xx::String_p> strings;
        xx::List_p<xx::Object_p> objects;

        typedef Collections ThisType;
        typedef xx::Object BaseType;
	    Collections(xx::MemPool* const& mempool) noexcept;
	    Collections(xx::BBuffer* const& bb);
		Collections(Collections const&) = delete;
		Collections& operator=(Collections const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Collections* const& o) const noexcept;
        Collections* MakeCopy() const noexcept;
        Collections_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 服务连接信息
    class ConnInfo : public xx::Object
    {
    public:
        // 服务类型
        PKG::Server::Types type = (PKG::Server::Types)0;
        // ipv4/6 地址
        xx::String_p ip;
        // 端口
        int32_t port = 0;
        // 令牌
        xx::String_p token;

        typedef ConnInfo ThisType;
        typedef xx::Object BaseType;
	    ConnInfo(xx::MemPool* const& mempool) noexcept;
	    ConnInfo(xx::BBuffer* const& bb);
		ConnInfo(ConnInfo const&) = delete;
		ConnInfo& operator=(ConnInfo const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(ConnInfo* const& o) const noexcept;
        ConnInfo* MakeCopy() const noexcept;
        ConnInfo_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
    // 出错( 通用 response 结果 )
    class Error : public xx::Object
    {
    public:
        int32_t id = 0;
        xx::String_p txt;

        typedef Error ThisType;
        typedef xx::Object BaseType;
	    Error(xx::MemPool* const& mempool) noexcept;
	    Error(xx::BBuffer* const& bb);
		Error(Error const&) = delete;
		Error& operator=(Error const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Error* const& o) const noexcept;
        Error* MakeCopy() const noexcept;
        Error_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
namespace Lobby_Client
{
    // 其他玩家的数据
    class Player : public xx::Object
    {
    public:
        // 玩家id
        int32_t id = 0;
        // 名字
        xx::String_p username;
        // 特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息
        int32_t game1_Level_Desk_SeatIndex = 0;

        typedef Player ThisType;
        typedef xx::Object BaseType;
	    Player(xx::MemPool* const& mempool) noexcept;
	    Player(xx::BBuffer* const& bb);
		Player(Player const&) = delete;
		Player& operator=(Player const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Player* const& o) const noexcept;
        Player* MakeCopy() const noexcept;
        Player_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
namespace DB
{
    class Account : public xx::Object
    {
    public:
        int32_t id = 0;
        xx::String_p username;
        xx::String_p password;

        typedef Account ThisType;
        typedef xx::Object BaseType;
	    Account(xx::MemPool* const& mempool) noexcept;
	    Account(xx::BBuffer* const& bb);
		Account(Account const&) = delete;
		Account& operator=(Account const&) = delete;
        void ToString(xx::String& s) const noexcept override;
        void ToStringCore(xx::String& s) const noexcept override;
        void ToBBuffer(xx::BBuffer& bb) const noexcept override;
        int FromBBuffer(xx::BBuffer& bb) noexcept override;
        int FromBBufferCore(xx::BBuffer& bb) noexcept;
        void CopyTo(Account* const& o) const noexcept;
        Account* MakeCopy() const noexcept;
        Account_p MakePtrCopy() const noexcept;
        inline static xx::Ptr<ThisType> defaultInstance;
    };
}
}
namespace xx
{
	template<> struct TypeId<PKG::Success> { static const uint16_t value = 3; };
	template<> struct TypeId<PKG::Error> { static const uint16_t value = 4; };
	template<> struct TypeId<PKG::ConnInfo> { static const uint16_t value = 6; };
	template<> struct TypeId<PKG::Collections> { static const uint16_t value = 7; };
	template<> struct TypeId<xx::List<int32_t>> { static const uint16_t value = 8; };
	template<> struct TypeId<xx::List<int64_t>> { static const uint16_t value = 9; };
	template<> struct TypeId<xx::List<xx::String_p>> { static const uint16_t value = 10; };
	template<> struct TypeId<xx::List<xx::Object_p>> { static const uint16_t value = 11; };
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
	template<> struct TypeId<xx::List<PKG::Lobby_Client::Game1_Level_Info_p>> { static const uint16_t value = 22; };
	template<> struct TypeId<PKG::Lobby_Client::Game1_Level_Info> { static const uint16_t value = 23; };
	template<> struct TypeId<PKG::Lobby_Client::Game1_Level> { static const uint16_t value = 24; };
	template<> struct TypeId<xx::List<PKG::Lobby_Client::Game1_Level_Desk_p>> { static const uint16_t value = 25; };
	template<> struct TypeId<PKG::Lobby_Client::Game1_Level_Desk> { static const uint16_t value = 26; };
	template<> struct TypeId<xx::List<PKG::Lobby_Client::Player_p>> { static const uint16_t value = 27; };
	template<> struct TypeId<PKG::Lobby::Player> { static const uint16_t value = 28; };
	template<> struct TypeId<PKG::Lobby::Place> { static const uint16_t value = 29; };
	template<> struct TypeId<xx::List<PKG::Lobby::Player_p>> { static const uint16_t value = 30; };
	template<> struct TypeId<PKG::Lobby::Root> { static const uint16_t value = 31; };
	template<> struct TypeId<xx::List<PKG::Lobby::Game_p>> { static const uint16_t value = 32; };
	template<> struct TypeId<PKG::Lobby::Game> { static const uint16_t value = 33; };
	template<> struct TypeId<PKG::Lobby::Game1> { static const uint16_t value = 34; };
	template<> struct TypeId<xx::List<PKG::Lobby::Game1_Level_p>> { static const uint16_t value = 35; };
	template<> struct TypeId<PKG::Lobby::Game1_Level> { static const uint16_t value = 36; };
	template<> struct TypeId<xx::List<PKG::Lobby::Game1_Level_Desk_p>> { static const uint16_t value = 37; };
	template<> struct TypeId<PKG::Lobby::Game1_Level_Desk> { static const uint16_t value = 38; };
	template<> struct TypeId<PKG::Login_DB::GetAccount> { static const uint16_t value = 39; };
	template<> struct TypeId<PKG::DB::Account> { static const uint16_t value = 40; };
}
namespace PKG
{
	inline Success::Success(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Success::Success(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Success::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
    }
    inline int Success::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Success::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        return 0;
    }

    inline void Success::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Success\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Success::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
    }
    inline void Success::CopyTo(Success* const& o) const noexcept
    {
    }
    inline Success* Success::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Success>();
        this->CopyTo(o);
        return o;
    }
    inline Success_p Success::MakePtrCopy() const noexcept
    {
        return Success_p(this->MakeCopy());
    }

	inline Error::Error(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Error::Error(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Error::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->txt);
    }
    inline int Error::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Error::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->txt)) return r;
        return 0;
    }

    inline void Error::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Error\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Error::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        if (this->txt) s.Append(", \"txt\":\"", this->txt, "\"");
        else s.Append(", \"txt\":nil");
    }
    inline void Error::CopyTo(Error* const& o) const noexcept
    {
        o->id = this->id;
        o->txt = this->txt;
    }
    inline Error* Error::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Error>();
        this->CopyTo(o);
        return o;
    }
    inline Error_p Error::MakePtrCopy() const noexcept
    {
        return Error_p(this->MakeCopy());
    }

	inline ConnInfo::ConnInfo(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline ConnInfo::ConnInfo(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void ConnInfo::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->type);
        bb.Write(this->ip);
        bb.Write(this->port);
        bb.Write(this->token);
    }
    inline int ConnInfo::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int ConnInfo::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->type)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->ip)) return r;
        if (int r = bb.Read(this->port)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->token)) return r;
        return 0;
    }

    inline void ConnInfo::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"ConnInfo\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void ConnInfo::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"type\":", this->type);
        if (this->ip) s.Append(", \"ip\":\"", this->ip, "\"");
        else s.Append(", \"ip\":nil");
        s.Append(", \"port\":", this->port);
        if (this->token) s.Append(", \"token\":\"", this->token, "\"");
        else s.Append(", \"token\":nil");
    }
    inline void ConnInfo::CopyTo(ConnInfo* const& o) const noexcept
    {
        o->type = this->type;
        o->ip = this->ip;
        o->port = this->port;
        o->token = this->token;
    }
    inline ConnInfo* ConnInfo::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<ConnInfo>();
        this->CopyTo(o);
        return o;
    }
    inline ConnInfo_p ConnInfo::MakePtrCopy() const noexcept
    {
        return ConnInfo_p(this->MakeCopy());
    }

	inline Collections::Collections(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Collections::Collections(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Collections::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->ints);
        bb.Write(this->longs);
        bb.Write(this->strings);
        bb.Write(this->objects);
    }
    inline int Collections::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Collections::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
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

    inline void Collections::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Collections\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Collections::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"ints\":", this->ints);
        s.Append(", \"longs\":", this->longs);
        s.Append(", \"strings\":", this->strings);
        s.Append(", \"objects\":", this->objects);
    }
    inline void Collections::CopyTo(Collections* const& o) const noexcept
    {
        o->ints = this->ints;
        o->longs = this->longs;
        o->strings = this->strings;
        o->objects = this->objects;
    }
    inline Collections* Collections::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Collections>();
        this->CopyTo(o);
        return o;
    }
    inline Collections_p Collections::MakePtrCopy() const noexcept
    {
        return Collections_p(this->MakeCopy());
    }

namespace Client_Login
{
	inline Auth::Auth(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Auth::Auth(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Auth::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->pkgMD5);
        bb.Write(this->username);
        bb.Write(this->password);
    }
    inline int Auth::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Auth::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->pkgMD5)) return r;
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->username)) return r;
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->password)) return r;
        return 0;
    }

    inline void Auth::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Client_Login.Auth\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Auth::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        if (this->pkgMD5) s.Append(", \"pkgMD5\":\"", this->pkgMD5, "\"");
        else s.Append(", \"pkgMD5\":nil");
        if (this->username) s.Append(", \"username\":\"", this->username, "\"");
        else s.Append(", \"username\":nil");
        if (this->password) s.Append(", \"password\":\"", this->password, "\"");
        else s.Append(", \"password\":nil");
    }
    inline void Auth::CopyTo(Auth* const& o) const noexcept
    {
        o->pkgMD5 = this->pkgMD5;
        o->username = this->username;
        o->password = this->password;
    }
    inline Auth* Auth::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Auth>();
        this->CopyTo(o);
        return o;
    }
    inline Auth_p Auth::MakePtrCopy() const noexcept
    {
        return Auth_p(this->MakeCopy());
    }

}
namespace Server
{
	inline Info::Info(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Info::Info(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Info::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->type);
    }
    inline int Info::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Info::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->type)) return r;
        return 0;
    }

    inline void Info::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Server.Info\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Info::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"type\":", this->type);
    }
    inline void Info::CopyTo(Info* const& o) const noexcept
    {
        o->type = this->type;
    }
    inline Info* Info::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Info>();
        this->CopyTo(o);
        return o;
    }
    inline Info_p Info::MakePtrCopy() const noexcept
    {
        return Info_p(this->MakeCopy());
    }

}
namespace Client_Lobby
{
	inline Enter::Enter(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Enter::Enter(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Enter::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->token);
    }
    inline int Enter::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Enter::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        bb.readLengthLimit = 64;
        if (int r = bb.Read(this->token)) return r;
        return 0;
    }

    inline void Enter::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Enter::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        if (this->token) s.Append(", \"token\":\"", this->token, "\"");
        else s.Append(", \"token\":nil");
    }
    inline void Enter::CopyTo(Enter* const& o) const noexcept
    {
        o->token = this->token;
    }
    inline Enter* Enter::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Enter>();
        this->CopyTo(o);
        return o;
    }
    inline Enter_p Enter::MakePtrCopy() const noexcept
    {
        return Enter_p(this->MakeCopy());
    }

	inline Enter_Game1::Enter_Game1(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Enter_Game1::Enter_Game1(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Enter_Game1::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
    }
    inline int Enter_Game1::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Enter_Game1::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        return 0;
    }

    inline void Enter_Game1::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Enter_Game1::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
    }
    inline void Enter_Game1::CopyTo(Enter_Game1* const& o) const noexcept
    {
    }
    inline Enter_Game1* Enter_Game1::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Enter_Game1>();
        this->CopyTo(o);
        return o;
    }
    inline Enter_Game1_p Enter_Game1::MakePtrCopy() const noexcept
    {
        return Enter_Game1_p(this->MakeCopy());
    }

	inline Enter_Game1_Level::Enter_Game1_Level(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Enter_Game1_Level::Enter_Game1_Level(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Enter_Game1_Level::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
    }
    inline int Enter_Game1_Level::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Enter_Game1_Level::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        return 0;
    }

    inline void Enter_Game1_Level::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1_Level\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Enter_Game1_Level::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
    }
    inline void Enter_Game1_Level::CopyTo(Enter_Game1_Level* const& o) const noexcept
    {
        o->id = this->id;
    }
    inline Enter_Game1_Level* Enter_Game1_Level::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Enter_Game1_Level>();
        this->CopyTo(o);
        return o;
    }
    inline Enter_Game1_Level_p Enter_Game1_Level::MakePtrCopy() const noexcept
    {
        return Enter_Game1_Level_p(this->MakeCopy());
    }

	inline Enter_Game1_Level_Desk::Enter_Game1_Level_Desk(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Enter_Game1_Level_Desk::Enter_Game1_Level_Desk(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Enter_Game1_Level_Desk::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->seatIndex);
    }
    inline int Enter_Game1_Level_Desk::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Enter_Game1_Level_Desk::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->seatIndex)) return r;
        return 0;
    }

    inline void Enter_Game1_Level_Desk::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Client_Lobby.Enter_Game1_Level_Desk\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Enter_Game1_Level_Desk::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        s.Append(", \"seatIndex\":", this->seatIndex);
    }
    inline void Enter_Game1_Level_Desk::CopyTo(Enter_Game1_Level_Desk* const& o) const noexcept
    {
        o->id = this->id;
        o->seatIndex = this->seatIndex;
    }
    inline Enter_Game1_Level_Desk* Enter_Game1_Level_Desk::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Enter_Game1_Level_Desk>();
        this->CopyTo(o);
        return o;
    }
    inline Enter_Game1_Level_Desk_p Enter_Game1_Level_Desk::MakePtrCopy() const noexcept
    {
        return Enter_Game1_Level_Desk_p(this->MakeCopy());
    }

	inline Back::Back(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Back::Back(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Back::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
    }
    inline int Back::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Back::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        return 0;
    }

    inline void Back::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Client_Lobby.Back\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Back::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
    }
    inline void Back::CopyTo(Back* const& o) const noexcept
    {
    }
    inline Back* Back::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Back>();
        this->CopyTo(o);
        return o;
    }
    inline Back_p Back::MakePtrCopy() const noexcept
    {
        return Back_p(this->MakeCopy());
    }

}
namespace Lobby_Client
{
	inline Self::Self(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Self::Self(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Self::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->money);
    }
    inline int Self::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Self::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->money)) return r;
        return 0;
    }

    inline void Self::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby_Client.Self\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Self::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        s.Append(", \"money\":", this->money);
    }
    inline void Self::CopyTo(Self* const& o) const noexcept
    {
        o->id = this->id;
        o->money = this->money;
    }
    inline Self* Self::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Self>();
        this->CopyTo(o);
        return o;
    }
    inline Self_p Self::MakePtrCopy() const noexcept
    {
        return Self_p(this->MakeCopy());
    }

	inline Player::Player(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Player::Player(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->username);
        bb.Write(this->game1_Level_Desk_SeatIndex);
    }
    inline int Player::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Player::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        if (int r = bb.Read(this->game1_Level_Desk_SeatIndex)) return r;
        return 0;
    }

    inline void Player::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby_Client.Player\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Player::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        if (this->username) s.Append(", \"username\":\"", this->username, "\"");
        else s.Append(", \"username\":nil");
        s.Append(", \"game1_Level_Desk_SeatIndex\":", this->game1_Level_Desk_SeatIndex);
    }
    inline void Player::CopyTo(Player* const& o) const noexcept
    {
        o->id = this->id;
        o->username = this->username;
        o->game1_Level_Desk_SeatIndex = this->game1_Level_Desk_SeatIndex;
    }
    inline Player* Player::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Player>();
        this->CopyTo(o);
        return o;
    }
    inline Player_p Player::MakePtrCopy() const noexcept
    {
        return Player_p(this->MakeCopy());
    }

	inline Root::Root(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Root::Root(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Root::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->gameIds);
        bb.Write(this->self);
    }
    inline int Root::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Root::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->gameIds)) return r;
        if (int r = bb.Read(this->self)) return r;
        return 0;
    }

    inline void Root::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby_Client.Root\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Root::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"gameIds\":", this->gameIds);
        s.Append(", \"self\":", this->self);
    }
    inline void Root::CopyTo(Root* const& o) const noexcept
    {
        o->gameIds = this->gameIds;
        o->self = this->self;
    }
    inline Root* Root::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Root>();
        this->CopyTo(o);
        return o;
    }
    inline Root_p Root::MakePtrCopy() const noexcept
    {
        return Root_p(this->MakeCopy());
    }

	inline Game1::Game1(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Game1::Game1(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game1::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->levels);
    }
    inline int Game1::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Game1::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->levels)) return r;
        return 0;
    }

    inline void Game1::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game1::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"levels\":", this->levels);
    }
    inline void Game1::CopyTo(Game1* const& o) const noexcept
    {
        o->levels = this->levels;
    }
    inline Game1* Game1::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game1>();
        this->CopyTo(o);
        return o;
    }
    inline Game1_p Game1::MakePtrCopy() const noexcept
    {
        return Game1_p(this->MakeCopy());
    }

	inline Game1_Level_Info::Game1_Level_Info(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Game1_Level_Info::Game1_Level_Info(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game1_Level_Info::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->minMoney);
    }
    inline int Game1_Level_Info::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level_Info::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        return 0;
    }

    inline void Game1_Level_Info::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1_Level_Info\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game1_Level_Info::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        s.Append(", \"minMoney\":", this->minMoney);
    }
    inline void Game1_Level_Info::CopyTo(Game1_Level_Info* const& o) const noexcept
    {
        o->id = this->id;
        o->minMoney = this->minMoney;
    }
    inline Game1_Level_Info* Game1_Level_Info::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game1_Level_Info>();
        this->CopyTo(o);
        return o;
    }
    inline Game1_Level_Info_p Game1_Level_Info::MakePtrCopy() const noexcept
    {
        return Game1_Level_Info_p(this->MakeCopy());
    }

	inline Game1_Level::Game1_Level(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Game1_Level::Game1_Level(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game1_Level::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->minMoney);
        bb.Write(this->desks);
    }
    inline int Game1_Level::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->desks)) return r;
        return 0;
    }

    inline void Game1_Level::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1_Level\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game1_Level::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        s.Append(", \"minMoney\":", this->minMoney);
        s.Append(", \"desks\":", this->desks);
    }
    inline void Game1_Level::CopyTo(Game1_Level* const& o) const noexcept
    {
        o->id = this->id;
        o->minMoney = this->minMoney;
        o->desks = this->desks;
    }
    inline Game1_Level* Game1_Level::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game1_Level>();
        this->CopyTo(o);
        return o;
    }
    inline Game1_Level_p Game1_Level::MakePtrCopy() const noexcept
    {
        return Game1_Level_p(this->MakeCopy());
    }

	inline Game1_Level_Desk::Game1_Level_Desk(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Game1_Level_Desk::Game1_Level_Desk(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game1_Level_Desk::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->players);
    }
    inline int Game1_Level_Desk::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level_Desk::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        return 0;
    }

    inline void Game1_Level_Desk::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby_Client.Game1_Level_Desk\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game1_Level_Desk::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        s.Append(", \"players\":", this->players);
    }
    inline void Game1_Level_Desk::CopyTo(Game1_Level_Desk* const& o) const noexcept
    {
        o->id = this->id;
        o->players = this->players;
    }
    inline Game1_Level_Desk* Game1_Level_Desk::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game1_Level_Desk>();
        this->CopyTo(o);
        return o;
    }
    inline Game1_Level_Desk_p Game1_Level_Desk::MakePtrCopy() const noexcept
    {
        return Game1_Level_Desk_p(this->MakeCopy());
    }

}
namespace Lobby
{
	inline Player::Player(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Player::Player(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Player::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->username);
        bb.Write(this->money);
        bb.Write(this->place);
        bb.Write(this->indexAtContainer);
        bb.Write(this->game1_Level_Desk_SeatIndex);
    }
    inline int Player::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Player::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        if (int r = bb.Read(this->money)) return r;
        if (int r = bb.Read(this->place)) return r;
        if (int r = bb.Read(this->indexAtContainer)) return r;
        if (int r = bb.Read(this->game1_Level_Desk_SeatIndex)) return r;
        return 0;
    }

    inline void Player::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby.Player\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Player::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        if (this->username) s.Append(", \"username\":\"", this->username, "\"");
        else s.Append(", \"username\":nil");
        s.Append(", \"money\":", this->money);
        s.Append(", \"place\":", this->place);
        s.Append(", \"indexAtContainer\":", this->indexAtContainer);
        s.Append(", \"game1_Level_Desk_SeatIndex\":", this->game1_Level_Desk_SeatIndex);
    }
    inline void Player::CopyTo(Player* const& o) const noexcept
    {
        o->id = this->id;
        o->username = this->username;
        o->money = this->money;
        o->place = this->place;
        o->indexAtContainer = this->indexAtContainer;
        o->game1_Level_Desk_SeatIndex = this->game1_Level_Desk_SeatIndex;
    }
    inline Player* Player::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Player>();
        this->CopyTo(o);
        return o;
    }
    inline Player_p Player::MakePtrCopy() const noexcept
    {
        return Player_p(this->MakeCopy());
    }

	inline Place::Place(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Place::Place(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Place::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->parent);
        bb.Write(this->players);
    }
    inline int Place::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Place::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->parent)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->players)) return r;
        return 0;
    }

    inline void Place::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby.Place\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Place::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"parent\":", this->parent);
        s.Append(", \"players\":", this->players);
    }
    inline void Place::CopyTo(Place* const& o) const noexcept
    {
        o->parent = this->parent;
        o->players = this->players;
    }
    inline Place* Place::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Place>();
        this->CopyTo(o);
        return o;
    }
    inline Place_p Place::MakePtrCopy() const noexcept
    {
        return Place_p(this->MakeCopy());
    }

	inline Root::Root(xx::MemPool* const& mempool) noexcept
        : PKG::Lobby::Place(mempool)
	{
	}
	inline Root::Root(xx::BBuffer* const& bb)
        : PKG::Lobby::Place(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Root::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->games);
    }
    inline int Root::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Root::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->games)) return r;
        return 0;
    }

    inline void Root::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby.Root\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Root::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"games\":", this->games);
    }
    inline void Root::CopyTo(Root* const& o) const noexcept
    {
        this->BaseType::CopyTo(o);
        o->games = this->games;
    }
    inline Root* Root::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Root>();
        this->CopyTo(o);
        return o;
    }
    inline Root_p Root::MakePtrCopy() const noexcept
    {
        return Root_p(this->MakeCopy());
    }

	inline Game::Game(xx::MemPool* const& mempool) noexcept
        : PKG::Lobby::Place(mempool)
	{
	}
	inline Game::Game(xx::BBuffer* const& bb)
        : PKG::Lobby::Place(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->id);
    }
    inline int Game::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        return 0;
    }

    inline void Game::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby.Game\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
    }
    inline void Game::CopyTo(Game* const& o) const noexcept
    {
        this->BaseType::CopyTo(o);
        o->id = this->id;
    }
    inline Game* Game::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game>();
        this->CopyTo(o);
        return o;
    }
    inline Game_p Game::MakePtrCopy() const noexcept
    {
        return Game_p(this->MakeCopy());
    }

	inline Game1::Game1(xx::MemPool* const& mempool) noexcept
        : PKG::Lobby::Game(mempool)
	{
	}
	inline Game1::Game1(xx::BBuffer* const& bb)
        : PKG::Lobby::Game(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game1::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->levels);
    }
    inline int Game1::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game1::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->levels)) return r;
        return 0;
    }

    inline void Game1::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby.Game1\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game1::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"levels\":", this->levels);
    }
    inline void Game1::CopyTo(Game1* const& o) const noexcept
    {
        this->BaseType::CopyTo(o);
        o->levels = this->levels;
    }
    inline Game1* Game1::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game1>();
        this->CopyTo(o);
        return o;
    }
    inline Game1_p Game1::MakePtrCopy() const noexcept
    {
        return Game1_p(this->MakeCopy());
    }

	inline Game1_Level::Game1_Level(xx::MemPool* const& mempool) noexcept
        : PKG::Lobby::Place(mempool)
	{
	}
	inline Game1_Level::Game1_Level(xx::BBuffer* const& bb)
        : PKG::Lobby::Place(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game1_Level::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->id);
        bb.Write(this->minMoney);
        bb.Write(this->desks);
    }
    inline int Game1_Level::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        if (int r = bb.Read(this->minMoney)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->desks)) return r;
        return 0;
    }

    inline void Game1_Level::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby.Game1_Level\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game1_Level::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        s.Append(", \"minMoney\":", this->minMoney);
        s.Append(", \"desks\":", this->desks);
    }
    inline void Game1_Level::CopyTo(Game1_Level* const& o) const noexcept
    {
        this->BaseType::CopyTo(o);
        o->id = this->id;
        o->minMoney = this->minMoney;
        o->desks = this->desks;
    }
    inline Game1_Level* Game1_Level::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game1_Level>();
        this->CopyTo(o);
        return o;
    }
    inline Game1_Level_p Game1_Level::MakePtrCopy() const noexcept
    {
        return Game1_Level_p(this->MakeCopy());
    }

	inline Game1_Level_Desk::Game1_Level_Desk(xx::MemPool* const& mempool) noexcept
        : PKG::Lobby::Place(mempool)
	{
	}
	inline Game1_Level_Desk::Game1_Level_Desk(xx::BBuffer* const& bb)
        : PKG::Lobby::Place(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Game1_Level_Desk::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        this->BaseType::ToBBuffer(bb);
        bb.Write(this->id);
    }
    inline int Game1_Level_Desk::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        if (int r = this->BaseType::FromBBuffer(bb)) return r;
        return this->FromBBufferCore(bb);
    }
    inline int Game1_Level_Desk::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        return 0;
    }

    inline void Game1_Level_Desk::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Lobby.Game1_Level_Desk\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Game1_Level_Desk::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
    }
    inline void Game1_Level_Desk::CopyTo(Game1_Level_Desk* const& o) const noexcept
    {
        this->BaseType::CopyTo(o);
        o->id = this->id;
    }
    inline Game1_Level_Desk* Game1_Level_Desk::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Game1_Level_Desk>();
        this->CopyTo(o);
        return o;
    }
    inline Game1_Level_Desk_p Game1_Level_Desk::MakePtrCopy() const noexcept
    {
        return Game1_Level_Desk_p(this->MakeCopy());
    }

}
namespace Login_DB
{
	inline GetAccount::GetAccount(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline GetAccount::GetAccount(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void GetAccount::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->username);
    }
    inline int GetAccount::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int GetAccount::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        return 0;
    }

    inline void GetAccount::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"Login_DB.GetAccount\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void GetAccount::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        if (this->username) s.Append(", \"username\":\"", this->username, "\"");
        else s.Append(", \"username\":nil");
    }
    inline void GetAccount::CopyTo(GetAccount* const& o) const noexcept
    {
        o->username = this->username;
    }
    inline GetAccount* GetAccount::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<GetAccount>();
        this->CopyTo(o);
        return o;
    }
    inline GetAccount_p GetAccount::MakePtrCopy() const noexcept
    {
        return GetAccount_p(this->MakeCopy());
    }

}
namespace DB
{
	inline Account::Account(xx::MemPool* const& mempool) noexcept
        : xx::Object(mempool)
	{
	}
	inline Account::Account(xx::BBuffer* const& bb)
        : xx::Object(bb)
	{
        if (int r = FromBBufferCore(*bb)) throw r;
	}
    inline void Account::ToBBuffer(xx::BBuffer& bb) const noexcept
    {
        bb.Write(this->id);
        bb.Write(this->username);
        bb.Write(this->password);
    }
    inline int Account::FromBBuffer(xx::BBuffer& bb) noexcept
    {
        return this->FromBBufferCore(bb);
    }
    inline int Account::FromBBufferCore(xx::BBuffer& bb) noexcept
    {
        if (int r = bb.Read(this->id)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->username)) return r;
        bb.readLengthLimit = 0;
        if (int r = bb.Read(this->password)) return r;
        return 0;
    }

    inline void Account::ToString(xx::String& s) const noexcept
    {
        if (this->memHeader().flags)
        {
        	s.Append("[ \"***** recursived *****\" ]");
        	return;
        }
        else this->memHeader().flags = 1;

        s.Append("{ \"pkgTypeName\":\"DB.Account\", \"pkgTypeId\":", xx::TypeId_v<ThisType>);
        ToStringCore(s);
        s.Append(" }");
        
        this->memHeader().flags = 0;
    }
    inline void Account::ToStringCore(xx::String& s) const noexcept
    {
        this->BaseType::ToStringCore(s);
        s.Append(", \"id\":", this->id);
        if (this->username) s.Append(", \"username\":\"", this->username, "\"");
        else s.Append(", \"username\":nil");
        if (this->password) s.Append(", \"password\":\"", this->password, "\"");
        else s.Append(", \"password\":nil");
    }
    inline void Account::CopyTo(Account* const& o) const noexcept
    {
        o->id = this->id;
        o->username = this->username;
        o->password = this->password;
    }
    inline Account* Account::MakeCopy() const noexcept
    {
        auto o = mempool->MPCreate<Account>();
        this->CopyTo(o);
        return o;
    }
    inline Account_p Account::MakePtrCopy() const noexcept
    {
        return Account_p(this->MakeCopy());
    }

}
}
namespace PKG
{
	inline void AllTypesRegister() noexcept
	{
        xx::MemPool::RegisterInternals();
	    xx::MemPool::Register<PKG::Success, xx::Object>();
	    xx::MemPool::Register<PKG::Error, xx::Object>();
	    xx::MemPool::Register<PKG::ConnInfo, xx::Object>();
	    xx::MemPool::Register<PKG::Collections, xx::Object>();
	    xx::MemPool::Register<xx::List<int32_t>, xx::Object>();
	    xx::MemPool::Register<xx::List<int64_t>, xx::Object>();
	    xx::MemPool::Register<xx::List<xx::String_p>, xx::Object>();
	    xx::MemPool::Register<xx::List<xx::Object_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Client_Login::Auth, xx::Object>();
	    xx::MemPool::Register<PKG::Server::Info, xx::Object>();
	    xx::MemPool::Register<PKG::Client_Lobby::Enter, xx::Object>();
	    xx::MemPool::Register<PKG::Client_Lobby::Enter_Game1, xx::Object>();
	    xx::MemPool::Register<PKG::Client_Lobby::Enter_Game1_Level, xx::Object>();
	    xx::MemPool::Register<PKG::Client_Lobby::Enter_Game1_Level_Desk, xx::Object>();
	    xx::MemPool::Register<PKG::Client_Lobby::Back, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby_Client::Self, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby_Client::Player, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby_Client::Root, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby_Client::Game1, xx::Object>();
	    xx::MemPool::Register<xx::List<PKG::Lobby_Client::Game1_Level_Info_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby_Client::Game1_Level_Info, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby_Client::Game1_Level, xx::Object>();
	    xx::MemPool::Register<xx::List<PKG::Lobby_Client::Game1_Level_Desk_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby_Client::Game1_Level_Desk, xx::Object>();
	    xx::MemPool::Register<xx::List<PKG::Lobby_Client::Player_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby::Player, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby::Place, xx::Object>();
	    xx::MemPool::Register<xx::List<PKG::Lobby::Player_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby::Root, PKG::Lobby::Place>();
	    xx::MemPool::Register<xx::List<PKG::Lobby::Game_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby::Game, PKG::Lobby::Place>();
	    xx::MemPool::Register<PKG::Lobby::Game1, PKG::Lobby::Game>();
	    xx::MemPool::Register<xx::List<PKG::Lobby::Game1_Level_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby::Game1_Level, PKG::Lobby::Place>();
	    xx::MemPool::Register<xx::List<PKG::Lobby::Game1_Level_Desk_p>, xx::Object>();
	    xx::MemPool::Register<PKG::Lobby::Game1_Level_Desk, PKG::Lobby::Place>();
	    xx::MemPool::Register<PKG::Login_DB::GetAccount, xx::Object>();
	    xx::MemPool::Register<PKG::DB::Account, xx::Object>();
	}
}
