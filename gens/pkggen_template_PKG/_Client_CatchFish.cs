#pragma warning disable 0169, 0414
using TemplateLibrary;

// Client -> Server
namespace Client_CatchFish
{
    [Desc("申请进入游戏. 成功返回 EnterSuccess. 失败直接被 T")]
    class Enter
    {
    }

    [Desc("开火")]
    class Shoot
    {
        int frameNumber;
        int cannonId;
        int bulletId;
        xx.Pos pos;
    }

    [Desc("碰撞检测")]
    class Hit
    {
        int cannonId;
        int bulletId;
        int fishId;
    }
}
