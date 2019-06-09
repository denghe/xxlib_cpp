#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <array>

inline void CalcLine(int& symbol, int& count, int const* cursor, std::array<int, 15> const& grid) noexcept {
	symbol = grid[*cursor];
	count = 1;
	auto end = cursor + 5;
	if (!symbol) {
		while (++cursor != end) {
			if ((symbol = grid[*cursor])) break;
			++count;
		}
		if (count >= 3) {
			symbol = 0;
			return;
		}
		++count;
	}
	while (++cursor != end) {
		int s = grid[*cursor];
		if (s && symbol != s) break;
		++count;
	}
}
inline void CalcLineReverse(int& symbol, int& count, int const* cursor, std::array<int, 15> const& grid) noexcept {
	symbol = grid[*cursor];
	count = 1;
	auto end = cursor - 5;
	if (!symbol) {
		while (--cursor != end) {
			if ((symbol = grid[*cursor])) break;
			++count;
		}
		if (count >= 3) {
			symbol = 0;
			return;
		}
		++count;
	}
	while (--cursor != end) {
		int s = grid[*cursor];
		if (s && symbol != s) break;
		++count;
	}
}

struct Result {
	int index;
	int direction;
	int symbol;
	int count;
};

inline void Calc(Result* results, int& resultsLen, std::array<int, 15> const& grid, std::array<std::array<int, 5>, 9> const& lines) noexcept {
	int s1, c1, s2, c2;
	for (size_t i = 0; i < lines.size(); ++i) {
		CalcLine(s1, c1, lines[i].data(), grid);
		CalcLineReverse(s2, c2, lines[i].data() + lines[i].size() - 1, grid);
		if (c1 >= 3) {
			auto&& r = results[resultsLen++];
			r.index = (int)i;
			r.direction = 0;
			r.symbol = s1;
			r.count = c1;
		}
		if (c2 >= 3 && (s1 != s2/* && c1 != c2*/)) {
			auto&& r = results[resultsLen++];
			r.index = (int)i;
			r.direction = 1;
			r.symbol = s2;
			r.count = c2;
		}
	}
}

int main() {
	std::array<int, 15> grid = {
		6, 4, 0, 2, 4,
		6, 1, 6, 4, 2,
		1, 7, 0, 3, 4
	};
	std::array<std::array<int, 5>, 9> lines = {
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
	Result results[18];
	int resultsLen = 0;

	auto currClock = (int64_t)clock();
	for (int i = 0; i < 10000000; ++i) {
		resultsLen = 0;
		Calc(results, resultsLen, grid, lines);
	}
	auto sec = (double)((int64_t)clock() - currClock) / CLOCKS_PER_SEC;
	printf("%g\n", sec);

	for (int i = 0; i < resultsLen; ++i) {
		auto r = results + i;
		printf("%d %s %d %d\n", r->index, (r->direction ? "<--" : "-->"), r->symbol, r->count);
	}
	return 0;
}
