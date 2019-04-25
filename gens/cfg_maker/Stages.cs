public static class Stages
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        // 大量随机小鱼
        {
            var s = NewStage(cfg, 60 * 10);                             // 关卡持续时长帧数
            {
                var e = NewElement<PKG.CatchFish.Stages.Emitter_RandomFishs>(s);
                e.cfg_bornTicksInterval = 2;                            // 每隔多少帧刷 1 只
                e.cfg_coin = 1;
                e.cfg_scaleFrom = 1;                                    // 体积范围
                e.cfg_scaleTo = 2;
            }
        }
        // 圆环鱼阵 + 自动补大鱼
        {
            var s = NewStage(cfg, 60 * 60);
            {
                var e = NewElement<PKG.CatchFish.Stages.Emitter_RingFishs>(s);
                e.cfg_numFishsPerBatch = 36;                            // 每波产生多少只
                e.cfg_bornTicksInterval = 60;                           // 波与波之间生成帧间隔
                e.cfg_speed = 3;                                        // 每帧移动像素跨度
                e.cfg_scale = 1;                                        // 体积
                e.cfg_coin = 1;
            }
            {
                var m = NewMonitor<PKG.CatchFish.Stages.Monitor_KeepBigFish>(s);
                m.cfg_bornTicksInterval = 30;
                m.cfg_coin = 20;
                m.cfg_scaleFrom = 5;
                m.cfg_scaleTo = 8;
                m.cfg_numFishsLimit = 2;                                // 总只数限制
                m.cfg_bornDelayFrameNumber = 60;                        // 预约 xx 帧 后再生( 预约时间太短客户端容易掉线 )
            }
        }
        // 大量随机中鱼
        {
            var s = NewStage(cfg, 60 * 15);
            {
                var e = NewElement<PKG.CatchFish.Stages.Emitter_RandomFishs>(s);
                e.cfg_bornTicksInterval = 4;
                e.cfg_coin = 5;
                e.cfg_scaleFrom = 3;
                e.cfg_scaleTo = 5;
            }
        }
    }

    public static PKG.CatchFish.Stages.Stage NewStage(PKG.CatchFish.Configs.Config cfg, int cfg_endTicks)
    {
        var s = new PKG.CatchFish.Stages.Stage();
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
        s.elements.Add(e);
        e.cfg_beginTicks = cfg_beginTicks;
        return e;
    }

    public static T NewMonitor<T>(PKG.CatchFish.Stages.Stage s, int cfg_beginTicks = 0) where T : PKG.CatchFish.Stages.StageElement, new()
    {
        var m = new T();
        s.monitors.Add(m);
        m.cfg_beginTicks = cfg_beginTicks;
        return m;
    }
}
