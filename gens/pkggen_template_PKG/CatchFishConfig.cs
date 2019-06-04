#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    namespace Configs
    {
        [AttachInclude, CustomInitCascade, Desc("游戏配置主体")]
        class Config
        {
            [Desc("所有固定路径( 工具创建 )")]
            List<Way> fixedWays;

            [Desc("所有鱼的配置信息")]
            List<Fish> fishs;

            [Desc("所有炮台的配置信息")]
            List<Cannon> cannons;

            [Desc("所有武器的配置信息")]
            List<Weapon> weapons;

            [Desc("循环关卡数据( Scene 初次创建时，从 stages[0] clone. 可以在内存中 cache 序列化后的 binary )")]
            List<Stages.Stage> stages;

            [Desc("基于设计尺寸为 1280 x 720, 屏中心为 0,0 点的 4 个玩家炮台的坐标( 0: 左下  1: 右下    2: 右上  3: 左上 )")]
            List<xx.Pos> sitPositons;

            [Desc("锁定点击范围 ( 增加容错, 不必点的太精确. 点击作用是 枚举该范围内出现的鱼, 找出并选取 touchRank 最大值那个 )")]
            float aimTouchRadius;

            [Desc("普通鱼最大半径 ( 用于生成鱼线确保鱼出现时刚好位于屏幕外 )")]
            float normalFishMaxRadius;

            [Desc("显示非当前玩家子弹时是否启用追帧快进令其同步( 会导致高延迟玩家发射的子弹看上去离炮口有点远 )")]
            bool enableBulletFastForward;
        }

        [Desc("配置基类")]
        class Item
        {
            [Desc("内部编号. 通常等同于所在容器下标")]
            int id;

            [Desc("放大系数( 影响各种判定, 坐标计算 )")]
            float scale;

            [Desc("初始z轴( 部分 boss 可能临时改变自己的 z )")]
            int zOrder;

            [Desc("帧集合 ( 用于贴图动态加载 / 卸载管理. 派生类所有帧都应该在此放一份 )")]
            List<SpriteFrame> frames;
        }

        [Desc("鱼配置基类 ( 派生类中不再包含 sprite frame 相关, 以便于资源加载管理扫描 )")]
        class Fish : Item
        {
            [Desc("金币 / 倍率随机范围 ( 最小值 )")]
            long minCoin;

            [Desc("金币 / 倍率随机范围 ( 最大值 )")]
            long maxCoin;

            [Desc("基于整个鱼的最大晃动范围的圆形碰撞检测半径( 2 判. <= 0 则直接进行 3 判: 物理检测 )")]
            float maxDetectRadius;

            [Desc("必然命中的最小检测半径( 1 判. <= 0 则直接进行 2 判. 如果 bulletRadius + minDetectRadius > 子弹中心到鱼中心的距离 就认为命中 )")]
            float minDetectRadius;

            [Desc("移动帧集合 ( 部分鱼可能具有多种移动状态, 硬编码确定下标范围 )")]
            List<FishSpriteFrame> moveFrames;

            [Desc("鱼死帧集合")]
            List<SpriteFrame> dieFrames;

            [Desc("点选优先级说明参数, 越大越优先")]
            int touchRank;

            [Desc("影子显示时的放大系数. 平时与 scale 相等. 部分 boss 影子比身体小.")]
            float shadowScale;

            [Desc("影子的偏移坐标")]
            xx.Pos shadowOffset;
        }

        [Desc("炮台 & 子弹配置基类")]
        class Cannon : Item
        {
            [Desc("初始角度")]
            float angle;

            [Desc("炮管长度")]
            float muzzleLen;

            [Desc("拥有的数量( -1: 无限 )")]
            int quantity;

            [Desc("同屏颗数限制 ( 到达上限就不允许继续发射 )")]
            int numLimit;

            [Desc("发射间隔帧数")]
            int fireCD;

            [Desc("子弹检测半径")]
            int radius;

            [Desc("子弹最大 / 显示半径")]
            int maxRadius;

            [Desc("子弹每帧前进距离")]
            float distance;

            [Desc("是否开启子弹到屏幕边缘时反弹, false: 不反弹, true: 反弹")]
            bool enableBulletBounce;

            // 基类 frames 帧集合 ( 包含炮身, 底座, 开火火焰, 子弹, 爆炸, 渔网等, 客户端显示代码自行硬编码定位 )
        }

        [AttachInclude, CustomInitCascade, Desc("精灵帧")]
        class SpriteFrame
        {
            [Desc("plist资源名")]
            string plistName;

            [Desc("帧名")]
            string frameName;
        }


        [AttachInclude, CustomInitCascade, Desc("物理建模 for 鱼与子弹碰撞检测")]
        class Physics
        {
            [Desc("基于当前帧图的多边形碰撞顶点包围区( 由多个凸多边形组合而成, 用于物理建模碰撞判定 )")]
            List<List<xx.Pos>> polygons;
        }

        [Desc("带物理检测区和锁定线等附加数据的鱼移动帧动画")]
        class FishSpriteFrame
        {
            [Desc("指向精灵帧")]
            SpriteFrame frame;

            [Desc("指向物理建模")]
            Physics physics;

            [Desc("首选锁定点( 如果该点还在屏幕上, 则 lock 准星一直在其上 )")]
            xx.Pos lockPoint;

            [Desc("锁定点集合( 串成一条线的锁定点. 当首选锁定点不在屏上时, 使用该线与所在屏的边线的交点作为锁定点 )")]
            List<xx.Pos> lockPoints;

            [Desc("本帧动画切到下一帧动画后应该移动的距离( 受 Fish.speedScale 影响 )")]
            float moveDistance;

        }

        [Desc("小鱼环绕的大鱼的特殊配置")]
        class BigFish : Fish
        {
            [Desc("每帧移动距离")]
            float moveFrameDistance;

            [Desc("小鱼只数")]
            int numChilds;

            [Desc("小鱼前进角速度")]
            float childsAngleInc;
        }

        [Desc("彩色鱼特殊配置( 红: 炸弹  绿：狂暴  蓝：钻头  白: 闪电 )")]
        class ColorFish : Fish
        {
            [Desc("每帧移动距离")]
            float moveFrameDistance;

            [Desc("红色数值")]
            byte r;
            [Desc("绿色数值")]
            byte g;
            [Desc("蓝色数值")]
            byte b;

            [Desc("鱼死后变的 weapon( 根据这个来选择创建相应类型的 Fish )")]
            Weapon weapon;
        }

        [Desc("打爆彩色鱼出现的特殊武器配置基类")]
        class Weapon : Item
        {
            [Desc("展示文本( 为简化设计先这样 )")]
            string txt;

            [Desc("展示时长 ( 帧数 )")]
            int showNumFrames;

            [Desc("每帧移动距离")]
            float distance;

            [Desc("爆炸半径( for bomb, light... )")]
            float explodeRadius;

            [Desc("飞到玩家坐标之后变化出来的炮台 cfg 之基类")]
            Cannon cannon;
        }

        [Desc("狂暴炮台( 炮台打出数量有限的大威力子弹. 威力用每 Fire 子弹数量体现. 增加单发与鱼死亡检测次数, 显得更容易打死鱼 )")]
        class FuryCannon : Cannon
        {
            [Desc("打击次数( fireCount = coin / hitCount )")]
            int hitCount;
        }

        [Desc("钻头炮台( 穿刺 )")]
        class DrillCannon : Cannon
        {
            [Desc("碰撞CD: 限定一定时间范围内，子弹与鱼的碰撞检测次数。需要在子弹上背负 fishId : timeoutFN 白名单. 超时时间 = scene.FN + hitCD. 目标鱼不在名单内或 timeoutFN >= scene.FN 则可进行 hit. 同时记录到名单或刷新 timeoutFN")]
            int hitCD;

            [Desc("打击次数( 每次碰撞执行的 hit 次数. 消耗子弹相应的 coin. 如果剩余 coin 耗尽则消失 )")]
            int hitCount;
        }
    }
}
