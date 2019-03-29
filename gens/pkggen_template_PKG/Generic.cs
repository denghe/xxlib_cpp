#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace Generic
{
    [Desc("通用返回")]
    class Success
    {
    }

    [Desc("通用错误返回")]
    class Error
    {
        long number;
        string message;
    }


    [Desc("心跳保持兼延迟测试 -- 请求")]
    class Ping
    {
        long ticks;
    }

    [Desc("心跳保持兼延迟测试 -- 回应")]
    class Pong
    {
        long ticks;
    }

}
