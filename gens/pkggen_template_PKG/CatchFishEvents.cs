#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    // 服务器发出的数据分为下列几种性质：
    // 1. 预约. 即将发生的确切事件，如: 鱼的服务器端产生预约生成，炸弹被点燃, 未来某帧爆炸， 如果在收到包时, 本地已超过预约时间点, 则需要重连新同步
    // 2. 通知. 已经发生的既定事实，如: 玩家进出，服务器端计算类得出结论( 像是钻头窜死鱼 ), 或是玩家汇报子弹碰到了鱼, 如果 server 发现该子弹和鱼还活着, 并且鱼当前状态可被命中, 并且子弹和鱼的距离非常接近( 可关闭该检测 ), 并且根据鱼死亡算法得出鱼被打死的结论, 就下发鱼死通知( 状态改变通知属于严格同步 )
    //          再如: 玩家发射钻头, 本地并不立刻射出去, server 判断超时必须射, 或是在收到发射指令并确认后下发通知, 本地收到通知后再射. 起始帧以 server 收到时为准.
    // 3. 转发. 其他玩家的操作模拟，如: 所有玩家切倍率炮台，发射一般子弹, 本地可于发包同时发射子弹, server 收到发射指令后信任客户端填充的发射时帧编号, 做合法判断后创建并追帧, 不做鱼碰撞检测， 只模拟飞行（为了完整同步时看起来自然）. 同时, 将该指令打包转发给所有玩家( 自己收到后忽略 )
    // 4. 推送. 帧同步数据, 心跳数据等

    // 注: 机器人部分功能依赖服务器端, 上面的部分例子变得不适合。在服务器端开启机器人发射的子弹和鱼的碰撞检测前提下，机器人自己不再模拟子弹飞行，不再做碰撞检测，不收 预约、通知、转发 数据，机器人的指令在服务器收到后经确认有效，生效后再转发给真实玩家。
    // 服务器知道某玩家是机器人( 账号有标志位 )，每几帧( 贴合某种炮发射频率 )向机器人 以请求的方式 发送所有能击打的鱼的分布图( fishId, cfgId, pos 数组 ), 机器人做空间检索，模拟玩家开火、锁定、切倍率、上下线等
    // 机器人没有帧逻辑。收到请求才开始计算并立刻返回处理结果。可以将机器人想象为是动态连接到服务器的另外一个服务器。

    // 注2：关于关卡（即同步到本地运行的鱼发生器），理论上讲是可以完全没有的。server 上来一发城主逻辑，所有鱼都通过预约下发生成。当然，要做到效果完整，还需对场景互动元素/特效等进一步规划，也走预约下发生成。

    // 事件相关, 主用于下发广播, 不含依赖类
    // 每帧结束时合并打包下发。合并后的包含有本帧编号。每个事件含有相关玩家id
    namespace Events
    {
        [Desc("事件基类")]
        class Event
        {
            [Desc("相关玩家id")]
            int playerId;
        }

        [Desc("通知: 玩家进入. 大部分字段从 Player 类复制. 添加了部分初始数值, 可还原出玩家类实例.")]
        class Enter : Event
        {
            [Desc("昵称")]
            string nickname;

            [Desc("头像id")]
            int avatar_id;

            [Desc("破产标识")]
            bool noMoney;

            [Desc("剩余金币值")]
            long coin;

            [Desc("座位")]
            Sits sit;

            [Desc("炮台配置id")]
            int cannonCfgId;

            [Desc("炮台币值")]
            long cannonCoin;
        }

        [Desc("通知: 玩家离开")]
        class Leave : Event { }

        [Desc("通知: 玩家破产")]
        class NoMoney : Event { }

        [Desc("通知: 退钱( 常见于子弹并发打中某鱼产生 miss 或鱼id未找到 或子弹生命周期结束 )")]
        class Refund : Event
        {
            [Desc("退款金额( coin * count )")]
            long coin;
        }


        [Desc("通知: 鱼被打死")]
        class FishDead : Event
        {
            [Desc("武器id( 非 0 则鱼被 weapon 打死. 为 0 则鱼被 cannon bullet 打死 )")]
            int weaponId;

            [Desc("炮台id")]
            int cannonId;

            [Desc("子弹id")]
            int bulletId;

            [Desc("金币总收入( fishs.coin * bullet.coin + left bullet coin )")]
            long coin;

            [Desc("死鱼id列表")]
            List<int> ids;
        }

        // todo: 下发 服务器创建的 bullet. 例如炸弹鱼变的. 含有 cannonId, bulletId, pos, coin. 如果炸死鱼, 将下发 FishDead, 指向该 bullet.
        //class PushBullet : Event
        //{
        //    [Desc("已于 server 端构造好的, 无牵挂的, 能干净下发的实例")]
        //    Bullet bullet;
        //}

        // todo: 预约: 炸弹爆炸

        [Desc("通知: 下发已生效 Weapon, 需要判断 beginFrameNumber, 放入 player.weapon 队列, 令 fishId 的鱼死掉")]
        class PushWeapon : Event
        {
            [Desc("死鱼id")]
            int fishId;

            [Desc("已于 server 端构造好的, 无牵挂的, 能干净下发的实例")]
            Weapon weapon;
        }

        [Desc("预约: 出鱼( 需判定 beginFrameNumber ), 放入 scene.borns 队列. 用不到 playerId")]
        class PushFish : Event
        {
            [Desc("已于 server 端构造好的, 无牵挂的, 能干净下发的实例")]
            FishBorn born;
        }

        [Desc("转发: 开启开火锁定")]
        class OpenAutoLock : Event { }

        [Desc("转发: 玩家锁定后瞄准某鱼")]
        class Aim : Event
        {
            [Desc("被瞄准的鱼id")]
            int fishId;
        }

        [Desc("转发: 玩家开火解除锁定")]
        class CloseAutoLock : Event { }

        [Desc("转发: 玩家自动开火")]
        class OpenAutoFire : Event { }

        [Desc("转发: 玩家解除自动开火")]
        class CloseAutoFire : Event { }

        [Desc("转发: 发子弹( 单次 ). 非特殊子弹, 只可能是 cannons[0] 原始炮台发射")]
        class Fire : Event
        {
            [Desc("起始帧编号 ( 来自客户端 )")]
            int frameNumber;

            [Desc("炮台id")]
            int cannonId;

            [Desc("子弹id")]
            int bulletId;

            [Desc("发射角度")]
            float angle;
        }

        [Desc("转发: 切换炮台")]
        class CannonSwitch : Event
        {
            [Desc("炮台配置id")]
            int cfgId;
        }

        [Desc("转发: 切换炮台倍率")]
        class CannonCoinChange : Event
        {
            [Desc("炮台id")]
            int cannonId;

            [Desc("币值 / 倍率")]
            long coin;
        }




        [Desc("调试信息( 开发阶段校验用 )")]
        class DebugInfo : Event
        {
            [Desc("鱼id集合")]
            List<int> fishIds;

            // todo: more
        }

    }
}
