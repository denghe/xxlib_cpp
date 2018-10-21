#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace Lobby
{
    [Desc("玩家明细")]
    class Player
    {
        [Desc("玩家id")]
        int id;

        [Desc("名字")]
        string username;

        [Desc("有多少钱")]
        double money;

        [Desc("当前位置")]
        Place place;

        [Desc("位于 players 数组中的下标( 便于交换删除 )")]
        int itemIndex;

        [Desc("特化: 当位于 Game1_Level_Desk.players 之中时的座次附加信息")]
        int Game1_Level_Desk_SeatIndex;

        // ... more special
    }

    [Desc("玩家容器基类")]
    class Place
    {
        [Desc("指向上层容器( Root 没有上层容器 )")]
        Place parent;

        [Desc("玩家容器基类")]
        List<Player> players;
    }

    [Desc("大厅根部")]
    class Root : Place
    {
        [Desc("所有游戏列表")]
        List<Game> games;
    }

    [Desc("游戏基类")]
    class Game : Place
    {
        [Desc("游戏id")]
        int id;
    }

    [Desc("Game 特化: Game1 具体配置信息")]
    class Game1 : Game
    {
        [Desc("Game1 级别列表")]
        List<Game1_Level> levels;
    }

    [Desc("Game1 级别的详细数据")]
    class Game1_Level : Place
    {
        [Desc("级别编号")]
        int id;

        [Desc("准入门槛")]
        double minMoney;

        [Desc("该级别下所有桌子列表")]
        List<Game1_Level_Desk> desks;
    }

    [Desc("Game1 级别 下的 桌子 的详细数据")]
    class Game1_Level_Desk : Place
    {
        [Desc("桌子编号")]
        int id;
    }
}
