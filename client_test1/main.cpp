#include <iostream>

template<typename T>
struct B : T {
	B() : T() {
		this->Xxx();
	}
};
struct A {
	inline void Xxx() {
		std::cout << "xxx";
	}
};
struct C : B<A> {
};

int main() {
	C c;
	return 0;
}
