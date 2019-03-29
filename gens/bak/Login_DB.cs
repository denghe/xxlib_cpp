#pragma warning disable 0169, 0414
using TemplateLibrary;

// login server send to db server
namespace Login_DB
{
    [Desc("根据用户名获取用户信息. 找到就返回 DB.Account. 找不到就返回 Error")]
    class GetAccount
    {
        string username;
    }
}
