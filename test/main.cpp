#include "static_lambda/static_lambda.hpp"
#include "static_lambda/detour_lambda.hpp"

struct Vector2
{
	float x, y;
};

bool target_func2(int a, Vector2 v)
{
	return v.x + v.y + a > 0;
}

int target_func(int a, int b)
{
	return a + b;
}

int main()
{
	{
		int result = target_func2(3, { 1.0f, 2.0f });
		int tmp = 0;
	}

	{
		int c = 1;

		auto replacement = [c](auto original, int a, Vector2 v) -> bool
		{
			return original(a, v) ? (c - 1) : (c + 1);
		};

		sl::detour<bool(int, Vector2)> a(&target_func2, replacement);

		int result = target_func2(3, { 1.0f, 2.0f });
		int tmp = 0;
	}

	{
		int c = 10;

		auto replacement = [c](auto original, int a, int b) -> int
		{
			return original(a, b) + c;
		};

		sl::detour<int(int, int)> a(&target_func, replacement);

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
