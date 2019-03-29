#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFishConfig
{
    [Desc("帧动画基本信息")]
    class SpriteFrameBase
    {
        [Desc("贴图名. 通过遍历扫描去重之后, 结合关卡数据, 可以针对即将出现的鱼以及短期内不再出现的鱼做异步加载/卸载")]
        string textureName;

        [Desc("帧名")]
        string frameName;

        //[Desc("指向显示用精灵帧")]
        //SpriteFramePointer spriteFrame;
    }

    [Desc("帧动画")]
    class SpriteFrame : SpriteFrameBase
    {
        [Desc("基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模 )")]
        List<List<xx.Pos>> polygons;

        [Desc("首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )")]
        xx.Pos lockPoint;

        [Desc("锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )")]
        List<xx.Pos> lockPoints;

        [Desc("这一帧切到下一帧后应该移动的距离( 受 Fish.speedScale 影响 )")]
        float distance;

        //[Desc("指向 chipmunk 空间指针")]
        //CpSpacePointer cpSpace;
    }

    [Desc("鱼配置信息基类 ( 部分 boss 可能具有更多 frame, state 等配置参数 )")]
    class Fish
    {
        [Desc("配置id")]
        int id;

        [Desc("鱼名")]
        string name;

        [Desc("金币 / 倍率 ( 最小值 )")]
        long minCoin;

        [Desc("金币 / 倍率 ( 最大值 )")]
        long maxCoin;

        [Desc("基于整个鱼的最大晃动范围的圆形碰撞检测半径( 粗判 )")]
        float maxDetectRadius;

        [Desc("鱼移动帧集合")]
        List<SpriteFrame> moveFrames;

        [Desc("鱼死帧集合")]
        List<SpriteFrameBase> dieFrames;

        [Desc("显示放大系数. 创建精灵时先设起. 后面不用反复改")]
        float scale;

        [Desc("屏幕显示 z 轴( 决定显示覆盖顺序 )")]
        int zOrder;

        [Desc("点选优先级说明参数, 越大越优先")]
        int hitRank;

        [Desc("影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.")]
        float shadowScale;

        [Desc("影子的偏移坐标")]
        xx.Pos shadowOffset;
    }

    // todo: more 子弹 / 炮台 配置

    class Cannon
    {
        [Desc("配置id")]
        int id;

        [Desc("对应子弹的配置")]
        Bullet bullet;

        // todo: 炮台帧集合, 中轴于图片的坐标, 炮口于中轴的偏移量, 炮管初始位置等
    }

    class Bullet
    {
        [Desc("配置id")]
        int id;

        [Desc("开火火焰帧集合")]
        List<SpriteFrameBase> fireFrames;

        [Desc("子弹移动帧集合")]
        List<SpriteFrameBase> moveFrames;

        [Desc("子弹爆炸帧集合")]
        List<SpriteFrameBase> boomFrames;

        [Desc("渔网帧集合")]
        List<SpriteFrameBase> fishingNetsFrames;
    }

    [Desc("游戏配置信息( 配置信息并不会随着网络同步而下发, 反序列化后需要手工还原 )")]
    class Config
    {
        [Desc("所有鱼的配置信息")]
        List<Fish> fishs;

        [Desc("所有炮台的配置信息")]
        List<Cannon> cannons;

        [Desc("基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )")]
        List<xx.Pos> sitPositons;
    }
}
