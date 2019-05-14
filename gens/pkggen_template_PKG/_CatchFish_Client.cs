#pragma warning disable 0169, 0414
using TemplateLibrary;

// CatchFish -> Client
namespace CatchFish_Client
{
    [Desc("申请进入游戏 成功")]
    class EnterSuccess
    {
        [Desc("完整的游戏场景")]
        CatchFish.Scene scene;

        [Desc("玩家强引用容器")]
        List<CatchFish.Player> players;

        [Desc("指向当前玩家")]
        Weak<CatchFish.Player> self;

        [Desc("当前 token( 为简化设计先放这. 正常情况下是前置服务告知 )")]
        string token;
    }

    [Desc("帧事件同步包")]
    class FrameEvents
    {
        [Desc("帧编号")]
        int frameNumber;

        [Desc("帧事件集合")]
        List<CatchFish.Events.Event> events;
    }
}
