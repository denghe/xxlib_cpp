#include <xx_threadpool_ex.h>

struct Foo {
	Foo() = default;
	void operator()(std::function<void(Foo&)>& job)
	{
		try {
			job(*this);
		}
		catch (int const& e) {
			xx::CoutN("catch throw int: ", e);
		}
		catch (std::exception const& e) {
			xx::CoutN("catch throw std::exception: ", e);
		}
		catch (...) {
			xx::CoutN("catch ...");
		}
	}
};

int main() {
	xx::ThreadPoolEx<Foo> pool;
	pool.Add([](auto f) {
		xx::CoutN("asdf");
		});
	pool.Add([](auto f) {
		throw 123;
		});
	std::cin.get();
	return 0;
}
