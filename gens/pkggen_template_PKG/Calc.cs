#pragma warning disable 0169, 0414
using TemplateLibrary;

// Calc
namespace Calc
{
    // 计算服务至少具备 总输入 & 输出 两项持久化累加值, 以便根据胜率曲线配型, 决策输赢. 
    // 控制的目的是令实际输赢尽量贴近设计的曲线, 达到可控范围内的大起大落效果.
    // 就曲线图来讲, x 值为总输入, y 值为胜率. 需要配置这样一张配置表( 机器人有另外一套胜率表 )
    // 当总输入 > 总输出 / 胜率 时, 差值将对胜率起到放大作用. 反之则缩小.
    // 长时间后最终盈利 = (输入 - 输出) + (机器人总得 - 机器人总押)

    namespace CatchFish
    {
        // 这里采用流式计算法, 省去两端组织 & 还原收发的数据. 每发生一次碰撞, 就产生一条 Hit 请求. 
        // 当一帧的收包 & 处理阶段结束后, 将产生的 Hit 队列打包发送给 Calc 服务计算.
        // Calc 依次处理, 以 fishId & playerId + bulletId 做 key, 逐个判断碰撞击打的成败. 
        // key: fishId, value: Hit 如果鱼还活着, 则值为 null, 鱼死则存当前 Hit 数据
        // key: pair<playerId, bulletId>, value: bulletCount 如果子弹数量有富余( 鱼死了, 没用完 ), 就会创建一行记录来累加
        // 最后将这两个字典的结果序列化返回

        [Desc("打击明细")]
        struct Hit
        {
            [Desc("鱼id 鱼主键1/1")]
            int fishId;

            [Desc("鱼币值")]
            long fishCoin;

            [Desc("玩家id 子弹主键1/2")]
            int playerId;

            [Desc("子弹id 子弹主键2/2")]
            int bulletId;

            [Desc("子弹数量( 炸弹, 强火力数量会超过 1 )")]
            int bulletCount;

            [Desc("子弹币值")]
            long bulletCoin;
        }

        [Desc("鱼被打死的明细")]
        struct Fish
        {
            int fishId;
            int playerId;
            int bulletId;
        }

        [Desc("子弹打空明细")]
        struct Bullet
        {
            int playerId;
            int bulletId;
            int bulletCount;
        }
    }

    // todo: more game namespaces here about shared totalIO
}
