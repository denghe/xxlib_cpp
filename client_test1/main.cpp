/*
777 游戏规则

格子: 坐标码( 数组下标 )
 0  1  2  3  4
 5  6  7  8  9
10 11 12 13 14

判定线下标列表：
5, 6, 7, 8, 9
0, 1, 2, 3, 4
10, 11, 12, 13, 14
0, 6, 12, 8, 4
10, 6, 2, 8, 14
0, 1, 7, 3, 4
10, 11, 7, 13, 14
5, 11, 12, 13, 9
5, 1, 2, 3, 9

中奖规则：
依据判定线下标 正反双向扫格子。从第 1 格开始数，连续出现相同形状 3 次或以上，即中奖，金额查表.
0 符号可替代任意非 0 符号, 计数时要考虑进去.
多条判定线可重复中奖. 最终奖励叠加.
另外, 存在所有格子同时判定的情况，比如全部都是相同符号或符合某种分类

符号列表:
0, 1, 2, 3, ......9

奖励表:
.......
0*3 特殊1
0*4 特殊2
0*5 特殊3
1*3 5倍
1*4 10倍
1*5 20倍
2*3 10倍
2*4 20倍
2*5 40倍
......
1*15 1000倍
2*15 2000倍
......
单数*15 500倍
双数*15 500倍

需求：
对所有格子随机填充符号, 统计出最终倍数
*/

#pragma execution_character_set("utf-8")

#include <iostream>
#include <array>
#include <random>
#include <cassert>
#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include <xx_file.h>

struct Result {
	int index;			// 线下标. -1 代表全屏
	int direction;		// 方向( 0: 从左到右. 1: 从右到左 )
	int symbol;			// 中奖符号
	int count;			// 符号个数
	void Dump() {
		std::cout << index << (direction ? " <-- " : " --> ") << symbol << "*" << count << std::endl;
	}
};

const int gridLen = 15;
const int lineLen = 5;
const int linesLen = 9;
using Grid = std::array<int, gridLen>;
using Line = std::array<int, lineLen>;
using Lines = std::array<Line, linesLen>;
using Results = std::array<Result, linesLen * 2>;


//inline std::mt19937_64 rnd;
//inline std::uniform_int_distribution gen(0, numSymbols - 1);
//rnd.seed(std::random_device()());
//inline void Fill() {
//	for (auto&& v : grid) {
//		v = gen(rnd);
//	}
//}

inline void Dump(Grid const& grid) {
	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 5; ++i) {
			auto&& v = grid[j * 5 + i];
			if (i) {
				std::cout << " ";
			}
			if (v == -1) {
				std::cout << "_";
			}
			else {
				std::cout << v;
			}
		}
		std::cout << std::endl;
	}
}

template<int offset, int step = (offset > 0 ? 1 : -1)>
inline uint16_t CalcLine(int& symbol, int& count, int const* cursor, Grid const& grid) noexcept {
	uint16_t mask = (1 << *cursor);
	symbol = grid[*cursor];
	count = 1;
	auto end = cursor + offset;
	// 如果 0 打头, 向后找出一个非 0 的来...
	if (!symbol) {
		while ((cursor += step) != end) {
			if ((symbol = grid[*cursor])) break;
			mask |= (1 << *cursor);
			++count;
		}
		// 3 个 0 优先判定
		if (count >= 3) {
			symbol = 0;
			return mask;
		}
		mask |= (1 << *cursor);
		++count;
	}
	// 继续数当前符号个数
	while ((cursor += step) != end) {
		int s = grid[*cursor];
		if (s && symbol != s) break;
		mask |= (1 << *cursor);
		++count;
	}
	return mask;
}

inline void CalcGrid(int& symbol, int& count, Grid const& grid) noexcept {
	int cursor = 0;
	symbol = grid[cursor];
	count = 1;
	const auto end = grid.size();
	// 如果 0 打头, 向后找出一个非 0 的来...
	if (!symbol) {
		while (++cursor != end) {
			if ((symbol = grid[cursor])) break;
			++count;
		}
		if (cursor == end) return;
		++count;
	}
	// 继续数当前符号个数
	while (++cursor != end) {
		int s = grid[cursor];
		if (s && symbol != s) break;
		++count;
	}
}

