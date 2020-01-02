#pragma once
#include <vector>
#include <queue>
#include <array>
#include <cstdint>
#include <cassert>

namespace moon
{
    struct astar_policy
    {
        enum vertex_state
        {
            none,
            closed,
            open
        };

        vertex_state vstate = none;
        float g = 0.0f; // cost of this node + it's predecessors
        float h = 0.0f; // heuristic estimate of distance to goal
        float f = 0.0f; // sum of cumulative cost of predecessors and self and heuristic
        astar_policy* parent = nullptr;
        astar_policy* child = nullptr;

        void clear()
        {
            g = 0.0f;
            h = 0.0f;
            f = 0.0f;
            vstate = none;
            parent = nullptr;
            child = nullptr;
        }
    };

    template<typename PathFindPolicy,typename UserContext>
    class graph
    {
    public:
        using policy_type = PathFindPolicy;
        using user_context_type = UserContext;

        inline static std::array<std::pair<int, int>, 8> neighbors = {
            std::pair<int, int>{1,0}
            , std::pair<int, int>{1,1}
            , std::pair<int, int>{ 0,1}
            , std::pair<int, int>{ -1,1}
            , std::pair<int, int>{-1,0}
            , std::pair<int, int>{-1,-1}
            , std::pair<int, int>{0,-1}
            , std::pair<int, int>{1,-1}
        };

        struct vertex :policy_type
        {
            vertex() = default;

            vertex(const vertex&) = delete;
            vertex& operator=(const vertex&) = delete;

            uint32_t version = 1;

            user_context_type context;
        };

        graph(int w, int h)
            :width_(w)
            , height_(h)
        {
            vertexs_ = new vertex[width_*height_];
        }

        ~graph()
        {
            delete[] vertexs_;
        }

        int width() const
        {
            return width_;
        }

        int height() const
        {
            return height_;
        }

        vertex* at(int x, int y) const
        {
            if (x < 0 || x >= width_ || y< 0 || y >= height_)
            {
                return nullptr;
            }
            auto vtx = &vertexs_[y*width_ + x];
            if (vtx->version != version_)
            {
                vtx->clear();
                vtx->version = version_;
            }
            return vtx;
        }

        void reset()
        {
            ++version_;
        }

    private:
        int32_t width_ = 0;
        int32_t height_ = 0;
        uint32_t version_ = 1;
        vertex* vertexs_ = nullptr;
    };

    template<typename Graph>
    class astar
    {
    public:
        enum state
        {
            invalid,
            searching,
            failed,
            succeeded
        };

        using vertex_type = typename Graph::vertex;
        using user_context_type = typename Graph::user_context_type;

        class vertex_greater_compare
        {
        public:
            bool operator() (const vertex_type* x, const vertex_type* y) const noexcept
            {
                if (x->f == y->f)
                {
                    return x > y;
                }
                return x->f > y->f;
            }
        };

        void init(Graph* g, int start_x, int start_y, int goal_x, int goal_y)
        {
            cancel_ = false;
            graph_ = g;
            graph_->reset();
            openlist_.clear();

            start_ = graph_->at(start_x, start_y);
            goal_ = graph_->at(goal_x, goal_y);
            assert(nullptr != start_ && nullptr != goal_);
            state_ = searching;

            start_->g = 0;
            start_->h = start_->context.estimate(goal_->context);
            start_->f = start_->g + start_->h;
            start_->parent = nullptr;

            open_add(start_);
        }

        state search()
        {
            if (openlist_.empty() || cancel_)
            {
                state_ = invalid;
                return state_;
            }

            auto from = open_pos_top();

            if (from==goal_)
            {
                goal_->parent = from->parent;
                goal_->g = from->g;

                if (from != start_)
                {
                    auto v = goal_;
                    auto p = goal_->parent;

                    do
                    {
                        p->child = v;
                        v = reinterpret_cast<vertex_type*>(p);
                        p = p->parent;
                    } while (v != start_);
                }
                state_ = succeeded;
                return state_;
            }
            else
            {
                int x = from->context.x;
                int y = from->context.y;
                for (const auto& ne : graph_->neighbors)
                {
                    vertex_type* to = graph_->at(x + ne.first, y + ne.second);
                    if (nullptr == to || !to->context.canpass())
                    {
                        continue;
                    }

                    auto newg = from->g + ((x == to->context.x || y == to->context.y) ? 1.0f : 1.414f);
                    if (to->vstate == astar_policy::closed&&to->g <= newg)
                    {
                        //The one on closed is cheaper than this one
                        continue;
                    }

                    // Now we need to check whether the vertex is on the open lists
                    if(to->vstate == astar_policy::open)
                    {
                        //but the vertex that is already on them is better
                        if (to->g <= newg)
                        {
                            //then we can forget about this vertex
                            continue;
                        }
                    }

                    //update its astar specific data 
                    to->parent = from;
                    assert(from->parent != to);
                    //printf("set parent %d %d -> %d %d \n", to->context.x, to->context.y, from->context.x, from->context.y);
                    to->g = newg;
                    to->h = to->context.estimate(goal_->context);
                    to->f = to->g + to->h;

                    // vertex in open list
                    if (to->vstate == astar_policy::open)
                    {
                        if(!std::is_heap(openlist_.begin(), openlist_.end(),comp_))
                        {
                            std::make_heap(openlist_.begin(), openlist_.end(), comp_);
                        }
                        continue;
                    }
                    open_add(to);
                }
                from->vstate = astar_policy::closed;
            }
            return state_;
        }

        void cancel()
        {
            cancel_ = true;
        }

        vertex_type* start() const
        {
            return start_;
        }
    private:
        void open_add(vertex_type* v)
        {
            v->vstate = astar_policy::open;
            openlist_.emplace_back(v);
            std::push_heap(openlist_.begin(), openlist_.end(), comp_);
        }

        vertex_type* open_pos_top()
        {
            auto from = openlist_.front();
            std::pop_heap(openlist_.begin(), openlist_.end(), comp_);
            openlist_.pop_back();
            from->vstate = astar_policy::none;
            return  from;
        }
    private:
        bool cancel_ = false;
        state state_ = invalid;
        vertex_type* start_ = nullptr;
        vertex_type* goal_ = nullptr;
        Graph* graph_ = nullptr;
        vertex_greater_compare comp_;
        std::vector< vertex_type*> openlist_;
    };
}
