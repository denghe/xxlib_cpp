#pragma warning disable 0169, 0414
using TemplateLibrary;


class Player
{
    int id;

    [NotSerialize]
    string token;

    Ref<Monster> target;
}

class Monster
{
}

class Scene
{
    List<Monster> monsters;
    List<Ref<Player>> players;
}

[Desc("当需要序列化同时携带玩家数据时, 临时构造这个结构体出来发送以满足 Ref<Player> 数据存放问题")]
class Sync
{
    List<Player> players;
    Scene scene;
}




































// for test
class Foo
{
    Ref<Foo> parent;
    List<Ref<Foo>> childs;
}


//// 一些通用结构
//namespace Generic
//{
//    class Success
//    {
//    }

//    class Error
//    {
//        int number;
//        string text;
//    }


//}