inline uint16_t Calc(Result* results, int& resultsLen, Grid const& grid, Lines const& lines) noexcept {
	uint16_t m = 0, m1, m2;
	int s1, c1, s2, c2;
	CalcGrid(s1, c1, grid);
	if (c1 == 15) {
		auto&& r = results[resultsLen++];
		r.index = -1;
		r.direction = 0;
		r.symbol = s1;
		r.count = c1;
		return 0b0111111111111111;
	}
	// 这里不需要总类全屏特殊判断
	for (size_t i = 0; i < linesLen; ++i) {
		m1 = CalcLine<lineLen>(s1, c1, (int*)& lines[i], grid);
		m2 = CalcLine<-lineLen>(s2, c2, (int*)& lines[i] + lineLen - 1, grid);
		if (c1 >= 3) {
			auto&& r = results[resultsLen++];
			r.index = (int)i;
			r.direction = 0;
			r.symbol = s1;
			r.count = c1;
			m |= m1;
		}
		if (c2 >= 3) {
			if (c1 < 3 || s1 != s2) {
				auto&& r = results[resultsLen++];
				r.index = (int)i;
				r.direction = 1;
				r.symbol = s2;
				r.count = c2;
				m |= m2;
			}
		}
	}
	return m;
}

union GridValues {
	struct {
		uint8_t c0 : 2;
		uint8_t c1 : 2;
		uint8_t c2 : 2;
		uint8_t c3 : 2;
		uint8_t c4 : 2;
		uint8_t c5 : 2;
		uint8_t c6 : 2;
		uint8_t c7 : 2;
		uint8_t c8 : 2;
		uint8_t c9 : 2;
		uint8_t c10 : 2;
		uint8_t c11 : 2;
		uint8_t c12 : 2;
		uint8_t c13 : 2;
		uint8_t c14 : 2;
		uint8_t c__ : 2;
	};
	uint32_t value;
};

union ShapeNums {
	struct {
		uint8_t n3 : 8;
		uint8_t n4 : 8;
		uint8_t n5 : 8;
		uint8_t n15 : 8;
	};
	uint32_t value;
};

union LineMask {
	struct {
		uint8_t index : 4;
		uint8_t direction : 1;
		uint8_t count : 3;
	};
	char value;
};

// 返回二进制的指定线上的 0 掩码
template<int offset, int step = (offset > 0 ? 1 : -1)>
inline int CalcLineZeroMask(int const& count, int const* const& line, Grid const& grid) noexcept {
	int rtv = 0;
	for (int i = 0; i < count; ++i) {
		if (!grid[*(line + i * step)]) {
			rtv |= (1 << i);
		}
	}
	return rtv;
}

// 返回二进制的 0 分布图
inline uint16_t CalcGridZeroMask(Grid const& grid) noexcept {
	uint16_t rtv = 0;
	for (int i = 0; i < grid.size(); ++i) {
		if (!grid[i]) {
			rtv |= (1 << i);
		}
	}
	return rtv;
}


// 返回 0 的个数
inline size_t CalcGridZeroCount(Grid const& grid) noexcept {
	size_t rtv = 0;
	for (int i = 0; i < grid.size(); ++i) {
		if (!grid[i]) {
			++rtv;
		}
	}
	return rtv;
}

// 返回 0 的平均出现个数( 结果为 3.75 )
inline void CalcAvgZeroCount() noexcept {
	Grid grid;
	GridValues g;
	size_t count = 0;
	size_t count2 = 0;
	for (g.value = 0; g.value <= 0b111111111111111111111111111111; ++g.value) {
		grid = {
			g.c0, g.c1, g.c2, g.c3, g.c4,
			g.c5, g.c6, g.c7, g.c8, g.c9,
			g.c10, g.c11, g.c12, g.c13, g.c14,
		};
		auto&& n = CalcGridZeroCount(grid);
		if (n) {
			count += n;
			++count2;
		}
	}
	std::cout << (double)count / 0b111111111111111111111111111111 << ", " << count2;
}

//int main() {
//	CalcAvgZeroCount();
//	return 0;
//}



inline Lines lines = {
	5, 6, 7, 8, 9,
	0, 1, 2, 3, 4,
	10, 11, 12, 13, 14,
	0, 6, 12, 8, 4,
	10, 6, 2, 8, 14,
	0, 1, 7, 3, 4,
	10, 11, 7, 13, 14,
	5, 11, 12, 13, 9,
	5, 1, 2, 3, 9,
};

