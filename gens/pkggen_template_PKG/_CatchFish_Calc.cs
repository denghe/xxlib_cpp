#pragma warning disable 0169, 0414
using TemplateLibrary;

// CatchFish -> Calc
namespace CatchFish_Calc
{
    [Desc("直接向总输入追加数据( 应对点杀之类需求 )")]
    class Push
    {
        long value;
    }

    [Desc("直接向总输出追加数据( 应对点送之类需求 )")]
    class Pop
    {
        long value;
    }

    [Desc("鱼死计算")]
    class HitCheck
    {
        [Desc("碰撞数据流")]
        List<Calc.CatchFish.Hit> hits;
    }
}
