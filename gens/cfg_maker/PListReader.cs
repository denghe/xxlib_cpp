using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Xml;

class Spritesheet
{
    public string Name { get; set; }
    public Dictionary<string, Image> Sprites { get; set; }

    public Spritesheet(string spriteSheetName)
    {
        this.Sprites = new Dictionary<string, Image>(50);
        this.Name = spriteSheetName;
    }

    public static Spritesheet LoadSpriteSheet(string coordinatesFile)
    {
        XmlDocument doc = new XmlDocument();
        doc.Load(coordinatesFile);
        XmlNode metadata = doc.SelectSingleNode("/plist/dict/key[.='metadata']");
        XmlNode realTextureFileName =
        metadata.NextSibling.SelectSingleNode("key[.='realTextureFileName']");
        string spritesheetName = realTextureFileName.NextSibling.InnerText;
        //Image spriteSheetImage = Image.FromFile(spritesheetName);
        XmlNode frames = doc.SelectSingleNode("/plist/dict/key[.='frames']");
        XmlNodeList list = frames.NextSibling.SelectNodes("key");

        Spritesheet spritesheet = new Spritesheet(coordinatesFile);

        foreach (XmlNode node in list)
        {
            XmlNode dict = node.NextSibling;
            string strRectangle = dict.SelectSingleNode
            ("key[.='frame']").NextSibling.InnerText;
            string strOffset = dict.SelectSingleNode
            ("key[.='offset']").NextSibling.InnerText;
            string strSourceRect = dict.SelectSingleNode
            ("key[.='sourceColorRect']").NextSibling.InnerText;
            string strSourceSize = dict.SelectSingleNode
            ("key[.='sourceSize']").NextSibling.InnerText;
            Rectangle frame = parseRectangle(strRectangle);
            Point offset = parsePoint(strOffset);
            Rectangle sourceRectangle = parseRectangle(strSourceRect);
            Point size = parsePoint(strSourceSize);

            string spriteFrameName = node.InnerText;
            //Image sprite = new Bitmap(size.X, size.Y);
            //Graphics drawer = Graphics.FromImage(sprite);
            //drawer.DrawImage(spriteSheetImage, sourceRectangle, frame, GraphicsUnit.Pixel);
            //drawer.Save();
            //drawer.Dispose();
            //spritesheet.Sprites.Add(spriteFrameName, sprite);
            spritesheet.Sprites.Add(spriteFrameName, null);
        }
        return spritesheet;
    }

    private static Rectangle parseRectangle(string rectangle)
    {
        Regex expression = new Regex(@"\{\{(\d+),(\d+)\},\{(\d+),(\d+)\}\}");
        Match match = expression.Match(rectangle);
        if (match.Success)
        {
            int x = int.Parse(match.Groups[1].Value);
            int y = int.Parse(match.Groups[2].Value);
            int w = int.Parse(match.Groups[3].Value);
            int h = int.Parse(match.Groups[4].Value);
            return new Rectangle(x, y, w, h);
        }
        return Rectangle.Empty;
    }

    private static Point parsePoint(string point)
    {
        Regex expression = new Regex(@"\{(\d+),(\d+)\}");
        Match match = expression.Match(point);
        if (match.Success)
        {
            int x = int.Parse(match.Groups[1].Value);
            int y = int.Parse(match.Groups[2].Value);
            return new Point(x, y);
        }
        return Point.Empty;
    }
}
