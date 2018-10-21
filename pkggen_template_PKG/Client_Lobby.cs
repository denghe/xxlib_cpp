#pragma warning disable 0169, 0414
using TemplateLibrary;

// client send to lobby server
namespace Client_Lobby
{
    [Desc("首包. 进入大厅. 成功返回 Self( 含 Root 以及个人信息 ). 如果已经位于具体游戏中, 返回 ConnInfo. 失败立即被 T")]
    class Enter
    {
        [Limit(64)] string token;
    }

    [Desc("进入 Game1, 位于 Root 时可发送, 返回 Game1. 失败立即被 T")]
    class Enter_Game1
    {
    }

    [Desc("进入 Game1 某个 Level, 位于 Game1 时可发送, 返回 Game1_Level. 失败立即被 T")]
    class Enter_Game1_Level
    {
        [Desc("指定 Level id")]
        int id;
    }

}
