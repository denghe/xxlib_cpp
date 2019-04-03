using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

public static class SpriteFrames
{
    public static List<string> GetFrames(string plistFileName)
    {
        var rtv = new List<string>();
        var doc = new XmlDocument();
        doc.Load(plistFileName);
        var frames = doc.SelectSingleNode("/plist/dict/key[.='frames']");
        var list = frames.NextSibling.SelectNodes("key");
        foreach (XmlNode node in list)
        {
            rtv.Add(node.InnerText);
        }
        return rtv;
    }

    public static void Fill()
    {
        // 得到 input\ 文件列表
        var fileNames = Directory.GetFiles(Program.inputPath);

        // 找出 *.plist 用于生成 SpriteFrame
        var fns = fileNames.Where(fn => fn.EndsWith(".plist"));
        foreach (var fn in fns)
        {
            var fs = GetFrames(fn);
            foreach (var f in fs)
            {
                var sf = new PKG.CatchFish.Configs.SpriteFrame();
                if (Program.frames.ContainsKey(f)) throw new System.Exception("发现重复的帧名:" + f);
                Program.frames[f] = sf;
                sf.frameName = f;
                sf.plistName = fn.Substring(Program.inputPath.Length + 1);  // 移除目录路径部分. 只留下文件名
            }
        }

        // 找出 *.physics.xml 转为物理顶点
        fns = fileNames.Where(fn => fn.EndsWith(".physics.xml"));
        foreach (var fn in fns)
        {
            var bodydef = Physics.FromXML(fn);
            foreach (var body in bodydef.Bodies.Body)
            {
                if (!Program.frames.ContainsKey(body.Name)) throw new System.Exception("未找到与物理帧名相同的帧:" + body.Name);
                if (Program.fishFrames.ContainsKey(body.Name)) throw new System.Exception("发现重复创建的鱼帧名:" + body.Name);

                var ff = new PKG.CatchFish.Configs.FishSpriteFrame();
                Program.fishFrames[body.Name] = ff;
                ff.frame = Program.frames[body.Name];
                ff.physics = new PKG.CatchFish.Configs.Physics();
                ff.physics.polygons = new xx.List<xx.List<xx.Pos>>();
                foreach (var fixture in body.Fixture)
                {
                    foreach (var polygon in fixture.Polygon)
                    {
                        var vs = new xx.List<xx.Pos>();
                        foreach (var vertex in polygon.Vertex)
                        {
                            vs.Add(new xx.Pos { x = (float)vertex.X, y = (float)-vertex.Y }); // y 值反向
                        }
                        ff.physics.polygons.Add(vs);
                    }
                }
            }
        }

        // 找出 *.lockpointers.xml 转为 首选锁定点, 锁定线
        fns = fileNames.Where(fn => fn.EndsWith(".lockpointers.xml"));
        foreach (var fn in fns)
        {
            var bodydef = Physics.FromXML(fn);
            foreach (var body in bodydef.Bodies.Body)
            {
                if (!Program.fishFrames.ContainsKey(body.Name)) throw new System.Exception("未找到鱼帧:" + body.Name);
                var ff = Program.fishFrames[body.Name];
                if (ff.lockPoints != null) throw new System.Exception("重复创建的锁定线 位于鱼帧:" + body.Name);
                ff.lockPoints = new xx.List<xx.Pos>();

                foreach (var fixture in body.Fixture)
                {
                    if (fixture.Polygon.Count == 0) throw new System.Exception("未找到 锁定线 数据 位于鱼帧:" + body.Name);
                    foreach (var polygon in fixture.Polygon)
                    {
                        if (polygon.Vertex.Count == 0) throw new System.Exception("未找到 锁定线 数据 位于鱼帧:" + body.Name);
                        foreach (var vertex in polygon.Vertex)
                        {
                            ff.lockPoints.Add(new xx.Pos { x = (float)vertex.X, y = (float)-vertex.Y }); // y 值反向
                        }
                    }
                }
            }
        }

        // 找出 *.lockpointer.xml 转为 首选锁定点
        fns = fileNames.Where(fn => fn.EndsWith(".lockpointer.xml"));
        foreach (var fn in fns)
        {
            var bodydef = Physics.FromXML(fn);
            foreach (var body in bodydef.Bodies.Body)
            {
                if (!Program.fishFrames.ContainsKey(body.Name)) throw new System.Exception("未找到鱼帧:" + body.Name);
                var ff = Program.fishFrames[body.Name];
                if (ff.lockPoint.x != 0 || ff.lockPoint.y != 0) throw new System.Exception("重复创建的 首选锁定点 位于鱼帧:" + body.Name);

                foreach (var fixture in body.Fixture)
                {
                    if (fixture.Circle.Count == 0) throw new System.Exception("未找到 首选锁定点 数据 位于鱼帧:" + body.Name);
                    if (fixture.Circle.Count > 1) throw new System.Exception("首选锁定点 数据过多 位于鱼帧:" + body.Name);

                    var circle = fixture.Circle[0];
                    ff.lockPoint = (new xx.Pos { x = (float)circle.X, y = (float)-circle.Y }); // y 值反向
                }
            }
        }
    }
}



