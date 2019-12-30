#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <array>
#include <cassert>
#include <algorithm>
#include <optional>
#include <chrono>

#define ENABLE_HEURISTIC
// 对于 map1 似乎结果输出不正确. 有空再修修看?
//#define ENABLE_CELLHEAP2

struct Cell {
	int x = 0, y = 0;
	int walkable = 0;
	int unused = 0;

	int isOpened = 0;
	int isClosed = 0;
	float heuristicStartToEndLen = 0.0f;
	float startToCurNodeLen = 0.0f;
#ifdef ENABLE_HEURISTIC
	int heuristicCurNodeToEndLen_hasValue = 0;
	float heuristicCurNodeToEndLen = 0.0f;
#endif
	Cell* parent = nullptr;

	inline void Clear() {
		memset(&isOpened, 0, sizeof(Cell) - 4 * 4);
	}

	inline void Fill(std::vector<Cell*>& path) {
		path.clear();
		auto c = this;
		path.push_back(this);
		while ((c = c->parent)) {
			path.push_back(c);
		}
	}
};

struct CellHeap {
	struct Comparer {
		bool operator() (Cell const* x, Cell const* y) const noexcept {
			return x->heuristicStartToEndLen > y->heuristicStartToEndLen;
		}
	} comparer;

	std::vector<Cell*> data;

	void Clear() {
		data.clear();
	}

	bool Empty() const {
		return data.empty();
	}

	void Add(Cell* const& c) {
		data.emplace_back(c);
		std::push_heap(data.begin(), data.end(), comparer);
	}

	Cell* DeleteMin() {
		auto from = data.front();
		std::pop_heap(data.begin(), data.end(), comparer);
		data.pop_back();
		return from;
	}
};

struct CellHeap2 {
	std::vector<Cell*> data;

	void Clear() {
		data.clear();
	}

	bool Empty() const {
		return data.empty();
	}

	void Add(Cell* const& c) {
		int p = (int)data.size(), p2;
		data.push_back(c);
		do {
			if (p == 0) break;
			p2 = (p - 1) / 2;
			if (data[p]->heuristicStartToEndLen < data[p2]->heuristicStartToEndLen) {
				std::swap(data[p], data[p2]);
				p = p2;
			}
			else break;
		} while (true);
	}

	Cell* DeleteMin() {
		auto result = data[0];
		int p = 0, p1, pn;
		data[0] = data[data.size() - 1];
		data.pop_back();
		do {
			pn = p;
			p1 = 2 * p + 1;
			if ((int)data.size() > p1 && data[p]->heuristicStartToEndLen > data[p1]->heuristicStartToEndLen) {
				p = p1;
			}
			if (p == pn) break;
			std::swap(data[p], data[pn]);
		} while (true);
		return result;
	}
};

struct Grid {
	int width = 0;
	int height = 0;
	Cell* startCell = nullptr;
	Cell* endCell = nullptr;
	std::vector<Cell> cells;
	std::vector<Cell*> path;
	static constexpr std::array<std::pair<int, int>, 8> neighborIndexs = {
		std::pair<int, int>{-1, -1}
		, std::pair<int, int>{0, -1}
		, std::pair<int, int>{1, -1}
		, std::pair<int, int>{-1, 0}
		, std::pair<int, int>{1, 0}
		, std::pair<int, int>{-1, 1}
		, std::pair<int, int>{0, 1}
		, std::pair<int, int>{1, 1}
	};
	const float sqrt_2 = sqrtf(2);
#ifdef ENABLE_CELLHEAP2
	CellHeap2 openList;
#else
	CellHeap openList;
#endif

	Cell& At(int const& x, int const& y) {
		assert(x >= 0 && y >= 0 && x < width && y < height);
		return cells[(size_t)y * width + x];
	}

#ifdef ENABLE_HEURISTIC
	static float Heuristic(Cell* const& a, Cell* const& b) {
		//return (std::abs(a->x - b->x) + std::abs(a->y - b->y));
		return sqrtf(float((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y)));
	}
#endif

	bool FindPath() {
		if (!startCell || !endCell) return false;
		Reset();
		openList.Clear();
		openList.Add(startCell);
		startCell->isOpened = 1;

		while (!openList.Empty()) {
			auto cell = openList.DeleteMin();
			cell->isOpened = 0;
			cell->isClosed = 1;

			if (cell == endCell) {
				cell->Fill(path);
				return true;
			}

			auto cx = cell->x;
			auto cy = cell->y;

			for (auto&& ni : neighborIndexs) {
				auto n = &At(ni.first + cell->x, ni.second + cell->y);
				if (n->isClosed || !n->walkable) continue;
				auto nx = n->x;
				auto ny = n->y;
				auto ng = cell->startToCurNodeLen + ((nx == cx || ny == cy) ? 1.0f : sqrt_2);

				if (!n->isOpened || ng < n->startToCurNodeLen) {
					n->startToCurNodeLen = ng;
#ifdef ENABLE_HEURISTIC
					if (!n->heuristicCurNodeToEndLen_hasValue) {
						n->heuristicCurNodeToEndLen = Heuristic(cell, endCell);
						n->heuristicCurNodeToEndLen_hasValue = 1;
					}
					n->heuristicStartToEndLen = n->startToCurNodeLen + n->heuristicCurNodeToEndLen;
#else
					n->heuristicStartToEndLen = n->startToCurNodeLen;
#endif
					n->parent = cell;
					if (!n->isOpened) {
						openList.Add(n);
						n->isOpened = 1;
					}
				}
			}
		}
		return false;
	}

	void Reset() {
		for (auto&& c : cells) {
			c.Clear();
		}
	}

	void LoadByFile(char const* const& fileName) {
		std::ifstream f(fileName);
		std::string tmp;
		std::vector<std::string> ss;
		while (getline(f, tmp)) {
			if (tmp[tmp.size() - 1] < 36) {
				tmp.resize(tmp.size() - 1);
			}
			ss.push_back(tmp);
		}

		width = (int)ss[0].size();
		height = (int)ss.size();
		cells.resize((size_t)width * height);
		path.clear();
		startCell = nullptr;
		endCell = nullptr;

		for (int y = 0; y < height; ++y) {
			auto& s = ss[y];
			for (int x = 0; x < width; ++x) {
				switch (s[x]) {
				case '@':
					startCell = &At(x, y);
					At(x, y).walkable = 1;
					break;
				case '*':
					endCell = &At(x, y);
				case ' ':
					At(x, y).walkable = 1;
					break;
				default:
					At(x, y).walkable = 0;
				}
			}
		}

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				At(x, y).x = x;
				At(x, y).y = y;
			}
		}
	}

	void Dump() {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				auto o = &At(x, y);
				if (o == startCell) {
					std::cout << "o";
				}
				else if (o == endCell) {
					std::cout << "*";
				}
				else if (path.size() && std::find(path.begin(), path.end(), o) != path.end()) {
					std::cout << "+";
				}
				else if (o->walkable) {
					std::cout << " ";
				}
				else {
					std::cout << "#";
				}
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
};

int64_t NowMS() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int main() {
	Grid g;
	g.LoadByFile("map1.txt");

	auto ms = NowMS();
	int count = 0;
	for (int i = 0; i < 10000; ++i) {
		if (g.FindPath()) {
			++count;
		}
	}
	std::cout << "elapsed ms = " << (NowMS() - ms) << std::endl;
	std::cout << "count = " << count << std::endl;
	std::cout << "map width = " << g.width << ", height = " << g.height << std::endl;
	if (count) g.Dump();
	return 0;
}
