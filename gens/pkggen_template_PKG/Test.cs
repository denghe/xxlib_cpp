#pragma warning disable 0169, 0414
using TemplateLibrary;

namespace Test
{
    class Foo
    {
    }

    class Player
    {
        int id;
        Weak<Scene> owner;
    }

    class Scene
    {
        List<Fish> fishs;
        List<Weak<Player>> players;
    }

    class Fish
    {
        int id;
    }

    class EnterSuccess
    {
        List<Player> players;
        Scene scene;
    }

}
