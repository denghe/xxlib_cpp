using System.Reflection;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using System.Text;
using System.Security.Cryptography;
using System.Xml;
using System.Collections.Generic;
using System.Xml.Serialization;

public static class Program
{
    /// <summary>
    /// 各种轨迹
    /// </summary>
    public static Dictionary<WayTypes, List<PKG.CatchFish.Way>> ways = new Dictionary<WayTypes, List<PKG.CatchFish.Way>>();

    /// <summary>
    /// 帧名( 通常跨贴图唯一 ) 与帧对象临时关联, 方便后面的填充
    /// </summary>
    public static Dictionary<string, PKG.CatchFish.Configs.SpriteFrame> frames = new Dictionary<string, PKG.CatchFish.Configs.SpriteFrame>();

    /// <summary>
    /// 帧名与鱼帧对象临时关联, 方便后面的填充
    /// </summary>
    public static Dictionary<string, PKG.CatchFish.Configs.FishSpriteFrame> fishFrames = new Dictionary<string, PKG.CatchFish.Configs.FishSpriteFrame>();

    /// <summary>
    /// 基础配置文件输入目录
    /// </summary>
    public static string inputPath = Application.StartupPath + "/../../../../input";

    /// <summary>
    /// 存盘输出目录
    /// </summary>
    public static string outputPath = Application.StartupPath + "/../../../../output";

    static void Main(string[] args)
    {
        // 注册序列化类型
        PKG.AllTypes.Register();

        // 填充 ways
        Ways.Fill();

        // 填充 frames, fishFrames
        SpriteFrames.Fill();

        // 创建配置实例
        var cfg = new PKG.CatchFish.Configs.Config();
        cfg.cannons = new xx.List<PKG.CatchFish.Configs.Cannon>();
        cfg.fishs = new xx.List<PKG.CatchFish.Configs.Fish>();
        cfg.sitPositons = new xx.List<xx.Pos>();
        cfg.stages = new xx.List<PKG.CatchFish.Stages.Stage>();
        cfg.fixedWays = new xx.List<PKG.CatchFish.Way>();
        cfg.weapons = new xx.List<PKG.CatchFish.Configs.Weapon>();

        // 基于 1280 x 720 的设计尺寸
        cfg.sitPositons.Add(new xx.Pos { x = -250, y = -335 });
        cfg.sitPositons.Add(new xx.Pos { x = 250, y = -335 });
        cfg.sitPositons.Add(new xx.Pos { x = 250, y = 335 });
        cfg.sitPositons.Add(new xx.Pos { x = -250, y = 335 });
        cfg.aimTouchRadius = 20;
        cfg.normalFishMaxRadius = 150;
        cfg.enableBulletFastForward = false;

        Fishs.Fill(cfg);
        Cannons.Fill(cfg);
        Stages.Fill(cfg);
        // todo: more fill

        // cfg 序列化存盘
        var bb = new xx.BBuffer();
        bb.WriteRoot(cfg);
        File.WriteAllBytes(Path.Combine(outputPath, "cfg.bin"), bb.DumpData());
        bb.ReadRoot(ref cfg);   // test read
    }
}
