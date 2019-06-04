public static class Weapons
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        // 炸弹武器
        {
            var w = cfg.NewWeapon<PKG.CatchFish.Configs.Weapon>();
            w.txt = "Bomb";
            w.cannon = null;        // 无 cannon 对应
            w.distance = 0;         // 不移动
            w.showNumFrames = 120;  // 120 帧后爆炸. 同时也是预约时长
            w.explodeRadius = 300;  // 爆炸半径
            // 先忽略
            //w.frames
            //w.scale
            //w.zOrder
        }

        // todo
        // 闪电 狂暴 钻头
        //w1.cannon 指向 闪电 狂暴 钻头 cannon
    }

    public static T NewWeapon<T>(this PKG.CatchFish.Configs.Config cfg) where T : PKG.CatchFish.Configs.Weapon, new()
    {
        var t = new T();
        t.id = cfg.weapons.dataLen;
        cfg.weapons.Add(t);
        return t;
    }
}
