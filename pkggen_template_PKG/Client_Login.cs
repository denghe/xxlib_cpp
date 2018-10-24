#pragma warning disable 0169, 0414
using TemplateLibrary;

// client send to login server
namespace Client_Login
{
    [Desc("校验身份, 成功返回 ConnInfo, 内含下一步需要连接的服务的明细. 失败立即被 T")]
    class Auth
    {
        // Limit 为 client -> server 时需要, 提供数组类的长度限定. 以免 server 反序列化时申请太多内存. 超长的数据发送没问题, 反序列化时将失败
        [Desc("包版本校验")]
        [Limit(64)] string pkgMD5;
        [Desc("用户名")]
        [Limit(64)] string username;
        [Desc("密码")]
        [Limit(64)] string password;
    }
}
