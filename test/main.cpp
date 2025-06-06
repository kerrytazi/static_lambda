#include "static_lambda/static_lambda.hpp"
#include "static_lambda/detour_lambda.hpp"

#include <iostream>

struct Vector2
{
	float x, y;
};

int g_val1 = 7;
int g_val2 = 11;

int target_func3(int a, int b)
{
	return g_val1 + a + b + g_val2;
}

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
		int result = target_func(3, 5);
		std::cout << "target_func: " << result << "\n";
		int tmp = 0;
	}

	{
		int result = target_func2(3, { 1.0f, 2.0f });
		std::cout << "target_func2: " << result << "\n";
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
		std::cout << "target_func2: " << result << "\n";
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
		std::cout << "target_func: " << result << "\n";
		int tmp = 0;
	}

	{
		int c = 10;
		sl::lambda<int(int, int)> a([c](int a, int b) -> int { return a + b + c; });

		int (*ptr)(int, int) = a.get_static_pointer();

		int result = ptr(3, 5);
		std::cout << "target_func: " << result << "\n";
		int tmp = 0;
	}

	{
		int c = 10;

		auto replacement = [c](auto original, int a, int b) -> int
		{
			return original(a, b) + c;
		};

		int(*volatile target)(int, int) = &target_func3;

		std::cout << "target_func3 before: " << target(3, 5) << "\n";

		sl::detour<int(int, int)> a(target, replacement);

		int result = target(3, 5);
		std::cout << "target_func3 after:  " << result << "\n";
		int tmp = 0;
	}
}
