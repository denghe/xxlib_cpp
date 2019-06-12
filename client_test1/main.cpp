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


#include <iostream>
#include <array>
#include <random>
#include <cassert>
#include <chrono>
#include <vector>

struct Result {
	int index;			// 线下标. -1 代表全屏
	int direction;		// 方向( 0: 从左到右. 1: 从右到左 )
	int symbol;			// 中奖符号
	int count;			// 符号个数
	void Dump() {
		std::cout << index << (direction ? " <-- " : " --> ") << symbol << "*" << count << std::endl;
	}
};

inline std::vector<Result> results;
inline std::array<int, 15> grid;
inline std::array<std::array<int, 5>, 9> lines;
inline int numSymbols = 9;

inline std::mt19937_64 rnd;
inline std::uniform_int_distribution gen(0, numSymbols - 1);

inline void Init() {
	lines[0] = { 5, 6, 7, 8, 9 };
	lines[1] = { 0, 1, 2, 3, 4 };
	lines[2] = { 10, 11, 12, 13, 14 };
	lines[3] = { 0, 6, 12, 8, 4 };
	lines[4] = { 10, 6, 2, 8, 14 };
	lines[5] = { 0, 1, 7, 3, 4 };
	lines[6] = { 10, 11, 7, 13, 14 };
	lines[7] = { 5, 11, 12, 13, 9 };
	lines[8] = { 5, 1, 2, 3, 9 };

	rnd.seed(std::random_device()());
	results.reserve(lines.size() * 2);
}

inline void Fill() {
	for (auto&& v : grid) {
		v = gen(rnd);
	}
}

inline void Dump() {
	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 5; ++i) {
			std::cout << grid[j * 5 + i] << " ";
		}
		std::cout << std::endl;
	}
}

template<typename Iter>
inline std::pair<int, int> CalcLine(Iter&& cursor, Iter&& end) {
	auto symbol = grid[*cursor];
	int n = 1;
	// 如果 0 打头, 向后找出一个非 0 的来...
	if (!symbol) {
		while (++cursor != end) {
			if ((symbol = grid[*cursor])) break;
			++n;
		}
		if (n >= 3) {	// 3 个 0 优先判定
			symbol = 0;
			goto TheEnd;
		}
		++n;
	}
	// 继续数当前符号个数
	while (++cursor != end) {
		auto&& cs = grid[*cursor];
		if (cs && symbol != cs) break;
		++n;
	}
TheEnd:
	return std::make_pair(symbol, n);
}

inline void Calc() {
	for (int i = 0; i < lines.size(); ++i) {
		auto&& line = lines[i];
		auto&& r1 = CalcLine(line.begin(), line.end());
		auto&& r2 = CalcLine(line.rbegin(), line.rend());
		if (r1.second >= 3) {
			results.push_back(Result{ i, 0, r1.first, r1.second });
		}
		if (r2.second >= 3 && r1 != r2) {
			results.push_back(Result{ i, 1, r2.first, r2.second });
		}
	}
	// todo: 全屏特殊判断
}
int main() {
	Init();
	Fill();
	Dump();
	Calc();
	for (auto&& r : results) {
		r.Dump();
	}
}




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
