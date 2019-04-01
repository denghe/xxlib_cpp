using System.Reflection;
using System.IO;
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


        var bb = new xx.BBuffer();
        bb.WriteRoot(cfg);
        File.WriteAllBytes(Path.Combine(outputPath, "cfg.bin"), bb.DumpData());
        bb.ReadRoot(ref cfg);   // test read
    }
}
