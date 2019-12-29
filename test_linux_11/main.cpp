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

struct Cell {
	int x = 0, y = 0;
	int walkable = 0;

	int isOpened = 0;
	int isClosed = 0;
#ifdef ENABLE_HEURISTIC
	int heuristicCurNodeToEndLen_hasValue = 0;
	float heuristicCurNodeToEndLen = 0.0f;
#endif
	float heuristicStartToEndLen = 0.0f;
	float startToCurNodeLen = 0.0f;
	Cell* parent = nullptr;

	inline void Clear() {
		memset(&isOpened, 0, sizeof(Cell) - 4 * 3);
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

struct CellComparer {
	static bool LargeThan(Cell* const& a, Cell* const& b) {
		return a->heuristicStartToEndLen > b->heuristicStartToEndLen;
	}
	static bool LessThan(Cell* const& a, Cell* const& b) {
		return a->heuristicStartToEndLen < b->heuristicStartToEndLen;
	}

	bool operator() (const Cell* x, const Cell* y) const noexcept {
		return x->heuristicStartToEndLen > y->heuristicStartToEndLen;
	}
};


template<typename T = Cell*, typename C = CellComparer>
struct IntervalHeap {
	struct Interval {
		T first = nullptr;
		T last = nullptr;
	};
	std::vector<Interval> heap;
	int size = 0;

	void Clear() {
		size = 0;
		memset(heap.data(), 0, sizeof(Interval) * heap.size());
	}

	void swapFirstWithLast(int const& cell1, int const& cell2) {
		auto first = heap[cell1].first;
		updateFirst(cell1, heap[cell2].last);
		updateLast(cell2, first);
	}

	void swapFirstWithFirst(int const& cell1, int const& cell2) {
		auto first = heap[cell2].first;
		updateFirst(cell2, heap[cell1].first);
		updateFirst(cell1, first);
	}

	bool heapifyMin(int const& cell) {
		bool result = false;
		if (2 * cell + 1 < size && C::LargeThan(heap[cell].first, heap[cell].last)) {
			result = true;
			swapFirstWithLast(cell, cell);
		}
		int num = cell;
		int num2 = 2 * cell + 1;
		int num3 = num2 + 1;
		if (2 * num2 < size && C::LessThan(heap[num2].first, heap[num].first)) {
			num = num2;
		}
		if (2 * num3 < size && C::LessThan(heap[num3].first, heap[num].first)) {
			num = num3;
		}
		if (num != cell) {
			swapFirstWithFirst(num, cell);
			heapifyMin(num);
		}
		return result;
	}

	void bubbleUpMin(int i) {
		if (i > 0) {
			auto first = heap[i].first;
			auto val = first;
			int num = (i + 1) / 2 - 1;
			while (i > 0 && C::LessThan(val, (first = heap[num = (i + 1) / 2 - 1].first))) {
				updateFirst(i, first);
				first = val;
				i = num;
			}
			updateFirst(i, val);
		}
	}

	void bubbleUpMax(int i) {
		if (i > 0) {
			auto last = heap[i].last;
			auto val = last;
			int num = (i + 1) / 2 - 1;
			while (i > 0 && C::LargeThan(val, (last = heap[num = (i + 1) / 2 - 1].last))) {
				updateLast(i, last);
				last = val;
				i = num;
			}
			updateLast(i, val);
		}
	}

	IntervalHeap(int const& capacity = 1024) {
		heap.resize(capacity);
	}

	void Add(T const& item) {
		if (size == 0) {
			size = 1;
			updateFirst(0, item);
			return;
		}
		if (size == 2 * (int)heap.size()) {
			heap.resize(size);
		}
		if (size % 2 == 0) {
			int num = size / 2;
			int num2 = (num + 1) / 2 - 1;
			auto last = heap[num2].last;
			if (C::LargeThan(item, last)) {
				updateFirst(num, last);
				updateLast(num2, item);
				bubbleUpMax(num2);
			}
			else {
				updateFirst(num, item);
				if (C::LessThan(item, heap[num2].first)) {
					bubbleUpMin(num);
				}
			}
		}
		else {
			int num3 = size / 2;
			auto first = heap[num3].first;
			if (C::LessThan(item, first)) {
				updateLast(num3, first);
				updateFirst(num3, item);
				bubbleUpMin(num3);
			}
			else {
				updateLast(num3, item);
				bubbleUpMax(num3);
			}
		}
		size++;
	}

	void updateLast(int const& cell, T const& item) {
		heap[cell].last = item;
	}

	void updateFirst(int const& cell, T const& item) {
		heap[cell].first = item;
	}

	T DeleteMin() {
		if (size == 0) {
			throw - 1;
		}
		auto first = heap[0].first;
		if (size == 1) {
			size = 0;
			heap[0].first = nullptr;
		}
		else {
			int num = (size - 1) / 2;
			if (size % 2 == 0) {
				updateFirst(0, heap[num].last);
				heap[num].last = nullptr;
			}
			else {
				updateFirst(0, heap[num].first);
				heap[num].first = nullptr;
			}
			size--;
			heapifyMin(0);
		}
		return first;
	}
};

struct CellHeap {
	CellComparer comparer;
	std::vector<Cell*> data;
	void Clear() {
		data.clear();
	}
	size_t size() const {
		return data.size();
	}
	void Add(Cell* const& c) {
		//c->isOpened = 1;
		data.emplace_back(c);
		std::push_heap(data.begin(), data.end(), comparer);
	}

	Cell* DeleteMin() {
		auto from = data.front();
		std::pop_heap(data.begin(), data.end(), comparer);
		data.pop_back();
		from->isOpened = 0;	// 该不该清??
		return from;
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
	//IntervalHeap<> openList;
	CellHeap openList;

	Cell& At(int const& x, int const& y) {
		assert(x >= 0 && y >= 0 && x < width && y < height);
		return cells[(size_t)y * width + x];
	}

#ifdef ENABLE_HEURISTIC
	static float Heuristic(Cell* const& a, Cell* const& b) {
		return (std::abs(a->x - b->x) + std::abs(a->y - b->y));
		//return sqrtf(float((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y)));
	}
#endif

	bool FindPath() {
		if (!startCell || !endCell) return false;
		Reset();
		openList.Clear();
		openList.Add(startCell);
		startCell->isOpened = 1;

		while (openList.size()) {
			//auto cell = openList.DeleteMin();
			auto cell = openList.DeleteMin();
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
	g.LoadByFile("map3.txt");

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
