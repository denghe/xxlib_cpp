public static class Stages
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        // 随机小鱼 + 炸弹
        {
            var s = NewStage(cfg, 60 * 60);                             // 关卡持续时长帧数
            {
                var e = NewElement<PKG.CatchFish.Stages.Emitter_RandomFishs>(s);
                e.cfg_bornTicksInterval = 20;                           // 每隔多少帧刷 1 只
                e.cfg_coin = 3;
                e.cfg_scaleFrom = 2;                                    // 体积范围
                e.cfg_scaleTo = 3;
            }
            {
                // 自动补炸弹
                var m = NewMonitor<PKG.CatchFish.Stages.Monitor_KeepBombFish>(s);
                m.cfg_bornTicksInterval = 0;
                m.cfg_numFishsLimit = 1;                                // 总只数限制
                m.cfg_bornDelayFrameNumber = 120;                       // 预约 xx 帧 后再生( 预约时间太短客户端容易掉线 )
                m.cfg_endTicks -= m.cfg_bornDelayFrameNumber;           // 监视器结束时间 应早于 关卡结束时间 - 预约延迟
            }
        }

        // BOSS: 大鱼周围环绕小鱼
        {
            var s = NewStage(cfg, 60 * 60);                             // 关卡持续时长帧数
            {
                // 自动补 BOSS
                var m = NewMonitor<PKG.CatchFish.Stages.Monitor_KeepBigFish>(s);
                m.cfg_bornTicksInterval = 30;
                m.cfg_numFishsLimit = 2;                                // 总只数限制
                m.cfg_bornDelayFrameNumber = 120;                       // 预约 xx 帧 后再生( 预约时间太短客户端容易掉线 )
                m.cfg_endTicks -= m.cfg_bornDelayFrameNumber;           // 监视器结束时间 应早于 关卡结束时间 - 预约延迟
            }
        }
        // 大量随机小鱼
        {
            var s = NewStage(cfg, 60 * 10);                             // 关卡持续时长帧数
            {
                var e = NewElement<PKG.CatchFish.Stages.Emitter_RandomFishs>(s);
                e.cfg_bornTicksInterval = 2;                            // 每隔多少帧刷 1 只
                e.cfg_coin = 2;
                e.cfg_scaleFrom = 2;                                    // 体积范围
                e.cfg_scaleTo = 3;
            }
        }
        // 组合
        {
            var s = NewStage(cfg, 60 * 60);
            {
                // 旋转出鱼--0
                var e1 = NewElement<PKG.CatchFish.Stages.Emitter_CircleFishs>(s);
                e1.cfg_endTicks /= 2;                                   // 停止运作时间为关卡时长的一半
                e1.cfg_angleBegin = 0;                                  // 起始角度
                e1.cfg_angleIncrease = (float)(System.Math.PI / 30);    // 每生成一只鱼的累加角度
                e1.cfg_bornTicksInterval = 3;                           // 两只鱼生成帧间隔
                e1.cfg_speed = 3;                                       // 每帧移动像素跨度
                e1.cfg_scale = 1;                                       // 体积
                e1.cfg_coin = 1;
                e1.angle = e1.cfg_angleBegin;
                {
                    // 旋转出鱼--180
                    var e1a = CloneElement(e1);
                    e1a.angle = e1a.cfg_angleBegin = (float)System.Math.PI;
                }

                // 圆环鱼阵--小鱼
                var e2 = NewElement<PKG.CatchFish.Stages.Emitter_RingFishs>(s);
                e2.cfg_beginTicks = e1.cfg_endTicks;                    // 生效时间：位于 旋转出鱼 结束之后
                e2.cfg_numFishsPerBatch = 36;                           // 每波产生多少只
                e2.cfg_bornTicksInterval = 60;                          // 波与波之间生成帧间隔
                e2.cfg_speed = 3;
                e2.cfg_scale = 1;
                e2.cfg_coin = 1;
                {
                    // 圆环鱼阵--中鱼( 与 圆环鱼阵--小鱼 交叠起作用 )
                    var e2a = CloneElement(e2);
                    e2a.cfg_beginTicks += e2.cfg_bornTicksInterval / 2;      // 生效时间：比 圆环鱼阵--小鱼 晚 波与波之间生成帧间隔 / 2
                    e2a.cfg_numFishsPerBatch = 18;
                    e2a.cfg_scale = 2;
                    e2a.cfg_coin = 2;
                }
            }
            {
                // 自动补肥鱼
                var m = NewMonitor<PKG.CatchFish.Stages.Monitor_KeepFatFish>(s);
                m.cfg_bornTicksInterval = 30;
                m.cfg_coin = 10;
                m.cfg_scaleFrom = 4;
                m.cfg_scaleTo = 5;
                m.cfg_numFishsLimit = 2;                                // 总只数限制
                m.cfg_bornDelayFrameNumber = 120;                       // 预约 xx 帧 后再生( 预约时间太短客户端容易掉线 )
                m.cfg_endTicks -= m.cfg_bornDelayFrameNumber;           // 监视器结束时间 应早于 关卡结束时间 - 预约延迟
            }
        }
    }

    public static PKG.CatchFish.Stages.Stage NewStage(PKG.CatchFish.Configs.Config cfg, int cfg_endTicks)
    {
        var s = new PKG.CatchFish.Stages.Stage();
        s.cfg = cfg;
        s.cfg_id = cfg.stages.dataLen;
        cfg.stages.Add(s);
        s.elements = new xx.List<PKG.CatchFish.Stages.StageElement>();
        s.monitors = new xx.List<PKG.CatchFish.Stages.StageElement>();
        s.cfg_endTicks = cfg_endTicks;
        s.ticks = -1;   // 方便前置 ++ 变成 0
        return s;
    }

    public static T NewElement<T>(PKG.CatchFish.Stages.Stage s, int cfg_beginTicks = 0) where T : PKG.CatchFish.Stages.StageElement, new()
    {
        var e = new T();
        e.owner = s;
        s.elements.Add(e);
        e.cfg_beginTicks = cfg_beginTicks;
        e.cfg_endTicks = s.cfg_endTicks;
        return e;
    }

    public static T NewMonitor<T>(PKG.CatchFish.Stages.Stage s, int cfg_beginTicks = 0) where T : PKG.CatchFish.Stages.StageElement, new()
    {
        var m = new T();
        m.owner = s;
        s.monitors.Add(m);
        m.cfg_beginTicks = cfg_beginTicks;
        m.cfg_endTicks = s.cfg_endTicks;
        return m;
    }

    public static xx.BBuffer bb = new xx.BBuffer();
    public static T CloneMonitor<T>(T tar) where T : PKG.CatchFish.Stages.StageElement
    {
        bb.Clear();
        bb.WriteRoot(tar);
        T m = default(T);
        bb.ReadRoot(ref m);
        m.owner = tar.owner;
        tar.owner.monitors.Add(m);
        return m;
    }
    public static T CloneElement<T>(T tar) where T : PKG.CatchFish.Stages.StageElement
    {
        bb.Clear();
        bb.WriteRoot(tar);
        T e = default(T);
        bb.ReadRoot(ref e);
        e.owner = tar.owner;
        tar.owner.elements.Add(e);
        return e;
    }
}
namespace PKG.CatchFish.Stages
{
    partial class StageElement
    {
        public Stage owner;
    }
    partial class Stage
    {
        public Configs.Config cfg;
    }
}
