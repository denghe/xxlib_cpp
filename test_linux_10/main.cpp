#include "xx_bbuffer.h"
int main(int argc, char* argv[]) {
	xx::BBuffer bb;
	std::vector<int> ints = { 1,2,3,4,5 };
	bb.Write(ints);
	xx::CoutN(bb);
	std::vector<int> ints2;
	bb.Read(ints2);
	xx::CoutN(ints2);

	return 0;
}
