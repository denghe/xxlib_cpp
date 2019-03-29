#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    namespace Config
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
            [Desc("基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模碰撞判定 )")]
            List<List<xx.Pos>> polygons;

            [Desc("首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )")]
            xx.Pos lockPoint;

            [Desc("锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )")]
            List<xx.Pos> lockPoints;

            [Desc("本帧动画切到下一帧动画后应该移动的距离( 受 Fish.speedScale 影响 )")]
            float moveDistance;

            //[Desc("指向 chipmunk 空间指针")]
            //CpSpacePointer cpSpace;
        }

        [Desc("配置基类")]
        class Base
        {
            [Desc("配置id")]
            int id;

            [Desc("名称")]
            string name;

            [Desc("放大系数( 影响各种判定, 坐标计算 )")]
            float scale;

            [Desc("初始z轴( 部分 boss 可能临时改变自己的 z )")]
            int zOrder;

            [Desc("帧集合 ( 用于贴图动态加载 / 卸载管理. 派生类所有帧都应该在此放一份 )")]
            List<SpriteFrameBase> frames;
        }

        [Desc("鱼配置基类 ( 派生类中不再包含 sprite frame 相关, 以便于资源加载管理扫描 )")]
        class Fish : Base
        {
            [Desc("金币 / 倍率随机范围 ( 最小值 )")]
            long minCoin;

            [Desc("金币 / 倍率随机范围 ( 最大值 )")]
            long maxCoin;

            [Desc("基于整个鱼的最大晃动范围的圆形碰撞检测半径( 粗判 )")]
            float maxDetectRadius;

            [Desc("移动帧集合 ( 部分鱼可能具有多种移动状态, 硬编码确定下标范围 )")]
            List<SpriteFrame> moveFrames;

            [Desc("鱼死帧集合")]
            List<SpriteFrameBase> dieFrames;

            [Desc("点选优先级说明参数, 越大越优先")]
            int touchRank;

            [Desc("影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.")]
            float shadowScale;

            [Desc("影子的偏移坐标")]
            xx.Pos shadowOffset;
        }

        [Desc("炮台 & 子弹配置基类")]
        class Cannon : Base
        {
            [Desc("炮管默认角度")]
            int angle;

            [Desc("炮口于座位坐标的距离 ( 适合大部分炮台 )")]
            float muzzleDistance;

            [Desc("拥有的数量( -1: 无限 )")]
            int bulletQuantity;

            [Desc("同屏颗数限制 ( 到达上限就不允许继续发射 )")]
            int numBulletLimit;

            [Desc("发射间隔帧数")]
            int shootCD;

            [Desc("帧集合 ( 包含炮身, 底座, 开火火焰, 子弹, 爆炸, 渔网等, 客户端显示代码自行硬编码定位 )")]
            List<SpriteFrameBase> frames;
        }

        // XxxxCannon

        [Desc("打爆部分特殊鱼出现的特殊武器配置基类")]
        class Weapon : Base
        {
            [Desc("每帧移动距离")]
            float distance;

            [Desc("展示时长 ( 帧数 )")]
            float showNumFrames;

            [Desc("飞到玩家坐标之后变化出来的炮台 cfg 之基类")]
            Cannon cannon;
        }

        [Desc("游戏配置信息( 配置信息并不会随着网络同步而下发, 反序列化后需要手工还原 )")]
        class Config
        {
            [Desc("所有鱼的配置信息")]
            List<Fish> fishs;

            [Desc("所有炮台的配置信息")]
            List<Cannon> cannons;

            [Desc("所有炮台的配置信息")]
            List<Weapon> weapons;

            [Desc("基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )")]
            List<xx.Pos> sitPositons;

            [Desc("锁定点击范围 ( 将枚举该范围内出现的鱼, 找出并选取 touchRank 最大值那个 )")]
            float aimTouchRadius;
        }
    }
}
