#pragma warning disable 0169, 0414
using TemplateLibrary;

// client send to lobby server
namespace Client_Lobby
{
    [Desc("首包. 进入大厅. 成功返回 所在位置( 初次应该位于大厅根部. 断线/顶线重连指不定在哪, 但是会返回所有上层数据 ), 以及个人信息. 失败立即被 T")]
    class Enter
    {
        [Limit(64)] string token;
    }

}
