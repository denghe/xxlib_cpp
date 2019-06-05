#include <iostream>
#include <stdint.h>
#include <string>

template<typename T>
void MyPrint(T const& v) {
	if constexpr (std::is_integral_v<T>) {
		printf("%lld\n", (int64_t)v);
	}
	else if constexpr (std::is_floating_point_v<T>) {
		printf("%g\n", (double)v);
	}
}

int main() {
	MyPrint(12345);
	MyPrint(1.234);
}
