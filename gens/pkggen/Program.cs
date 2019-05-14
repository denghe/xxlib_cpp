using System.Reflection;
using System.IO;
using System.Windows.Forms;
using System.Text;
using System.Security.Cryptography;

public static class Program
{
    public const string templatePrefix = "pkggen_template_";
    public const string outputPath = "../../../../output";

    public static string GetMD5(this byte[] data)
    {
        MD5 md5 = new MD5CryptoServiceProvider();
        byte[] bytes = md5.ComputeHash(data);
        string str = "";
        for (int i = 0; i < bytes.Length; i++)
        {
            str += bytes[i].ToString("x2");
        }
        return str;
    }

    static void Main(string[] args)
    {
#if !DEBUG
        global::System.Console.WriteLine("please run generater in debug mode!");
        global::System.Console.ReadKey();
        return;
#endif
        var outPath = args.Length > 0 ? args[0] : outputPath;

        // 扫所有添加过引用的, 名字前缀为 templatePrefix 的 dll, 执行相应的生成
        foreach (var fn in Directory.GetFiles(Application.StartupPath, templatePrefix + "*.dll"))
        {
            var md5 = File.ReadAllBytes(fn).GetMD5();
            var asm = Assembly.LoadFile(fn);
            var shortfn = new FileInfo(fn).Name;
            shortfn = shortfn.Substring(0, shortfn.LastIndexOf('.'));
            var path = Path.Combine(Application.StartupPath, outPath);
            var tn = shortfn.Substring(templatePrefix.Length);

            if (!GenTypeId.Gen(asm, path, tn))
            {
                System.Console.WriteLine(tn + "_TypeIdMappings.cs 已生成. 请将其放入模板项目并再次生成. ");
                continue;
            }

            GenCPP_Class.Gen(asm, path, tn, md5);
            GenCPP_SQLite.Gen(asm, path, tn);

            GenLUA_Class.Gen(asm, path, tn, md5);

            GenCS_Class.Gen(asm, path, tn, md5);
            GenCS_MySql.Gen(asm, path, tn);
        }
    }
}