// 用于加载 PE 按 AndEngine xml 导出的文件的 xml 类
public static class Physics
{
    // xml to obj
    public static Bodydef FromXML(string fn)
    {
        var serializer = new XmlSerializer(typeof(Bodydef));
        var buffer = File.ReadAllBytes(fn);
        using (var stream = new MemoryStream(buffer))
        {
            return (Bodydef)serializer.Deserialize(stream);
        }
    }


    [XmlRoot(ElementName = "vertex")]
    public class Vertex
    {
        [XmlAttribute(AttributeName = "x")]
        public double X { get; set; }
        [XmlAttribute(AttributeName = "y")]
        public double Y { get; set; }
    }

    [XmlRoot(ElementName = "circle")]

    public class Circle
    {
        [XmlAttribute(AttributeName = "r")]
        public double R { get; set; }
        [XmlAttribute(AttributeName = "x")]
        public double X { get; set; }
        [XmlAttribute(AttributeName = "y")]
        public double Y { get; set; }
    }

    [XmlRoot(ElementName = "polygon")]
    public class Polygon
    {
        [XmlElement(ElementName = "vertex")]
        public List<Vertex> Vertex { get; set; }
        [XmlAttribute(AttributeName = "numVertexes")]
        public string NumVertexes { get; set; }
    }

    [XmlRoot(ElementName = "fixture")]
    public class Fixture
    {
        [XmlElement(ElementName = "polygon")]
        public List<Polygon> Polygon { get; set; }
        [XmlElement(ElementName = "circle")]
        public List<Circle> Circle { get; set; }
        [XmlAttribute(AttributeName = "density")]
        public string Density { get; set; }
        [XmlAttribute(AttributeName = "friction")]
        public string Friction { get; set; }
        [XmlAttribute(AttributeName = "restitution")]
        public string Restitution { get; set; }
        [XmlAttribute(AttributeName = "filter_categoryBits")]
        public string Filter_categoryBits { get; set; }
        [XmlAttribute(AttributeName = "filter_groupIndex")]
        public string Filter_groupIndex { get; set; }
        [XmlAttribute(AttributeName = "filter_maskBits")]
        public string Filter_maskBits { get; set; }
        [XmlAttribute(AttributeName = "isSensor")]
        public string IsSensor { get; set; }
        [XmlAttribute(AttributeName = "type")]
        public string Type { get; set; }
        [XmlAttribute(AttributeName = "numPolygons")]
        public string NumPolygons { get; set; }
    }

    [XmlRoot(ElementName = "body")]
    public class Body
    {
        [XmlElement(ElementName = "fixture")]
        public List<Fixture> Fixture { get; set; }
        [XmlAttribute(AttributeName = "name")]
        public string Name { get; set; }
        [XmlAttribute(AttributeName = "dynamic")]
        public string Dynamic { get; set; }
        [XmlAttribute(AttributeName = "numFixtures")]
        public string NumFixtures { get; set; }
    }

    [XmlRoot(ElementName = "bodies")]
    public class Bodies
    {
        [XmlElement(ElementName = "body")]
        public List<Body> Body { get; set; }
        [XmlAttribute(AttributeName = "numBodies")]
        public string NumBodies { get; set; }
    }

    [XmlRoot(ElementName = "metadata")]
    public class Metadata
    {
        [XmlElement(ElementName = "format")]
        public string Format { get; set; }
        [XmlElement(ElementName = "ptm_ratio")]
        public string Ptm_ratio { get; set; }
    }

    [XmlRoot(ElementName = "bodydef")]
    public class Bodydef
    {
        [XmlElement(ElementName = "bodies")]
        public Bodies Bodies { get; set; }
        [XmlElement(ElementName = "metadata")]
        public Metadata Metadata { get; set; }
        [XmlAttribute(AttributeName = "version")]
        public string Version { get; set; }
    }

}
