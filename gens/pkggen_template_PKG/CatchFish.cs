#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    [AttachInclude, CustomInitCascade, Desc("场景")]
    class Scene
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

        [Desc("加减炮注跨度( coin )( 针对普通炮台 )")]
        long stepBet;

        [Desc("进出游戏时 money 自动兑换成 coin 要 乘除 的系数")]
        int exchangeCoinRatio;



        [Desc("帧编号, 每帧 + 1. 用于同步")]
        int frameNumber;

        [Desc("本地鱼生成专用随机数发生器")]
        xx.Random rnd;

        [Desc("自增id ( 从 1 开始, 用于填充 本地鱼 id )")]
        int autoIncId;


        [Desc("所有活鱼 ( 乱序 )")]
        List<Fish> fishs;

        [Desc("所有已创建非活鱼 ( 乱序 )")]
        List<Item> items;

        [Desc("所有鱼预约生成 ( 乱序 )")]
        List<FishBorn> borns;

        [Desc("当前关卡. endFrameNumber 到达时切换到下一关( clone from cfg.stages[(stage.id + 1) % cfg.stages.len] 并修正 各种 frameNumber )")]
        Stages.Stage stage;

        [Desc("空闲座位下标( 初始时填入 Sits.LeftBottom RightBottom LeftTop RightTop )")]
        List<Sits> freeSits;

        [Desc("所有玩家( 弱引用. 具体容器在 Scene 之外 )")]
        List<Weak<Player>> players;
    }

    [AttachInclude, Desc("场景元素的共通基类")]
    class Item
    {
        [Desc("标识码")]
        int id;

        [Desc("位于容器时的下标 ( 用于快速交换删除. 部分类型不一定用到 )")]
        int indexAtContainer;
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

    [AttachInclude, CustomInitCascade, Desc("玩家 ( 存在于服务 players 容器. 被 Scene.players 弱引用 )")]
    class Player : Item
    {
        [Desc("昵称 用于客户端显示 ( 填充自 db )")]
        string nickname;

        [Desc("头像id 用于客户端显示 ( 填充自 db )")]
        int avatar_id;


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


    [AttachInclude, CustomInitCascade, Desc("炮台基类. 下列属性适合大多数炮")]
    class Cannon : Item
    {
        [Desc("配置id")]
        int cfgId;

        [Desc("币值 / 倍率 ( 初始填充自 db. 玩家可调整数值. 范围限制为 Scene.minBet ~ maxBet )")]
        long coin;

        [Desc("炮管角度 ( 每次发射时都填充一下 )")]
        float angle;

        [Desc("所有子弹")]
        List<Bullet> bullets;
    }

    [Desc("子弹 & 鱼 & 武器 的基类")]
    class MoveItem : Item
    {
        [Desc("中心点坐标")]
        xx.Pos pos;

        [Desc("当前角度")]
        float angle;

        [Desc("每帧的直线移动坐标增量( 不一定用得上 )")]
        xx.Pos moveInc;
    }

    [AttachInclude, CustomInitCascade, Desc("子弹基类")]
    class Bullet : MoveItem
    {
        [Desc("金币 / 倍率( 记录炮台开火时的 Bet 值 )")]
        long coin;
    }

    [AttachInclude, CustomInitCascade, Desc("鱼基类( 支持每帧 pos += moveInc 简单移动 )")]
    class Fish : MoveItem
    {
        [Desc("配置id")]
        int cfgId;

        [Desc("币值 / 倍率")]
        long coin;

        [Desc("移动速度系数 ( 默认为 1 )")]
        float speedScale;

        [Desc("运行时缩放比例( 通常为 1 )")]
        float scale;

        [Desc("当前帧下标( 循环累加 )")]
        int spriteFrameIndex;

        [Desc("帧比值, 平时为 1, 如果为 0 则表示鱼不动( 比如实现冰冻效果 ), 帧图也不更新. 如果大于 1, 则需要在 1 帧内多次驱动该鱼( 比如实现快速离场的效果 )")]
        int frameRatio;
    }

    [AttachInclude, CustomInitCascade, Desc("武器基类 ( 有一些特殊鱼死后会变做 某种武器 / 炮台，死时有个滞空展示时间，被用于解决网络同步延迟。所有端应该在展示时间结束前收到该预约。展示完成后武器将飞向炮台变为附加炮台 )")]
    class Weapon : MoveItem
    {
        [Desc("配置id")]
        int cfgId;

        [Desc("开始起作用的帧编号( 和预约下发相关 )")]
        int beginFrameNumber;

        [Desc("币值 / 倍率( 填充自死鱼 )")]
        long coin;
    }

    [Desc("预约出鱼")]
    class FishBorn
    {
        [Desc("开始 / 生效帧编号")]
        int beginFrameNumber;

        [Desc("当 currentFrameNumber == beginFrameNumber 时，将 fish 放入 Scene.fishs 并自杀")]
        Fish fish;
    }

    [Desc("路点")]
    struct WayPoint
    {
        [Desc("坐标")]
        xx.Pos pos;

        [Desc("角度")]
        float angle;

        [Desc("当前点到下一个点的物理/逻辑距离( 下一个点可能是相同坐标, 停在原地转身的效果 )")]
        float distance;
    }

    [Desc("路径. 预约下发安全, 将复制路径完整数据")]
    class Way
    {
        [Desc("路点集合")]
        List<WayPoint> points;

        [Desc("总距离长度( sum( points[all].distance ). 如果非循环线, 不包含最后一个点的距离值. )")]
        float distance;

        [Desc("是否循环( 即移动到最后一个点之后又到第 1 个点, 永远走不完")]
        bool loop;
    }

    [AttachInclude, Desc("基于路径移动的鱼基类")]
    class WayFish : Fish
    {
        [Desc("移动路径. 动态生成, 不引用自 cfg. 同步时被复制. 如果该值为空, 则启用 wayTypeIndex / wayIndex")]
        Way way;

        [Desc("cfg.ways[wayTypeIndex]")]
        int wayTypeIndex;

        [Desc("cfg.ways[wayTypeIndex][wayIndex]")]
        int wayIndex;

        [Desc("当前路点下标")]
        int wayPointIndex;

        [Desc("当前路点上的已前进距离")]
        float wayPointDistance;

        [Desc("是否为在路径上倒着移动( 默认否 )")]
        bool reverse;
    }



    [AttachInclude, Desc("围绕目标鱼 圆周 旋转的小鱼( 实现自己的 Move 函数并附加几个计算参数, 被 BigFish Move 调用 )")]
    class RoundFish : Fish
    {
        [Desc("目标大鱼到当前小鱼的角度")]
        float tarAngle;
    }

    [AttachInclude, Desc("一只大鱼, 身边围了几只小鱼. 分摊伤害. 随机直线慢移. 自动再生. 切换关卡时快速逃离")]
    class BigFish : Fish
    {
        [Desc("围在身边的小鱼( Update, HitCheck 时级联处理 )")]
        List<RoundFish> childs;
    }


    [AttachInclude, Desc("色彩覆盖鱼, 打死后可能爆炸或得到某种武器/炮台. 切换炮台可能导致倍率基数发生变化( 如果支持倍率切换的话 )")]
    class ColorFish : Fish
    {
    }


    [AttachInclude, Desc("炸弹鱼( 红 )")]
    class BombFish : ColorFish
    {
    }

    [AttachInclude, Desc("狂暴鱼( 绿 )")]
    class FuryFish : ColorFish
    {
    }

    [AttachInclude, Desc("钻头鱼( 蓝 )")]
    class DrillFish : ColorFish
    {
    }

    [AttachInclude, Desc("闪电鱼( 白 )")]
    class LightFish : ColorFish
    {
    }


    [AttachInclude, CustomInitCascade, Desc("炸弹鱼武器")]
    class BombWeapon : Weapon
    {
    }

    [AttachInclude, CustomInitCascade, Desc("狂暴鱼武器")]
    class FuryWeapon : Weapon
    {
    }

    [AttachInclude, CustomInitCascade, Desc("钻头鱼武器")]
    class DrillWeapon : Weapon
    {
    }

    [AttachInclude, CustomInitCascade, Desc("闪电鱼武器")]
    class LightWeapon : Weapon
    {
    }


    [AttachInclude, Desc("狂暴炮台")]
    class FuryCannon : Cannon
    {
    }

    [AttachInclude, Desc("钻头炮台")]
    class DrillCannon : Cannon
    {
    }


    [AttachInclude, Desc("狂暴子弹")]
    class FuryBullet : Bullet
    {
    }

    [AttachInclude, Desc("钻头子弹")]
    class DrillBullet : Bullet
    {
    }
}
