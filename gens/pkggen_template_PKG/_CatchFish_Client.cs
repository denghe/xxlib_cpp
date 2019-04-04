#pragma warning disable 0169, 0414
using TemplateLibrary;

// Server -> Client
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
