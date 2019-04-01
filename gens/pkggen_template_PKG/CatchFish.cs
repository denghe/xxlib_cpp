#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    // 下列类中，Scene, Player 无法干净下发。

    [Desc("场景基础配置参数 ( 主要来自 db )")]
    class SceneConfig
    {
        [Desc("游戏id")]
        int gameId;

        [Desc("级别id")]
        int levelId;

        [Desc("房间id")]
        int roomId;

        [Desc("准入金")]
        double minMoney;

        [Desc("最低炮注( coin )( 针对普通炮台 )")]
        long minBet;

        [Desc("最高炮注( coin )( 针对普通炮台 )")]
        long maxBet;

        [Desc("进出游戏时 money 自动兑换成 coin 要 乘除 的系数")]
        int exchangeCoinRatio;

        [Desc("子弹颗数限制 ( 分别针对每个炮台 )")]
        long maxBulletsPerCannon;
    }

    [Desc("场景")]
    class Scene
    {
        [Desc("场景基础配置参数 ( 主要来自 db )")]
        SceneConfig cfg;

        [Desc("帧编号, 每帧 + 1. 用于同步")]
        int frameNumber;

        [Desc("本地鱼生成专用随机数发生器")]
        xx.Random rnd;

        [Desc("自增id ( 从 1 开始, 用于填充 本地鱼 id )")]
        int autoIncId;

        //[Desc("自减id ( 从 -1 开始, 用于服务器下发鱼生成 )")]
        //int autoDecId;

        [Desc("所有活鱼 ( 乱序 )")]
        List<Fish> fishs;

        [Desc("所有已创建非活鱼 ( 乱序 )")]
        List<Item> items;

        [Desc("所有鱼预约生成 ( 乱序 )")]
        List<FishBorn> borns;

        [Desc("当前关卡. endFrameNumber 到达时切换到下一关( clone from cfg.stages[(stage.id + 1) % cfg.stages.len] 并修正 各种 frameNumber )")]
        Timers.Stage stage;

        [Desc("空闲座位下标( 初始时填入 Sits.LeftBottom RightBottom LeftTop RightTop )")]
        List<Sits> freeSits;

        [Desc("所有玩家")]
        List<Weak<Player>> players;
    }

    [Desc("座位列表")]
    enum Sits
    {
        [Desc("左下")]
        LeftBottom,

        [Desc("右下")]
        RightBottom,

        [Desc("右上")]
        RightTop,

        [Desc("左上")]
        LeftTop,
    }

    [Desc("炮台基类. 下列属性适合大多数炮")]
    class Cannon
    {
        [Desc("炮台id")]
        int id;

        [Desc("配置id")]
        int cfgId;

        [Desc("币值 / 倍率 ( 初始填充自 db. 玩家可调整数值. 范围限制为 Scene.minBet ~ maxBet )")]
        long coin;

        [Desc("炮管角度 ( 每次发射时都填充一下 )")]
        float angle;

        [Desc("所有子弹")]
        List<Bullet> bullets;
    }

    [Desc("玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )")]
    class Player
    {
        //[Desc("所在场景")]
        //Weak<Scene> scene;

        [Desc("账号id. 用于定位玩家 ( 填充自 db )")]
        int id;

        [Desc("昵称 用于客户端显示 ( 填充自 db )")]
        string nickname;

        [Desc("头像id 用于客户端显示 ( 填充自 db )")]
        int avatar_id;

        //[Desc("当 Client 通过 Lobby 服务到 Game 发 Enter 时, Game 需要生成一个 token 以便 Client Enter 时传入以校验身份")]
        //string token;

        //[Desc("开炮等行为花掉的金币数汇总 ( 统计 )")]
        //long consumeCoin;

        [Desc("破产标识 ( 每帧检测一次总资产是否为 0, 是就标记之. 总资产包括 coin, 已爆出的 weapons, 已获得的附加炮台, 飞行中的 bullets )")]
        bool noMoney;

        [Desc("剩余金币值( 不代表玩家总资产 ). 当玩家进入到游戏时, 该值填充 money * exchangeCoinRatio. 玩家退出时, 做除法还原为 money.")]
        long coin;

        [Desc("座位")]
        Sits sit;

        [Desc("自动锁定状态")]
        bool autoLock;

        [Desc("自动开火状态")]
        bool autoFire;

        [Desc("锁定瞄准的鱼")]
        Weak<Fish> aimFish;

        [Desc("自增id ( 从 1 开始, 用于填充 炮台, 子弹 id )")]
        int autoIncId;

        [Desc("炮台堆栈 ( 例如: 常规炮 打到 钻头, 钻头飞向玩家变为 钻头炮, 覆盖在常规炮上 )")]
        List<Cannon> cannons;

        [Desc("武器集合 ( 被打死的特殊鱼转为武器对象, 飞向玩家, 变炮消失前都在这里 )")]
        List<Weapon> weapons;
    }

    [Desc("场景元素的共通基类")]
    class Item
    {
        [Desc("自增id ( 服务器实时下发的id为负 )")]
        int id;

        [Desc("位于容器时的下标 ( 用于快速交换删除 )")]
        int indexAtContainer;
    }

    [Desc("子弹 & 鱼 & 武器 的基类")]
    class MoveItem : Item
    {
        [Desc("中心点坐标")]
        xx.Pos pos;

        [Desc("当前角度")]
        float angle;

        // FishLine
    }

    [Desc("子弹基类")]
    class Bullet : MoveItem
    {
        [Desc("每帧的直线移动坐标增量( 60fps )")]
        xx.Pos moveInc;

        [Desc("金币 / 倍率( 记录炮台开火时的 Bet 值 )")]
        long coin;
    }

    [Desc("鱼基类 ( 下列属性适合大多数鱼, 不一定适合部分 boss )")]
    class Fish : MoveItem
    {
        [Desc("配置id")]
        int cfgId;

        [Desc("币值 / 倍率")]
        long coin;

        [Desc("移动速度系数 ( 默认为 1 )")]
        float speedScale;

        [Desc("碰撞 | 显示 体积系数 ( 默认为 1 )")]
        float sizeScale;
    }

    [Desc("武器基类 ( 有一些特殊鱼死后会变做 某种武器 / 炮台，死时有个滞空展示时间，被用于解决网络同步延迟。所有端应该在展示时间结束前收到该预约。展示完成后武器将飞向炮台变为附加炮台 )")]
    class Weapon : MoveItem
    {
        [Desc("配置id")]
        int cfgId;

        [Desc("开始飞行的帧编号")]
        int flyFrameNumber;
    }


    [Desc("定时器基类")]
    class Timer
    {
        [Desc("开始 / 生效帧编号")]
        int beginFrameNumber;
    }

    [Desc("预约出鱼")]
    class FishBorn : Timer
    {
        [Desc("当 currentFrameNumber == beginFrameNumber 时，将 fish 放入 Scene.fishs 并自杀")]
        Fish fish;
    }

}
