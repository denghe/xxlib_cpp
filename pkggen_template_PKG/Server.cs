#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace Server
{
    enum Types
    {
        Unknown,
        Login,
        Lobby,
        DB,

        MAX
    }

    [Desc("服务间互表身份的首包")]
    class Info
    {
        Types type;
    }

}
