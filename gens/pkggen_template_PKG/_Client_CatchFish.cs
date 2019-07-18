#pragma warning disable 0169, 0414
using TemplateLibrary;

// Client -> CatchFish
namespace Client_CatchFish
{
    [Desc("申请进入游戏. 成功返回 EnterSuccess. 失败直接被 T")]
    class Enter
    {
        [Desc("传递先前保存的 token 以便断线重连. 没有传空")]
        [Limit(64)]
        string token;
    }

    [Desc("调整炮台倍率")]
    class Bet
    {
        int cannonId;
        long coin;
    }

    [Desc("开火")]
    class Fire
    {
        int frameNumber;
        int cannonId;
        int bulletId;
        float angle;
        // todo: more
    }

    [Desc("碰撞检测")]
    class Hit
    {
        int cannonId;
        int bulletId;
        int fishId;
    }
}
