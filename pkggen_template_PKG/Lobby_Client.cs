#pragma warning disable 0169, 0414
using TemplateLibrary;

// lobby server send to client
namespace Lobby_Client
{
    class PlayerInfo
    {
        int id;
        double money;
    }

    [Desc("大厅根部")]
    class Root
    {
        [Desc("当前玩家可见的游戏列表")]
        List<int> gameIds;
    }
}
