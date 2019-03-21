#include "xx_dict.h"
struct Foo;
std::unordered_map<int, std::shared_ptr<Foo>> foos;
struct Foo {
	int id;
	Foo(int const& id) : id(id) {}
	~Foo() {
		foos.erase(id);
	}
};

struct Bar;
xx::Dict<int, std::shared_ptr<Bar>> bars;
struct Bar {
	int id;
	Bar(int const& id) : id(id) {}
	~Bar() {
		bars.Remove(id);
	}
};


int main(int argc, char* argv[]) {
	foos[1] = std::make_shared<Foo>(1);
	foos[2] = std::make_shared<Foo>(2);
	foos[3] = std::make_shared<Foo>(3);
	for (auto&& iter = foos.begin(); iter != foos.end();) {
		(iter++)->second.reset();
	}
	xx::CoutN(foos.size());

	bars[1] = std::make_shared<Bar>(1);
	bars[2] = std::make_shared<Bar>(2);
	bars[3] = std::make_shared<Bar>(3);
	for (auto&& kv : bars) {
		kv.value.reset();
	}
	xx::CoutN(bars.Count());
	return 0;
}
