#pragma warning disable 0169, 0414
using TemplateLibrary;

// Calc -> CatchFish
namespace Calc_CatchFish
{
    [Desc("死鱼检查结果")]
    class FishDieCheckResult
    {
        [Desc("判定结果表. ")]
        List<Calc.CatchFish.Result> results;
    }
}
