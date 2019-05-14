#pragma warning disable 0169, 0414
using TemplateLibrary;

// CatchFish -> Calc
namespace CatchFish_Calc
{
    [Desc("鱼死计算")]
    class HitCheck
    {
        [Desc("碰撞数据流")]
        List<Calc.CatchFish.Hit> hits;
    }
}
