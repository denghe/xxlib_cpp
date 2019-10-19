#include <stdlib.h>

int* ints = nullptr;
int xxx(int len, int idx) {
	ints = (int*)malloc(sizeof(int) * len);
	return idx;
}
int main() {
	ints[xxx(1, 0)] = 123;	// crash on gcc
	return 0;
}
