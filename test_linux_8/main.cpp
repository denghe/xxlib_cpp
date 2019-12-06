#include <cstddef>
#include <functional>
struct A {
	A() {}
	A(int a) {}
	A(int a, int b) {}
};
struct B : A {
	using B::A::A;
};

int main(int argc, char** argv) {
	return 0;
}
