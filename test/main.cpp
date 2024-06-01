#include "../static_lambda/static_lambda.hpp"
#include "../static_lambda/detour_lambda.hpp"

int target_func(int a, int b)
{
	return a + b;
}

void _start()
{
	{
		int c = 10;

		int (*original)(int, int) = nullptr;

		auto replacement = [c, &original](int a, int b) -> int
		{
			return original(a, b) + c;
		};

		sl::detour<int(int, int)> a(&target_func, &original, replacement);

		int result = target_func(3, 5);


		int tmp = 0;
	}

	{
		int c = 10;
		sl::lambda<int(int, int)> a([c](int a, int b) -> int { return a + b + c; });

		int (*ptr)(int, int) = a.get_static_pointer();

		int result = ptr(3, 5);
		int tmp = 0;
	}
}
