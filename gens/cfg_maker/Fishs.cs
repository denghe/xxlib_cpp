using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public static class Fishs
{
    public static void Fill(PKG.CatchFish.Configs.Config cfg)
    {
        {
            var f = new PKG.CatchFish.Configs.Fish();
            f.id = cfg.fishs.dataLen;
            cfg.fishs.Add(f);
            f.minCoin = 3;
            f.maxCoin = 3;
            f.minDetectRadius = 20;
            f.maxDetectRadius = 32;
            f.scale = 1;
            f.zOrder = 3;
            f.shadowOffset = new xx.Pos { x = 5, y = 5 };
            f.shadowScale = 1;
            FillFishFrames(f, "xiaochouyu_move", 30, "xiaochouyu_die", 10, 2.5f);
        }
        {
            var f = new PKG.CatchFish.Configs.Fish();
            f.id = cfg.fishs.dataLen;
            cfg.fishs.Add(f);
            f.minCoin = 20;
            f.maxCoin = 50;
            f.minDetectRadius = 20;
            f.maxDetectRadius = 32;
            f.scale = 6;
            f.zOrder = 4;
            f.shadowOffset = new xx.Pos { x = 10, y = 10 };
            f.shadowScale = 1;
            FillFishFrames(f, "xiaochouyu_move", 30, "xiaochouyu_die", 10, 2.5f);
        }
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
