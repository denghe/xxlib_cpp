#pragma warning disable 0169, 0414
using TemplateLibrary;

// Calc -> CatchFish
namespace Calc_CatchFish
{
    [Desc("鱼死计算结果")]
    class HitCheckResult
    {
        [Desc("死鱼列表")]
        List<Calc.CatchFish.Fish> fishs;

        [Desc("打空的子弹列表")]
        List<Calc.CatchFish.Bullet> bullets;
    }
}
