#pragma warning disable 0169, 0414
using TemplateLibrary;

// Calc
namespace Calc
{
    [Desc("需要持久化 / 进程启动时还原的数据上下文")]
    class Context
    {
        [Desc("总押注金额")]
        long totalInput;

        [Desc("总中奖金额")]
        long totalOutput;

        // todo: more
    }

    namespace CatchFish
    {
        [Desc("鱼")]
        class Fish
        {
            [Desc("主键1/1")]
            int id;

            [Desc("币值")]
            long coin;
        }

        [Desc("子弹( 单颗 )")]
        class Bullet
        {
            [Desc("主键1/2")]
            int playerId;
            [Desc("主键2/2")]
            int id;

            [Desc("数量")]
            long count;

            [Desc("币值")]
            long coin;
        }

        [Desc("1 鱼 n 弹 分组模型. 记录同一帧命中同一鱼的多颗子弹")]
        class Fish_Bullet_1_n
        {
            Fish fish;
            List<Bullet> bullets;
        }

        [Desc("判定结果类型")]
        enum ResultType : sbyte
        {
            [Desc("子弹打死了鱼，得到收益")]
            Die,
            [Desc("子弹没打死鱼，子弹消耗掉了")]
            NotDie,
            [Desc("子弹打空了，需要退还子弹 coin")]
            Miss
        }

        [Desc("判定结果")]
        class Result
        {
            [Desc("判定结果类型")]
            ResultType type;

            [Desc("鱼主键1/1")]
            int fishId;

            [Desc("子弹主键1/2")]
            int playerId;
            [Desc("子弹主键2/2")]
            int bulletId;
        }
    }

    // todo: more game namespaces here about shared totalIO
}
