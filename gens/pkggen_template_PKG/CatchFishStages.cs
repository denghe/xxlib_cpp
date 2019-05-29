#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace CatchFish
{
    namespace Stages
    {
        // 命名规则：以 cfg_ 打头的 fields 为配置性质的东西，数据维持不变. 其他为运行时数据可变

        [Desc("游戏关卡. 一切元素皆使用 Stage.ticks 来计算时间. 可弱引用 Stage 本身. 需要可以干净序列化")]
        class Stage
        {
            [Desc("关卡 id( 通常等于下标值 )")]
            int cfg_id;

            [Desc("结束时间点")]
            int cfg_endTicks;

            [Desc("帧编号( 运行时每帧 +1 )")]
            int ticks;

            [Desc("元素集合")]
            List<StageElement> elements;

            [Desc("监视器集合, 服务端专用")]
            List<StageElement> monitors;
        }

        [AttachInclude, Desc("关卡元素基类")]
        class StageElement
        {
            [Desc("生效时间点")]
            int cfg_beginTicks;

            [Desc("结束时间点")]
            int cfg_endTicks;
        }

        [AttachInclude, CustomInitCascade, Desc("发射器: 随机小鱼")]
        class Emitter_RandomFishs : StageElement
        {
            [Desc("配置: 两条鱼生成帧间隔")]
            int cfg_bornTicksInterval;

            [Desc("配置: 币值")]
            long cfg_coin;

            [Desc("配置: 体积随机起始范围")]
            float cfg_scaleFrom;

            [Desc("配置: 体积随机结束范围")]
            float cfg_scaleTo;

            [Desc("记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )")]
            int bornAvaliableTicks;
        }

        [AttachInclude, CustomInitCascade, Desc("监视器: 自动再生肥鱼, 服务端预约下发")]
        class Monitor_KeepFatFish : Emitter_RandomFishs
        {
            [Desc("配置: 鱼总数限制")]
            int cfg_numFishsLimit;

            [Desc("配置: 预约延迟")]
            int cfg_bornDelayFrameNumber;
        }

        [AttachInclude, CustomInitCascade, Desc("监视器: 自动再生大鱼, 服务端预约下发")]
        class Monitor_KeepBigFish : StageElement
        {
            [Desc("配置: 两条鱼生成帧间隔")]
            int cfg_bornTicksInterval;

            [Desc("配置: 鱼总数限制")]
            int cfg_numFishsLimit;

            [Desc("配置: 预约延迟")]
            int cfg_bornDelayFrameNumber;

            [Desc("记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )")]
            int bornAvaliableTicks;
        }

        [AttachInclude, CustomInitCascade, Desc("监视器: 自动再生炸弹, 服务端预约下发")]
        class Monitor_KeepBombFish : Monitor_KeepBigFish
        {
        }

        [AttachInclude, CustomInitCascade, Desc("发射器: 从屏幕中间圆环批量出小鱼")]
        class Emitter_RingFishs : StageElement
        {
            [Desc("配置: 每波鱼只数")]
            int cfg_numFishsPerBatch;

            [Desc("配置: 两波鱼生成帧间隔")]
            int cfg_bornTicksInterval;

            [Desc("配置: 每只鱼币值")]
            long cfg_coin;

            [Desc("配置: 每只鱼体积")]
            float cfg_scale;

            [Desc("配置: 每只鱼移动速度( 帧跨越像素距离 )")]
            float cfg_speed;

            [Desc("记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )")]
            int bornAvaliableTicks;
        }

        [AttachInclude, CustomInitCascade, Desc("发射器: 从屏幕中间 0 度开始旋转式出小鱼")]
        class Emitter_CircleFishs : StageElement
        {
            [Desc("配置: 起始角度")]
            float cfg_angleBegin;

            [Desc("配置: 每只鱼偏转角度")]
            float cfg_angleIncrease;

            [Desc("配置: 两只鱼生成帧间隔")]
            int cfg_bornTicksInterval;

            [Desc("配置: 每只鱼币值")]
            long cfg_coin;

            [Desc("配置: 每只鱼体积")]
            float cfg_scale;

            [Desc("配置: 每只鱼移动速度( 帧跨越像素距离 )")]
            float cfg_speed;

            [Desc("记录下次生成需要的帧编号( 在生成时令该值 = Stage.ticks + cfg_bornTicksInterval )")]
            int bornAvaliableTicks;

            [Desc("当前角度")]
            float angle;
        }


        //[Desc("特效基类 ( 声音，画面等元素。派生类进一步具备具体信息 )")]
        //class Effect : Timer
        //{
        //}
    }
}