int Gen() {
	Grid grid;
	std::array<int, 4> counts;
	std::map<uint32_t, int> shapes;
	std::map<uint32_t, std::unordered_set<std::string>> shapesResults;
	Results results;
	int resultsLen = 0;
	GridValues g;
	for (g.value = 0; g.value <= 0b111111111111111111111111111111; ++g.value) {
		if ((g.value & 0b11111111111111111111111) == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		grid = {
			g.c0, g.c1, g.c2, g.c3, g.c4,
			g.c5, g.c6, g.c7, g.c8, g.c9,
			g.c10, g.c11, g.c12, g.c13, g.c14,
		};


		resultsLen = 0;
		auto&& mask = Calc((Result*)& results, resultsLen, grid, lines);
		if (!resultsLen) continue;
		if (results[0].index == -1) continue;		// ignore full screen
		
		counts.fill(0);
		std::string masks;
		auto zeroMask = CalcGridZeroMask(grid) & mask;	// todo: &= results 的 mask 以去除 line 之外的 0
		masks.append((char*)& zeroMask, sizeof(zeroMask));
		for (int i = 0; i < resultsLen; ++i) {
			++counts[results[i].count - 3];
			LineMask m;
			m.index = results[i].index;
			m.direction = results[i].direction;
			m.count = results[i].count;
			masks.append((char*)&m.value, sizeof(m.value));
		}

		auto key = counts[0] | counts[1] << 8 | counts[2] << 16;
		++shapes[key];

		shapesResults[key].insert(std::move(masks));
	}

	std::cout << shapes.size() << std::endl;

	size_t siz = 0;
	ShapeNums n;
	std::string s;
	for (auto&& iter : shapes) {
		n.value = iter.first;
		s.clear();
		if (n.n3) {
			s.append("3*");
			s.append(std::to_string(n.n3));
		}
		if (n.n4) {
			if (s.size()) {
				s.append(", ");
			}
			s.append("4*");
			s.append(std::to_string(n.n4));
		}
		if (n.n5) {
			if (s.size()) {
				s.append(", ");
			}
			s.append("5*");
			s.append(std::to_string(n.n5));
		}
		if (n.n15) {
			if (s.size()) {
				s.append(", ");
			}
			s.append("15*");
			s.append(std::to_string(n.n5));
		}
		auto&& numResults = shapesResults[iter.first].size();
		siz += numResults;
		s = s + " : " + std::to_string(iter.second) + ", shapesResults.size() = " + std::to_string(numResults);
		std::cout << s << std::endl;


	}
	std::cout << "total shapesResults 's size = " << siz << std::endl;
	
	xx::BBuffer bb;
	for (auto&& iter : shapesResults) {
		bb.Write(iter.first);				// 3x4y5z
		bb.Write(iter.second.size());		// len
		for (auto&& str : iter.second) {	// masks( grid zero mask 2 bytes + LineMask 1 byte * n
			bb.Write(str);
		}
	}
	xx::WriteAllBytes("huga.bin", bb);
	std::cout << "bb.len = " << bb.len << std::endl;		// 最终输出 482131 KB
}





inline static std::mt19937_64 rnd;
inline static std::uniform_int_distribution gen(1, 8);
//rnd.seed(std::random_device()());

inline void FillGridByZeroMask(Grid& grid, uint16_t const& mask) {
	for (int i = 0; i < 15; ++i) {
		if (mask & (1 << i)) {
			grid[i] = 0;
		}
	}
}

inline void FillLinesCrossCellIndex(Grid& grid, std::vector<std::vector<int>> const& lineIdxss, size_t const& currLineIdx, int const& cellIndex) {
	// 遍历所有线, 找出途径 cellIndex 的
	for (size_t i = 0; i < lineIdxss.size(); ++i) {
		// 跳过当前线
		if (i == currLineIdx) continue;
		// 判断是否途径
		auto&& lineIndexs = lineIdxss[i];
		bool found = false;
		for (auto&& idx : lineIndexs) {
			if (idx == cellIndex) {
				found = true;
				break;
			}
		}
		// 扫出空格并填充
		if (found) {
			for (auto&& idx : lineIndexs) {
				auto&& cell = grid[idx];
				if (cell == -1) {
					cell = grid[cellIndex];
				}
			}
		}
	}
}

inline void FillGridByLineIdxss(Grid& grid, std::vector<std::vector<int>> const& lineIdxss, size_t const& currLineIdx) {
	auto&& lineIdxs = lineIdxss[currLineIdx];

	// 通扫一遍，统计空位格数, 当前符号
	int numFreeCells = 0;
	int symbol = -1;
	for (int i = 0; i < lineIdxs.size(); ++i) {
		auto&& cell = grid[lineIdxs[i]];
		if (cell == -1) {
			++numFreeCells;
		}
		if (cell > 0) {
			symbol = cell;
		}
	}

	// 如果没有空位直接退出
	if (!numFreeCells) return;
	
	// 没当前符号: 随机选择一个符号
	if (symbol == -1) {
		symbol = gen(rnd);
	}

	// 符号填入空位
	for (int i = 0; i < lineIdxs.size(); ++i) {
		auto&& idx = lineIdxs[i];
		auto&& cell = grid[idx];
		if (cell == -1) {
			cell = symbol;
			// 找出所有途径该 cell 的 line , 都填充这个 symbol
			FillLinesCrossCellIndex(grid, lineIdxss, currLineIdx, idx);
		}
	}
}

// 中间格子下标列表
inline std::array<int, 9> middleIdxs = { 1,2,3, 6,7,8, 11,12,13 };

// 边缘格子与邻格对应表
inline std::array<std::pair<int, std::vector<int>>, 6> edgeNeighborss = {
	std::make_pair(0, std::vector<int>{ 1, 6 }),
	std::make_pair(5, std::vector<int>{ 1, 6, 11 }),
	std::make_pair(10, std::vector<int>{ 6, 11 }),
	std::make_pair(4, std::vector<int>{ 3, 8 }),
	std::make_pair(9, std::vector<int>{ 3, 8, 13 }),
	std::make_pair(14, std::vector<int>{ 8, 13 })
};

inline void FillGridByShape(Grid& grid, std::string const& s) {
	// 初始化填充
	grid.fill(-1);

	// 按掩码填充 0
	FillGridByZeroMask(grid, *(uint16_t*)s.data());

	// 将 LineMask ( char ) 转为 vector<vector<int>>
	std::vector<std::vector<int>> lineIdxss;
	LineMask m;
	for (size_t i = 2; i < s.size(); ++i) {
		m.value = s[i];
		auto&& lineIdxs = lineIdxss.emplace_back();
		auto&& line = lines[m.index];
		for (int i = 0; i < m.count; ++i) {
			auto&& idx = line[m.direction ? line.size() - i - 1 : i];
			lineIdxs.push_back(idx);
		}
	}

	// 逐线填充
	for (size_t i = 0; i < lineIdxss.size(); ++i) {
		FillGridByLineIdxss(grid, lineIdxss, i);
	}

	// 填充杂物
	// 增加限制，避免造成中奖线延长( 3 个变 4 个之类 )
	// 扫描目标格子是否有中奖线的延长线经过 如果有(特指 3 变 4), 拿到这些中奖线的 symbol, 随机时例外

	// 预处理：整理中奖线，找出 3 个的，取其符号与第 4 格的坐标作为 key. 符号 list 作为 values
	std::unordered_map<int, std::vector<int>> cellDeniedSymbols;
	for (size_t i = 2; i < s.size(); ++i) {
		m.value = s[i];
		if (m.count != 3) continue;
		// 判断中奖线的符号
		int symbol = -1;
		auto&& line = lines[m.index];
		for (int i = 0; i < m.count; ++i) {
			auto&& idx = line[m.direction ? line.size() - i - 1 : i];
			if (grid[idx] != 0) {
				symbol = grid[idx];
				break;
			}
		}
		if (symbol != -1) {
			int cell4idx = line[m.direction ? line.size() - 3 - 1 : 3];	// 第 4 格下标
			cellDeniedSymbols[cell4idx].push_back(symbol);
		}
	}

	// 开始填充
	for (auto&& idx : middleIdxs) {
		if (grid[idx] == -1) {
			auto&& deniedSymbols = cellDeniedSymbols[idx];
			grid[idx] = gen(rnd);
			while (std::find(deniedSymbols.cbegin(), deniedSymbols.cend(), grid[idx]) != deniedSymbols.cend()) {
				grid[idx] = gen(rnd);
			}
		}
	}

	// 遍历两侧最边上的格子，随机成与自己横向斜向相邻格子不同的符号
	for (auto&& edgeNeighbors : edgeNeighborss) {
		auto&& idx = edgeNeighbors.first;
		if (grid[idx] == -1) {
			while (true) {
				grid[idx] = gen(rnd);
				bool notFound = true;
				for (auto&& neighbor : edgeNeighbors.second) {
					if (grid[idx] == grid[neighbor]) {
						notFound = false;
						break;
					}
				}
				if (notFound) break;
			}
		}
	}
}

int Test() {
	std::vector<std::pair<uint32_t, std::vector<std::string>>> shapes;
	{
		xx::BBuffer bb;
		xx::ReadAllBytes("huga.bin", bb);
		xx::CoutN(bb.len);

		size_t len;
		while (bb.offset < bb.len) {
			auto&& ss = shapes.emplace_back();
			if (int r = bb.Read(ss.first)) return r;
			if (int r = bb.Read(len)) return r;
			ss.second.reserve(len);
			for (size_t i = 0; i < len; ++i) {
				auto&& s = ss.second.emplace_back();
				if (int r = bb.Read(s)) return r;
			}
		}
		xx::CoutN("shapes.len = ", shapes.size());
	}
	ShapeNums n;
	std::string s;
	for (int i = 0; i < shapes.size(); ++i) {
		auto&& pair = shapes[i];
		n.value = pair.first;
		s.clear();
		if (n.n3) {
			s.append("3*");
			s.append(std::to_string(n.n3));
		}
		if (n.n4) {
			if (s.size()) {
				s.append(", ");
			}
			s.append("4*");
			s.append(std::to_string(n.n4));
		}
		if (n.n5) {
			if (s.size()) {
				s.append(", ");
			}
			s.append("5*");
			s.append(std::to_string(n.n5));
		}
		if (n.n15) {
			if (s.size()) {
				s.append(", ");
			}
			s.append("15*");
			s.append(std::to_string(n.n5));
		}
		s = s + " : " + std::to_string(pair.second.size());
		std::cout << "shapes[" << i << "] = " << s << std::endl;
	}

	// 试定位到某 shape 某 grid zero mask
	auto&& ss = shapes[0].second;
	Grid g;
	for (auto&& s : ss) {
		FillGridByShape(g, s);
		Dump(g);
		xx::CoutN();
	}

	return 0;
}

int main() {
	//return Gen();
	return Test();
}


/*

..... 约 993 种 shape + 全屏

*/



//int Test() {//inline std::array<std::array<int, 5>, 1> lines;
	//lines[0] = { 0, 1, 2, 3, 4 };
//	Init();
//	results.reserve(lines.size() * 2);
//	{
//		grid = { 2, 0, 0, 0, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 2, 0, 0, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 2, 2, 0, 0, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 2, 2, 0, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 2, 2, 2, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 1, 1, 1, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(1, 3));
//	}
//	{
//		grid = { 1, 1, 0, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 2 && results[0] == std::make_pair(1, 3) && results[1] == std::make_pair(2, 3));
//	}
//	{
//		grid = { 1, 0, 1, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(1, 3));
//	}
//	{
//		grid = { 1, 0, 0, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 2 && results[0] == std::make_pair(1, 3) && results[1] == std::make_pair(2, 4));
//	}
//	{
//		grid = { 0, 1, 1, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(1, 3));
//	}
//	{
//		grid = { 0, 1, 0, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 2 && results[0] == std::make_pair(1, 3) && results[1] == std::make_pair(2, 3));
//	}
//	{
//		grid = { 0, 0, 1, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(1, 3));
//	}
//	{
//		grid = { 0, 0, 0, 2, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 2 && results[0] == std::make_pair(0, 3) && results[1] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 0, 0, 0, 0, 2 };
//		results.clear();
//		Calc();
//		assert(results.size() == 2 && results[0] == std::make_pair(0, 4) && results[1] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 0, 0, 0, 2, 0 };
//		results.clear();
//		Calc();
//		assert(results.size() == 2 && results[0] == std::make_pair(0, 3) && results[1] == std::make_pair(2, 5));
//	}
//	{
//		grid = { 0, 0, 0, 0, 0 };
//		results.clear();
//		Calc();
//		assert(results.size() == 1 && results[0] == std::make_pair(0, 5));
//	}
//
//	return 0;
//}
