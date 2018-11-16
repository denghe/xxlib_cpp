#pragma warning disable 0169, 0414
using TemplateLibrary;

// tiled map 存档对应结构( 基于 1.2+ )

public enum OrientationTypes
{
    Unknown,
    Orthogonal,
    Isometric,
    Staggered,
    Hexagonal
}

public enum RenderOrderTypes
{
    RightDown,
    RightUp,
    LeftDown,
    LeftUp
}

struct Color4B
{
    byte r, g, b, a;
}

class Map
{
    string version;
    string tiledversion;
    OrientationTypes orientation;
    RenderOrderTypes renderorder;
    int width, height;
    int tilewidth, tileheight;
    int infinite;
    int nextlayerid;
    int nextobjectid;
    // backgroundcolor?
    List<TileSet> tilesets;
    List<Layer> layers;
    List<ObjectGroup> objectgroups;
    // imagelayer, group
    List<Property> properties;
}

class Property
{
    string name, value;
}

class TileSet   // 简化版
{
    int firstgid;
    string name;
    int tilewidth, tileheight;
    int spacing, margin;
    // int tilecount, columns;

    // Image 内容展开
    string source;          // 图片文件名
    int width, height;
    Color4B trans;

    // terraintypes, tiles 省略
}

// todo: TilesetTile

class Layer
{
    int id;
    string name;
    int width, height;
    // opacity, visible, offsetx, offsety
}

class Tile
{
    int gid;
    // x, y, HorizontalFlip, VerticalFlip, DiagonalFlip
}

class ObjectGroup
{
    int id;
    string name;
    List<Object> data;
}

class Object
{
    int id;
    int gid;
    float x, y;
    int width, height;
}
