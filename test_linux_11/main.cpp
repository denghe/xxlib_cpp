#include <stack>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <algorithm>
#include "Stopwatch.h"
#include "AStar.h"

enum class ItemType : short
{
    None,
    Wall,
    Space,
};

struct Item
{
    ItemType type;
    int x, y;
    bool IsWalkable(Item const& o)
    {
        return o.type == ItemType::Space;
    }
};


Map<Item>* Fill(int& aX, int& aY, int& bX, int& bY)
{
    std::ifstream f("map2.txt");
    std::string tmp;
    std::vector<std::string> ss;
    while (getline(f, tmp))
    {
        //ss.push_back(std::string(tmp.data(), tmp.size()-1));
        ss.push_back(tmp);
    }

    auto result = new Map<Item>((int)ss[0].size(), (int)ss.size());

    for (int y = 0; y < (int)ss.size(); ++y)
    {
        auto& s = ss[y];
        for (int x = 0; x < (int)s.size(); ++x)
        {
            auto& c = s[x];
            auto& item = result->At(x, y);
            switch (c)
            {
            case '%':
                item = { ItemType::None, x, y };
                break;
            case '#':
                item = { ItemType::Wall, x, y };
                break;
            case ' ':
                item = { ItemType::Space, x, y };
                break;
            case '@':
                item = { ItemType::Space, x, y };
                aX = x; aY = y;
                break;
            case '*':
                item = { ItemType::Space, x, y };
                bX = x; bY = y;
                break;
            }
        }
    }

    return result;
}
void Dump(Map<Item>& m, int aX, int aY, int bX, int bY, AStar<Item>* astar = nullptr)
{
    for (int y = 0; y < m.h; ++y)
    {
        for (int x = 0; x < m.w; ++x)
        {
            auto& item = m.At(x, y);
            switch (item.type)
            {
            case ItemType::None:
                std::cout << "%";
                break;
            case ItemType::Space:
                if (aX == x && aY == y)
                {
                    std::cout << "@";
                }
                else if (bX == x && bY == y)
                {
                    std::cout << "*";
                }
                else
                {
                    if (astar)
                    {
                        bool found = false;
                        for (auto& o : astar->searchResults)
                        {
                            if (o->x == x && o->y == y)
                            {
                                found = true;
                                break;
                            }
                        }
                        if (found)
                        {
                            std::cout << "+";
                        }
                        else
                        {
                            std::cout << " ";
                        }
                    }
                    else std::cout << " ";
                }
                break;
            case ItemType::Wall:
                std::cout << "#";
                break;
            default:
                assert(false);
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main()
{
    int aX, aY, bX, bY;
    auto m = Fill(aX, aY, bX, bY);
    //Dump( *m, aX, aY, bX, bY );

    AStar<Item> astar(*m);

    xxx::Stopwatch sw;
    int count = 0;
    for (int i = 0; i < 1000; ++i)
    {
        if (astar.Search(aX, aY, bX, bY))
        {
            ++count;
        }
    }
    std::cout << "elapsed ms = " << sw.ElapsedMillseconds() << std::endl;
    std::cout << "count = " << count << std::endl;
    std::cout << "map width = " << m->w << ", height = " << m->h << std::endl;
    //if( count ) Dump( *m, aX, aY, bX, bY, &astar );

    delete m;
    return 0;
}
