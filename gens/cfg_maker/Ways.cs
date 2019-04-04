using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;


// todo: 这里主要实现曲线的生成填充. 直线已改为运行时及时生成。


/// <summary>
/// 轨迹分类
/// </summary>
public enum WayTypes
{
    随机直线, 随机小曲线 // todo: more
}

public static class Ways
{
    static List<PKG.CatchFish.Way> beelines = new List<PKG.CatchFish.Way>();
    // todo: more

    public static void Fill()
    {
        Program.ways[WayTypes.随机直线] = beelines;
        // todo: more

        // 填充鱼线之直线

        {
            var p1_ = new Point(1000, 0);
            var p2_ = new Point(-1000, 0);

            // 一端带 +- 45 度倾斜角范围内的直线
            for (int a = 45; a >= -45; a -= 5)
            {
                var p2 = p2_.Rotate(a);
                FillBeeline(p1_, p2, 5, NormalItemRadius, a);
            }
        }
    }

    // 屏幕设计尺寸
    public static int ScreenWidth = 1280;
    public static int ScreenHeight = 720;

    // 一般物件半径( 一般鱼线并不用于 boss 显示. 因为要避免穿帮, 屏幕设计尺寸 + 一般物件半径 = 安全出生框, 这个框如果太大，会导致物件花很长时间才能移动到屏幕显示区 )
    public static int NormalItemRadius = 300;

    // 矢量旋转
    public static Point Rotate(this Point pos, double a)
    {
        a = a / 180 * Math.PI;
        double sinA = Math.Sin(a);
        double cosA = Math.Cos(a);
        return new Point(pos.X * cosA - pos.Y * sinA, pos.X * sinA + pos.Y * cosA);
    }

    // 计算直线的角度
    public static double GetAngle(Point from, Point to)
    {
        if (from == to) return 0;
        double len_y = to.Y - from.Y;
        double len_x = to.X - from.X;
        return Math.Atan2(len_y, len_x) / Math.PI * 180.0;
    }

    // 找出线段 p01 与 p23 的交点，填充到 p 并返回 true. 如果没有焦点则返回 false
    public static bool GetSegmentIntersection(Point p0, Point p1, Point p2, Point p3, out Point p)
    {
        Point s02 = new Point(), s10 = new Point(), s32 = new Point();
        p = new Point();
        double s_numer, t_numer, denom, t;
        s10.X = p1.X - p0.X;
        s10.Y = p1.Y - p0.Y;
        s32.X = p3.X - p2.X;
        s32.Y = p3.Y - p2.Y;

        denom = s10.X * s32.Y - s32.X * s10.Y;
        if (denom == 0) return false; // Collinear
        bool denomPositive = denom > 0;

        s02.X = p0.X - p2.X;
        s02.Y = p0.Y - p2.Y;
        s_numer = s10.X * s02.Y - s10.Y * s02.X;
        if ((s_numer < 0) == denomPositive) return false; // No collision

        t_numer = s32.X * s02.Y - s32.Y * s02.X;
        if ((t_numer < 0) == denomPositive) return false; // No collision

        if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
            return false; // No collision
                          // Collision detected
        t = t_numer / denom;
        p.X = p0.X + (t * s10.X);
        p.Y = p0.Y + (t * s10.Y);

        return true;
    }

    // 找出直线 AB 与 CD 的交点，填充到 p 并返回 true. 如果平行则返回 false
    public static bool GetBeelineIntersect(Point A, Point B, Point C, Point D, ref Point p)
    {
        // Line AB represented as a1x + b1y = c1
        double a1 = B.Y - A.Y;
        double b1 = A.X - B.X;
        double c1 = a1 * (A.X) + b1 * (A.Y);

        // Line CD represented as a2x + b2y = c2
        double a2 = D.Y - C.Y;
        double b2 = C.X - D.X;
        double c2 = a2 * (C.X) + b2 * (C.Y);

        double determinant = a1 * b2 - a2 * b1;
        if (determinant == 0)
        {
            return false;
        }
        else
        {
            p.X = (b2 * c1 - b1 * c2) / determinant;
            p.Y = (a1 * c2 - a2 * c1) / determinant;
            return true;
        }
    }

