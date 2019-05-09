using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public static class Fishs
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        var f1 = new PKG.CatchFish.Configs.Fish();
        f1.id = cfg.fishs.dataLen;
        cfg.fishs.Add(f1);
        f1.minCoin = 1;
        f1.maxCoin = 3;
        f1.minDetectRadius = 6;
        f1.maxDetectRadius = 32;
        f1.scale = 1;
        f1.zOrder = 3;
        f1.shadowOffset = new xx.Pos { x = 5, y = 5 };
        f1.shadowScale = 1;
        FillFishFrames(f1, "xiaochouyu_move", 30, "xiaochouyu_die", 10, 2.5f);


        var f2 = new PKG.CatchFish.Configs.BigFish();
        f2.id = cfg.fishs.dataLen;
        cfg.fishs.Add(f2);
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
