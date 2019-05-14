#pragma warning disable 0169, 0414
using TemplateLibrary;

// CatchFish -> Calc
namespace CatchFish_Calc
{
    [Desc("死鱼检查")]
    class FishDieCheck
    {
        [Desc("在遍历 player cannon bullets 过程中, 按被 hit 的鱼来分组，得到击打列表")]
        List<Calc.CatchFish.Fish_Bullet_1_n> fishBullets;
    }
}
