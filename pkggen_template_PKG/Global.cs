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
