public static class Weapons
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        var w1 = cfg.NewWeapon<PKG.CatchFish.Configs.Weapon>();
        //w1.cannon = 
        // todo
    }

    public static T NewWeapon<T>(this PKG.CatchFish.Configs.Config cfg) where T : PKG.CatchFish.Configs.Weapon, new()
    {
        var t = new T();
        t.id = cfg.weapons.dataLen;
        cfg.weapons.Add(t);
        return t;
    }
}
