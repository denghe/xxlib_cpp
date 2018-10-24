#pragma warning disable 0169, 0414
using TemplateLibrary;

// 告知生成器生成 lua 时只生成下列命名空间的东西

[LuaFilter(null)]
[LuaFilter(nameof(Login_Client))]
[LuaFilter(nameof(Client_Login))]
[LuaFilter(nameof(Lobby_Client))]
[LuaFilter(nameof(Client_Lobby))]
interface ILuaFilter
{
}
