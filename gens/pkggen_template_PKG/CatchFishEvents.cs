#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    // 事件相关, 主用于下发广播, 不含依赖类
    namespace Events
    {
        [Desc("事件基类")]
        class EventBase
        {
            [Desc("事件相关玩家id")]
            int id;
        }

        [Desc("玩家进入")]
        class PlayerEnter : EventBase
        {
            // 下面大部分字段从 Player 类复制. 添加了部分初始数值, 可还原出玩家类实例.

            [Desc("昵称")]
            string nickname;

            [Desc("头像id")]
            int avatar_id;

            [Desc("破产标识")]
            bool dead;

            [Desc("剩余金币值")]
            long coin;

            [Desc("座位")]
            Sits sit;

            [Desc("炮台id")]
            int cannonId;

            [Desc("炮台配置id")]
            int cannonCfgId;

            [Desc("炮台币值")]
            long cannonCoin;
        }

        [Desc("玩家离开")]
        class PlayerLeave : EventBase { }

        [Desc("玩家开火锁定")]
        class FireLock : EventBase { }

        [Desc("玩家锁定后瞄准某鱼")]
        class FireAim : EventBase
        {
            [Desc("被瞄准的鱼id")]
            int fishId;
        }

        [Desc("玩家开火解除锁定")]
        class FireUnlock : EventBase { }

        [Desc("玩家自动开火")]
        class FireAutomating : EventBase { }

        [Desc("玩家解除自动开火")]
        class FireUnAutomating : EventBase { }

        [Desc("玩家开火( 单次 / 点射 )")]
        class Fire : EventBase
        {
            [Desc("起始帧编号 ( 来自客户端 )")]
            int frameNumber;

            // 炮台id

            [Desc("子弹id")]
            int bulletId;

            [Desc("子弹的发射目标坐标")]
            xx.Pos tarPos;

            [Desc("子弹的发射角度")]
            float tarAngle;

            [Desc("币值 / 倍率")]
            long coin;
        }

        [Desc("切换炮台")]
        class CannonSwitch : EventBase
        {
            [Desc("炮台配置id")]
            int cfgId;
        }

        [Desc("切换炮台倍率")]
        class CannonCoinChange : EventBase
        {
            [Desc("币值 / 倍率")]
            long coin;
        }

        [Desc("变更炮台角度")]
        class CannonAngleChange : EventBase
        {
            [Desc("角度")]
            float angle;
        }

        //[Desc("子弹命中( 与鱼死分离. 鱼死相关可能要等服务器跨线程回调送回结果才能下发 )")]
        //class BulletHit : EventBase
        //{
        //    [Desc("子弹流水号")]
        //    int bulletSerialNumber;
        //}

        [Desc("鱼被打死 ( 多次炸弹可继承这个类加个剩余次数 )")]
        class FishDead : EventBase
        {
            [Desc("鱼id")]
            int fishId;

            [Desc("子弹id")]
            int bulletId;

            [Desc("金币所得( fish.coin * bullet.coin 或 server 计算牵连鱼之后的综合结果 )")]
            long coin;

            [Desc("牵连的鱼")]
            List<FishDead> fishDeads;
        }

        }
        [Desc("玩家发射钻头弹")]
        class SendDrillBullet : EventBase
        {
            DrillBullet bullet;
        }

        [Desc("钻头弹炸死的鱼")]
        class DrillBoom : EventBase
        {
            int bulletSerialNumber;
            [Desc("爆炸总收益")]
            long coin;
            [Desc("钻头弹总收益")]
            long allCoin;
            [Desc("炸死的鱼的包")]
            List<FishDead> fishDeads;
        }
        [Desc("烈焰风暴游戏结算包")]
        class FireStormClear : EventBase
        {
            [Desc("总收益")]
            long allCoin;
            [Desc("剩余子弹收益")]
            long returnCoin;

        }

        [Desc("服务器根据某种事件 & 条件生成鱼下发")]
        class FishBorn
        {
            [Desc("鱼流水号( 用负数, 以便与自动生成的区分开来 )")]
            int fishSerialNumber;

            [Desc("鱼配置编号")]
            int cfgId;

            [Desc("鱼线配置编号")]
            int fishLineCfgId;

            [Desc("出生帧编号( 可能会比当前帧大, 相当于是批量预约下发出鱼 )")]
            int frameNumber;
        }
        [Desc("玩家进入破产状态")]
        class BankRuptcy : EventBase
        {
        }

        [Desc("玩家充值")]
        class PlayerCharge : EventBase
        {
            [Desc("新增币值")]
            long coin;
        }

    }
}
