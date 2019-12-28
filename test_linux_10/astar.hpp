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

        static constexpr std::array<int,16> dirs = {
            1,0,
            1,1,
            0,1,
            -1,1,
            -1,0,
            -1,-1,
            0,-1,
            1,-1
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

        int width()
        {
            return width_;
        }

        int height()
        {
            return height_;
        }

        vertex* at(int x, int y)
        {
            auto vtx = &vertexs_[x*width_ + y];
            if (vtx->version != version_)
            {
                vtx->clear();
                vtx->version = version_;
            }
            return vtx;
        }

        const std::vector<std::pair<vertex*, float>>& neighbors(vertex* v)
        {
            neighbors_.clear();

            auto N = height_;
            auto M = width_;
            auto x = v->context.x;
            auto y = v->context.y;

            for (int i = 0; i < dirs.size(); i+=2)
            {
                int _x = x + dirs[i];
                int _y = y + dirs[i+1];
                if (_x < width_ && _x >=0 && _y < height_ && _y >= 0)
                {
                    auto vtx = at(_x, _y);
                    if (vtx->context.canpass())
                    {
                        if (_x == x || _y == y)
                        {
                            neighbors_.emplace_back(at(_x, _y), 1.0f);
                        }
                        else
                        {
                            neighbors_.emplace_back(at(_x, _y), 1.44f);
                        }
                    }
                }
            }
            return neighbors_;
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
        std::vector<std::pair<vertex*,float>> neighbors_;
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

        class vertex_less_compare
        {
        public:
            bool operator() (const vertex_type *x, const vertex_type *y) const noexcept
            {
                if (x->f == y->f)
                {
                    return x > y;
                }
                return x->f > y->f;
            }
        };

        using open_continer_type = std::priority_queue<vertex_type*, std::vector<vertex_type*>, vertex_less_compare>;

        //using open_continer_type = std::set<vertex_type*,vertex_less_compare>;

        void init(Graph* g, const user_context_type& start, const user_context_type& goal)
        {
            cancel_ = false;

            graph_ = g;

            start_ = graph_->at(start.x, start.y);
            goal_ = graph_->at(goal.x, goal.y);
            assert(nullptr != start_ && nullptr != goal_);
            state_ = searching;

            graph_->reset();
            openlist_ = open_continer_type();

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
                for (auto& e : graph_->neighbors(from))
                {
                    vertex_type* to = e.first;
                    auto newg = from->g + e.second;
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
                    to->g = newg;
                    to->h = to->context.estimate(goal_->context);
                    to->f = to->g + to->h;

                    // vertex in opem list
                    if (to->vstate == astar_policy::open)
                    {
                        // remove vertex from open list, then  reinsert  it
                        //assert(false);
                        /*openlist_.erase(to);*/
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
            openlist_.emplace(v);
        }

        vertex_type* open_pos_top()
        {
            auto from = openlist_.top();
            openlist_.pop();
            from->vstate = astar_policy::none;
            return from;
        }
    private:
        bool cancel_ = false;
        state state_ = invalid;
        vertex_type* start_ = nullptr;
        vertex_type* goal_ = nullptr;
        Graph* graph_ = nullptr;
        open_continer_type openlist_;
    };
}