    // 找出直线与屏外安全出生框相交的直线上的两个点( 乱序 )( 给定的直线上的两点在显示区域 )
    public static List<Point> GetBeelineEdgeIntersect(Point p1, Point p2, int itemRadius)
    {
        var rtv = new List<Point>();

        var w = (ScreenWidth + itemRadius) / 2;
        var h = (ScreenHeight + itemRadius) / 2;

        // 找出 p1,2 与 出生框 4根直线 的所有交点, 过滤掉绝对值大于 w 或 h 的, 最后应该只剩2个点
        var p = new Point();
        if (GetBeelineIntersect(p1, p2, new Point(-w, -h), new Point(w, -h), ref p) && p.X >= -w && p.Y >= -h && p.X <= w && p.Y <= h) rtv.Add(p);
        if (GetBeelineIntersect(p1, p2, new Point(w, -h), new Point(w, h), ref p) && p.X >= -w && p.Y >= -h && p.X <= w && p.Y <= h) rtv.Add(p);
        if (GetBeelineIntersect(p1, p2, new Point(w, h), new Point(-w, h), ref p) && p.X >= -w && p.Y >= -h && p.X <= w && p.Y <= h) rtv.Add(p);
        if (GetBeelineIntersect(p1, p2, new Point(-w, h), new Point(-w, -h), ref p) && p.X >= -w && p.Y >= -h && p.X <= w && p.Y <= h) rtv.Add(p);
        Debug.Assert(rtv.Count == 2);

        // 输出时确保 [0] 离 p1 近, [1] 离 p2 近
        if ((rtv[0] - p1).Length > (rtv[0] - p2).Length)
        {
            var tmp = rtv[0];
            rtv[0] = rtv[1];
            rtv[1] = tmp;
        }

        return rtv;
    }

    // 填充直线( 非循环线 )
    public static void FillBeeline(Point p1_, Point p2_, int step, int itemRadius, int thatAngle)
    {
        // 旋转生成 36 根
        for (int a = 0; a < 360; a += step)
        {
            if (((a + thatAngle) > 50 && (a + thatAngle) < 120) || ((a + thatAngle) > 200 && (a + thatAngle) < 270)) continue;

            var p1 = p1_.Rotate(a);
            var p2 = p2_.Rotate(a);
            var angle = GetAngle(p1, p2);

            // 找出与屏外安全出鱼框相交的点
            var w = (ScreenWidth + itemRadius) / 2;
            var h = (ScreenHeight + itemRadius) / 2;

            Point p = new Point();
            if (GetSegmentIntersection(new Point(0, 0), p1, new Point(-w, -h), new Point(w, -h), out p)) p1 = p;
            if (GetSegmentIntersection(new Point(0, 0), p1, new Point(w, -h), new Point(w, h), out p)) p1 = p;
            if (GetSegmentIntersection(new Point(0, 0), p1, new Point(w, h), new Point(-w, h), out p)) p1 = p;
            if (GetSegmentIntersection(new Point(0, 0), p1, new Point(-w, h), new Point(-w, -h), out p)) p1 = p;

            if (GetSegmentIntersection(new Point(0, 0), p2, new Point(-w, -h), new Point(w, -h), out p)) p2 = p;
            if (GetSegmentIntersection(new Point(0, 0), p2, new Point(w, -h), new Point(w, h), out p)) p2 = p;
            if (GetSegmentIntersection(new Point(0, 0), p2, new Point(w, h), new Point(-w, h), out p)) p2 = p;
            if (GetSegmentIntersection(new Point(0, 0), p2, new Point(-w, h), new Point(-w, -h), out p)) p2 = p;

            // 算距离
            var distance = (p2 - p1).Length;

            var way = new PKG.CatchFish.Way();
            beelines.Add(way);
            way.points = new xx.List<PKG.CatchFish.WayPoint>();
            way.points.Add(new PKG.CatchFish.WayPoint { pos = { x = (float)p1.X, y = (float)p1.Y }, angle = (float)angle, distance = (float)distance });
            way.points.Add(new PKG.CatchFish.WayPoint { pos = { x = (float)p2.X, y = (float)p2.Y }, angle = 0, distance = 0 });
            way.distance = (float)distance;
        }
    }
}
