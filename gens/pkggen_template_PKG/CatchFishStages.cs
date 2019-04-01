#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    namespace Stages
    {

        [Desc("游戏关卡. 位于 Stage.timers 中的 timer, 使用 stageFrameNumber 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化")]
        class Stage
        {
            [Desc("同下标")]
            int id;

            [Desc("关卡帧编号( clone 后需清0. 每帧 +1 )")]
            int stageFrameNumber;

            [Desc("当前阶段结束时间点( clone 后需修正 )")]
            int endFrameNumber;

            [Desc("关卡元素集合")]
            List<Timer> timers;
        }

        [Desc("服务器本地脚本( 关卡元素 )")]
        class Script : Timer
        {
            int lineNumber;
        }

        // todo: Emitter? Effect? 
        //[Desc("特效基类 ( 声音，画面等元素。派生类进一步具备具体信息 )")]
        //class Effect : Timer
        //{
        //}

    }
}
