using System.Reflection;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using System.Text;
using System.Security.Cryptography;

public static class Program
{
    public const string inputPath = "../../../../input";
    public const string outputPath = "../../../../output";

    static void Main(string[] args)
    {
        PKG.AllTypes.Register();
        var cfg = new PKG.CatchFish.Configs.Config();
        cfg.frames = new xx.List<PKG.CatchFish.Configs.SpriteFrame>();

        // todo: 从 input 得到文件列表。找出 *.plist 用于生成 SpriteFrame
        var fileNames = Directory.GetFiles(inputPath);
        var plists = fileNames.Where(fn => fn.EndsWith(".plist"));
        foreach (var fn in plists)
        {
            var ss = Spritesheet.LoadSpriteSheet(fn);
            foreach (var kv in ss.Sprites)
            {
                var sf = new PKG.CatchFish.Configs.SpriteFrame();
                sf.frameName = kv.Key;
                sf.textureName = ss.Name;
                cfg.frames.Add(sf);
            }
        }

        var bb = new xx.BBuffer();
        bb.WriteRoot(cfg);
        File.WriteAllBytes(Path.Combine(outputPath, "cfg.bin"), bb.DumpData());
        bb.ReadRoot(ref cfg);   // test read
    }
}
