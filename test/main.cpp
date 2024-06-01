#include "../static_lambda/static_lambda.hpp"

int target_func(int a, int b)
{
	return a + b;
}

void _start()
{
	int c = 10;
	sl::lambda<int(int, int)> a([c](int a, int b) -> int { return a + b + c; });

	int (*ptr)(int, int) = a.get_static_pointer();

	int result = ptr(3, 5);
}
