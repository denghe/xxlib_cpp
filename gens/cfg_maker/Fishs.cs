using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public static class Fishs
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        // 一般 / 通用鱼
        var f1 = cfg.NewFish<PKG.CatchFish.Configs.Fish>();
        f1.minCoin = 1;
        f1.maxCoin = 3;
        f1.minDetectRadius = 6;
        f1.maxDetectRadius = 32;
        f1.scale = 1;
        f1.zOrder = 3;
        f1.shadowOffset = new xx.Pos { x = 5, y = 5 };
        f1.shadowScale = 1;
        FillFishFrames(f1, "xiaochouyu_move", 30, "xiaochouyu_die", 10, 2.5f);

        // 小鱼环绕鱼
        {
            var f2 = cfg.NewFish<PKG.CatchFish.Configs.BigFish>();
            f2.minCoin = 50;
            f2.maxCoin = 50;
            f2.minDetectRadius = f1.minDetectRadius;
            f2.maxDetectRadius = f1.maxDetectRadius;
            f2.scale = 6;
            f2.zOrder = 4;
            f2.shadowOffset = new xx.Pos { x = 10, y = 10 };
            f2.shadowScale = f1.shadowScale;
            f2.frames = f1.frames;
            f2.moveFrames = f1.moveFrames;
            f2.dieFrames = f1.dieFrames;

            f2.moveFrameDistance = 2.5f;
            f2.numChilds = 16;
            f2.childsAngleInc = (float)(Math.PI / 90.0);
        }

        // 炸弹鱼
        {
            var f3 = cfg.NewFish<PKG.CatchFish.Configs.BombFish>();
            f3.minCoin = 100;
            f3.maxCoin = 100;
            f3.minDetectRadius = f1.minDetectRadius;
            f3.maxDetectRadius = f1.maxDetectRadius;
            f3.scale = 7;
            f3.zOrder = 5;
            f3.shadowOffset = new xx.Pos { x = 12, y = 12 };
            f3.shadowScale = f1.shadowScale;
            f3.frames = f1.frames;
            f3.moveFrames = f1.moveFrames;
            f3.dieFrames = f1.dieFrames;

            f3.moveFrameDistance = 0.8f;
            f3.weapon = null;
            f3.explodeRadius = 300;
            f3.r = 255;
            f3.g = 0;
            f3.b = 0;
        }

        // todo
        //// 狂暴鱼
        //{
        //    var f4 = cfg.NewFish<PKG.CatchFish.Configs.ColorFish>();
        //    f4.minCoin = 500;
        //    f4.maxCoin = 500;
        //    f4.minDetectRadius = f1.minDetectRadius;
        //    f4.maxDetectRadius = f1.maxDetectRadius;
        //    f4.scale = 7;
        //    f4.zOrder = 5;
        //    f4.shadowOffset = new xx.Pos { x = 12, y = 12 };
        //    f4.shadowScale = f1.shadowScale;
        //    f4.frames = f1.frames;
        //    f4.moveFrames = f1.moveFrames;
        //    f4.dieFrames = f1.dieFrames;

        //    f4.weapon = null;   // todo: 去 weapons 定位
        //    f4.r = 0;
        //    f4.g = 255;
        //    f4.b = 0;
        //}

        //// 钻头鱼
        //{
        //    var f5 = cfg.NewFish<PKG.CatchFish.Configs.ColorFish>();
        //    f5.minCoin = 500;
        //    f5.maxCoin = 500;
        //    f5.minDetectRadius = f1.minDetectRadius;
        //    f5.maxDetectRadius = f1.maxDetectRadius;
        //    f5.scale = 7;
        //    f5.zOrder = 5;
        //    f5.shadowOffset = new xx.Pos { x = 12, y = 12 };
        //    f5.shadowScale = f1.shadowScale;
        //    f5.frames = f1.frames;
        //    f5.moveFrames = f1.moveFrames;
        //    f5.dieFrames = f1.dieFrames;

        //    f5.weapon = null;   // todo: 去 weapons 定位
        //    f5.r = 0;
        //    f5.g = 0;
        //    f5.b = 255;
        //}
    }


    public static T NewFish<T>(this PKG.CatchFish.Configs.Config cfg) where T : PKG.CatchFish.Configs.Fish, new()
    {
        var f = new T();
        f.id = cfg.fishs.dataLen;
        cfg.fishs.Add(f);
        return f;
    }

    public static string GetFrameName(string name, int i)
    {
        return name + " (" + i + ")";
    }


    public static PKG.CatchFish.Configs.FishSpriteFrame GetFishSpriteFrame(string fn)
    {
        if (!Program.fishFrames.ContainsKey(fn)) throw new System.Exception("fish frame not found:" + fn);
        return Program.fishFrames[fn];
    }
    public static PKG.CatchFish.Configs.FishSpriteFrame GetFishSpriteFrame(string name, int i)
    {
        return GetFishSpriteFrame(GetFrameName(name, i));
    }



    public static PKG.CatchFish.Configs.SpriteFrame GetSpriteFrame(string fn)
    {
        if (!Program.frames.ContainsKey(fn)) throw new System.Exception("frame not found:" + fn);
        return Program.frames[fn];
    }
    public static PKG.CatchFish.Configs.SpriteFrame GetSpriteFrame(string name, int i)
    {
        return GetSpriteFrame(GetFrameName(name, i));
    }


    public static void FillFishFrames(PKG.CatchFish.Configs.Fish f, string moveName, int moveFrameCount, string dieName = null, int dieFrameCount = 0, float moveDistance = 2.5f)
    {
        f.frames = new xx.List<PKG.CatchFish.Configs.SpriteFrame>();
        f.moveFrames = new xx.List<PKG.CatchFish.Configs.FishSpriteFrame>();
        f.dieFrames = new xx.List<PKG.CatchFish.Configs.SpriteFrame>();

        var fn = GetFrameName(moveName, 1);
        var firstFrame = GetFishSpriteFrame(fn);
        if (firstFrame.physics == null) throw new Exception("frame's physics == null:" + fn);
        if (firstFrame.lockPoints == null) throw new Exception("frame's lockPoints == null:" + fn);

        for (int i = 1; i <= moveFrameCount; ++i)
        {
            var fishFrame = GetFishSpriteFrame(moveName, i);
            f.moveFrames.Add(fishFrame);
            f.frames.Add(fishFrame.frame);
            if (i > 1)                                      // 复用第一帧的数据
            {
                if (fishFrame.lockPoints == null)
                {
                    fishFrame.lockPoints = firstFrame.lockPoints;
                    fishFrame.lockPoint = firstFrame.lockPoint;
                }
                if (fishFrame.physics == null)
                {
                    fishFrame.physics = firstFrame.physics;
                }
            }
            fishFrame.moveDistance = moveDistance;
        }
        for (int i = 1; i <= dieFrameCount; ++i)
        {
            var frame = GetSpriteFrame(dieName, i);
            f.dieFrames.Add(frame);
        }
    }



    public static xx.Pos CalcShadowOffset(int priority)
    {
        return new xx.Pos { x = (2 + 2 * (priority > 18 ? 18 : priority)), y = -(2 + 2 * (priority > 18 ? 18 : priority)) };
    }
}
