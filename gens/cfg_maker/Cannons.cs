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
            var c = new PKG.CatchFish.Configs.Cannon();
            c.id = cfg.cannons.dataLen;
            cfg.cannons.Add(c);

            c.angle = 90;
            c.bulletQuantity = -1;
            c.frames = new xx.List<PKG.CatchFish.Configs.SpriteFrame>();
            c.frames.Add(GetSpriteFrame("pao_01"));
            c.frames.Add(GetSpriteFrame("zidan_01"));
            c.frames.Add(GetSpriteFrame("yuwang_1"));
            c.muzzleDistance = 100;
            c.numBulletLimit = 2;
            c.scale = 1;
            c.shootCD = 15;
            c.zOrder = 100;
            c.bulletRadius = 15;
        }
    }


    public static PKG.CatchFish.Configs.SpriteFrame GetSpriteFrame(string fn)
    {
        if (!Program.frames.ContainsKey(fn)) throw new System.Exception("frame not found:" + fn);
        return Program.frames[fn];
    }
}
