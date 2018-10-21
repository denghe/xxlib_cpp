#pragma warning disable 0169, 0414
using TemplateLibrary;

// client send to login server
namespace Client_Login
{
    [Desc("校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T")]
    class Auth
    {
        [Desc("包版本校验")]
        [Limit(64)] string pkgMD5;
        [Desc("用户名")]
        [Limit(64)] string username;
        [Desc("密码")]
        [Limit(64)] string password;
    }
}
