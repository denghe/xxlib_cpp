#pragma warning disable 0169, 0414
using TemplateLibrary;

// lobby server send to client
namespace Lobby_Client
{
    // 考虑到安全性, 数据流量, 无法直发 Lobby 命名空间下面的类数据. 下列类全是精简安全版.

    [Desc("玩家自己的数据")]
    class Player
    {
        [Desc("玩家id")]
        int id;

        [Desc("有多少钱")]
        double money;
    }

    [Desc("其他玩家的数据")]
    class OtherPlayer
    {
        [Desc("玩家id")]
        int id;

        [Desc("名字")]
        string username;
    }

    [Desc("大厅根部")]
    class Root
    {
        [Desc("当前玩家可见的游戏列表")]
        List<int> gameIds;

        [Desc("玩家自己的数据")]
        Player self;
    }

    [Desc("Game 特化: Game1 具体配置信息")]
    class Game1
    {
        [Desc("Game1 级别列表")]
        List<Game1_Level_Info> levels;
    }

    [Desc("Game1 级别的详细数据")]
    class Game1_Level_Info
    {
        [Desc("级别编号")]
        int id;

        [Desc("准入门槛")]
        double minMoney;
    }

    [Desc("Game1 级别的详细数据")]
    class Game1_Level
    {
        [Desc("级别编号")]
        int id;

        [Desc("准入门槛")]
        double minMoney;

        [Desc("该级别下所有桌子列表")]
        List<Game1_Level_Desk> desks;
    }

    [Desc("Game1 级别 下的 桌子 的详细数据")]
    class Game1_Level_Desk
    {
        [Desc("桌子编号")]
        int id;

        [Desc("玩家列表")]
        List<OtherPlayer> players;
    }
}
