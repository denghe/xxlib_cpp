
#include <iostream>
#include <chrono>
#include <algorithm>
#include <memory>
#include <math.h>
#include "file.hpp"
#include "astar.hpp"

using namespace moon;

static int64_t microsecond()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct user_context
{
    
    int x = 0;
    int y = 0;
    int t = 0;

    float estimate(const user_context& goal)
    {
        //return 0;
        return sqrtf(float((x - goal.x) * (x - goal.x) + (y - goal.y) * (y - goal.y)));
        //return (std::abs(x - goal.x) + std::abs(y - goal.y));
    }

    bool canpass()
    {
        return t==1;
    }
};

using graph_t = graph<astar_policy, user_context>;
using astar_t = astar< graph_t>;

std::unique_ptr<graph_t> make_graph(int w, int h, const std::string& obs)
{
    auto m = std::make_unique<graph_t>(w,h);
    for (int j = 0; j < h; ++j)
    {
        for (int i = 0; i < w; ++i)
        {
            auto vtx = m->at(i, j);
            vtx->context.x = i;
            vtx->context.y = j;
            vtx->context.t = obs[(w)*j + i]=='%'?0:1;
        }
    }
    return m;
}

void print_path(const astar_t& a, graph_t* g2)
{
    std::vector< std::pair<int, int>> pathpos;
    auto v = a.start();
    do {
        v = (astar_t::vertex_type*)v->child;
        pathpos.emplace_back(v->context.x, v->context.y);
    } while (v->child != nullptr);

    std::string filename = "path.txt";

    std::ofstream outfile(filename.c_str(), std::ofstream::binary);
    for (int y = 0; y != g2->height(); ++y)
    {
        for (int x = 0; x != g2->width(); ++x)
        {
            std::pair<int, int> id{ x, y };
            //outfile << std::left << std::setw(1);
            if (std::find(pathpos.begin(), pathpos.end(), id) != pathpos.end())
                outfile << '@';
            else if (g2->at(x, y)->context.canpass())
            {
                outfile << ' ';
            }
            else
                outfile << '%';
        }
        outfile << '\n';
    }
}

void _search(graph_t* g2, int start_x, int start_y, int goal_x, int goal_y)
{
    auto bt = microsecond();

    int count = 10000;
    int success = 0;

    astar_t a;
    for (int i = 0; i < count; ++i)
    {
        a.init(g2, start_x, start_y, goal_x, goal_y);
        do
        {
            auto st = a.search();
            if (st == astar_t::searching)
            {
                continue;
            }
            else if (st == astar_t::succeeded)
            {
                success++;
              
                //print_path(a, g2);
                break;
            }
            else
            {
                printf("failed");
                return;
            }
        } while (true);
    }

    printf("total cost %lld us\n", microsecond() - bt);
    printf("success %d.\n", success);
}

int main(int argc, char*argv[])
{
    std::string grids = moon::file::read_all("map2.txt", std::ios::binary | std::ios::in);
    std::remove_if(grids.begin(), grids.end(), [](char c) {
        return c == '\n' || c == '\r';
    });

    int width = 100, height = 100;
    {
        auto bt =  microsecond();
        auto g2 = make_graph(width, height, grids);
        printf("make graph cost %lld us\n", microsecond() - bt);
        //_search(g2.get(), 3, 2, 5, 5);
        _search(g2.get(), 3, 2, 96,97);
        //_search(g2.get(), 19, 21 , 81,20);
    }
    return 0;
}