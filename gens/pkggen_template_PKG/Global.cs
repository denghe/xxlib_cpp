#pragma warning disable 0169, 0414
using TemplateLibrary;

[Desc("操作成功( 默认 response 结果 )")]
class Success
{
}

[Desc("出错( 通用 response 结果 )")]
class Error
{
    int id;
    string txt;
}

//[Desc("服务连接信息")]
//class ConnInfo
//{
//    [Desc("服务类型")]
//    Server.Types type;

//    [Desc("ipv4/6 地址")]
//    string ip;

//    [Desc("端口")]
//    int port;

//    [Desc("令牌")]
//    string token;
//}

//[Desc("并非一般的数据包. 仅用于声明各式 List<T>")]
//class Collections
//{
//    List<int> ints;
//    List<long> longs;
//    List<string> strings;
//    List<object> objects;
//}
