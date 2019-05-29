using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public static class Cannons
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        {
            var c = cfg.NewCannon<PKG.CatchFish.Configs.Cannon>();
            c.frames = new xx.List<PKG.CatchFish.Configs.SpriteFrame>();
            c.frames.Add(GetSpriteFrame("pao_01"));
            c.frames.Add(GetSpriteFrame("zidan_01"));
            c.frames.Add(GetSpriteFrame("yuwang_1"));
            c.angle = (float)(90.0 * Math.PI / 180.0);
            c.quantity = -1;
            c.muzzleLen = 200;
            c.numLimit = 30;
            c.scale = 1f;
            c.fireCD = 6;
            c.zOrder = 100;
            c.radius = 6;
            c.maxRadius = 60;
            c.distance = 720 / 30;  //720 / 60;  // 1秒钟上下穿越屏幕?
            c.enableBulletBounce = true;
        }

        // todo: fury, drill
    }

    public static T NewCannon<T>(this PKG.CatchFish.Configs.Config cfg) where T : PKG.CatchFish.Configs.Cannon, new()
    {
        var t = new T();
        t.id = cfg.cannons.dataLen;
        cfg.cannons.Add(t);
        return t;
    }

    public static PKG.CatchFish.Configs.SpriteFrame GetSpriteFrame(string fn)
    {
        if (!Program.frames.ContainsKey(fn)) throw new System.Exception("frame not found:" + fn);
        return Program.frames[fn];
    }
}
