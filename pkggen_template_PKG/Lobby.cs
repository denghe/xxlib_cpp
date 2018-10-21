#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace Lobby
{
    [Desc("游戏基类")]
    class Game
    {
        int id;
    }

    [Desc("大厅根部")]
    class Root
    {
        [Desc("所有游戏列表")]
        List<Game> games;
    }
}
