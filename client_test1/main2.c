#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

typedef struct Result {
	int index;
	int direction;
	int symbol;
	int count;
} Result_t;

static void CalcLine(int* symbol, int* count, int* cursor, int* grid) {
	int* end = cursor + 5;
	*symbol = grid[*cursor];
	*count = 1;
	if (!*symbol) {
		while (++cursor != end) {
			if ((*symbol = grid[*cursor])) break;
			++* count;
		}
		if (*count >= 3) {
			*symbol = 0;
			return;
		}
		++* count;
	}
	while (++cursor != end) {
		int s = grid[*cursor];
		if (s && *symbol != s) break;
		++* count;
	}
}
static void Calc(Result_t* results, int* resultsLen, int* grid, int* lines) {
	int lineIndex = 0;
	for (int i = 0; i < 5 * 9 * 2; i += 10) {
		int symbol1, count1, symbol2, count2;
		CalcLine(&symbol1, &count1, lines + i, grid);
		CalcLine(&symbol2, &count2, lines + i + 5, grid);
		if (count1 >= 3) {
			Result_t* r = &results[(*resultsLen)++];
			r->index = lineIndex;
			r->direction = 0;
			r->symbol = symbol1;
			r->count = count1;
		}
		if (count2 >= 3 && (symbol1 != symbol2/* && count1 != count2*/)) {
			Result_t* r = &results[(*resultsLen)++];
			r->index = lineIndex;
			r->direction = 1;
			r->symbol = symbol2;
			r->count = count2;
		}
		++lineIndex;
	}
}

int main() {
	int grid[] = { 6, 4, 0, 2, 4, 6, 1, 6, 4, 2, 1, 7, 0, 3, 4 };
	int lines[] = {
		5, 6, 7, 8, 9,
		9, 8, 7, 6, 5,

		0, 1, 2, 3, 4,
		4, 3, 2, 1, 0,

		10, 11, 12, 13, 14,
		14, 13, 12, 11, 10,

		0, 6, 12, 8, 4,
		4, 8, 12, 6, 0,

		10, 6, 2, 8, 14,
		14, 8, 2, 6, 10,

		0, 1, 7, 3, 4,
		4, 3, 7, 1, 0,

		10, 11, 7, 13, 14,
		14, 13, 7, 11, 10,

		5, 11, 12, 13, 9,
		9, 13, 12, 11, 5,

		5, 1, 2, 3, 9,
		9, 3, 2, 1, 5
	};
	Result_t results[18];
	int resultsLen = 0;

	clock_t currClock = clock();
	for (int i = 0; i < 10000000; ++i) {
		resultsLen = 0;
		Calc(results, &resultsLen, grid, lines);
	}
	double sec = (double)(clock() - currClock) / CLOCKS_PER_SEC;
	printf("%g\n", sec);

	for (int i = 0; i < resultsLen; ++i) {
		struct Result* r = results + i;
		printf("%d %d %d %d\n", r->index, r->direction, r->symbol, r->count);
	}
	return 0;
}
