public static class Stages
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        // 大量随机小鱼
        {
            var s = NewStage(cfg, 60 * 10);                             // 持续 10 sec
            {
                var e = NewElement<PKG.CatchFish.Stages.Emitter1>(s);
                e.cfg_bornTicksInterval = 2;                            // 1 秒刷 30 只
            }
        }
        // 少量随机小鱼 + 自动补大鱼
        {
            var s = NewStage(cfg, 60 * 60);                             // 持续 1 分钟
            {
                var e = NewElement<PKG.CatchFish.Stages.Emitter1>(s);
                e.cfg_bornTicksInterval = 20;                           // 1 秒刷 3 只
            }
            {
                var m = NewMonitor<PKG.CatchFish.Stages.Monitor1>(s);
                m.cfg_bornTicksInterval = 120;                          // 每隔 2 秒 1 只
                m.cfg_numFishsLimit = 2;                                // 一共 2 只
                m.cfg_bornDelayFrameNumber = 120;                       // 延迟 2 秒再生
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
