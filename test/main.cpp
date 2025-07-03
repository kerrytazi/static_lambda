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

int target_func1(int a, int b)
{
	return a + b;
}

float target_func__default(int a, float b, Vector2 v) { return (float)a + v.x - b + v.y; }
float __cdecl target_func__cdecl(int a, float b, Vector2 v) { return (float)a + v.x - b + v.y; }
#if defined(_M_IX86) || defined(__i386__)
float __stdcall target_func__stdcall(int a, float b, Vector2 v) { return (float)a + v.x - b + v.y; }
float __fastcall target_func__fastcall(int a, float b, Vector2 v) { return (float)a + v.x - b + v.y; }
#endif // defined(_M_IX86) || defined(__i386__)
float __vectorcall target_func__vectorcall(int a, float b, Vector2 v) { return (float)a + v.x - b + v.y; }

#if defined(_M_IX86) || defined(__i386__)
struct target_class
{
	float b;
	Vector2 v;

	float __thiscall target_func__thiscall(int a) { return (float)a + v.x - b + v.y; }
	float __thiscall target_func__thiscall_const(int a) const { return (float)a + v.x - b + v.y; }
};
#endif // defined(_M_IX86) || defined(__i386__)

int main()
{
	{
		const char* target_name = "target_func__default";
		volatile auto target = &target_func__default;

		{
			int c = 1;

			auto replacement = [c](int a, float b, Vector2 v, auto original) -> float
			{
				return original(a, b, v) + (float)c;
			};

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " before (2): " << result << "\n";
			}

			sl::detour<float(int, float, Vector2)> a(target, replacement);

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " after  (3): " << result << "\n";
			}
		}
	}
	{
		const char* target_name = "target_func__cdecl";
		volatile auto target = &target_func__cdecl;

		{
			int c = 1;

			auto replacement = [c](int a, float b, Vector2 v, auto original) -> float
			{
				return original(a, b, v) + (float)c;
			};

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " before (2): " << result << "\n";
			}

			sl::detour<float(int, float, Vector2)> a(target, replacement);

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " after  (3): " << result << "\n";
			}
		}
	}
#if defined(_M_IX86) || defined(__i386__)
	{
		const char* target_name = "target_func__stdcall";
		volatile auto target = &target_func__stdcall;

		{
			int c = 1;

			auto replacement = [c](int a, float b, Vector2 v, auto original) -> float
			{
				return original(a, b, v) + (float)c;
			};

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " before (2): " << result << "\n";
			}

			sl::detour<float __stdcall(int, float, Vector2)> a(target, replacement);

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " after  (3): " << result << "\n";
			}
		}
	}
	{
		const char* target_name = "target_func__fastcall";
		volatile auto target = &target_func__fastcall;

		{
			int c = 1;
	
			auto replacement = [c](int a, float b, Vector2 v, auto original) -> float
			{
				return original(a, b, v) + (float)c;
			};

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " before (2): " << result << "\n";
			}

			sl::detour<float __fastcall(int, float, Vector2)> a(target, replacement);

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " after  (3): " << result << "\n";
			}
		}
	}
#endif // defined(_M_IX86) || defined(__i386__)
	{
		const char* target_name = "target_func__vectorcall";
		volatile auto target = &target_func__vectorcall;

		{
			int c = 1;

			auto replacement = [c](int a, float b, Vector2 v, auto original) -> float
			{
				return original(a, b, v) + (float)c;
			};

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " before (2): " << result << "\n";
			}

			sl::detour<float __vectorcall(int, float, Vector2)> a(target, replacement);

			{
				float result = target(3, 4.0f, { 1.0f, 2.0f });
				std::cout << target_name << " after  (3): " << result << "\n";
			}
		}
	}
#if defined(_M_IX86) || defined(__i386__)
	{
		const char* target_name = "target_func__thiscall";
		volatile auto target = &target_class::target_func__thiscall;
		target_class tc{ .b = 4.0f, .v = { 1.0f, 2.0f }};

		{
			int c = 1;

			auto replacement = [c](target_class* _this, int a, auto original) -> float
			{
				return (*_this.*original)(a) + (float)c;
			};

			{
				float result = (tc.*target)(3);
				std::cout << target_name << " before (2): " << result << "\n";
			}

			sl::detour<float __thiscall(target_class*, int)> a(target, replacement);

			{
				float result = (tc.*target)(3);
				std::cout << target_name << " after  (3): " << result << "\n";
			}
		}
	}
	{
		const char* target_name = "target_func__thiscall_const";
		volatile auto target = &target_class::target_func__thiscall_const;
		const target_class tc{ .b = 4.0f, .v = { 1.0f, 2.0f }};

		{
			int c = 1;

			auto replacement = [c](const target_class* _this, int a, auto original) -> float
			{
				return (*_this.*original)(a) + (float)c;
			};

			{
				float result = (tc.*target)(3);
				std::cout << target_name << " before (2): " << result << "\n";
			}

			sl::detour<float __thiscall(const target_class*, int)> a(target, replacement);

			{
				float result = (tc.*target)(3);
				std::cout << target_name << " after  (3): " << result << "\n";
			}
		}
	}
#endif // defined(_M_IX86) || defined(__i386__)

	{
		int result = target_func1(3, 5);
		std::cout << "target_func (8): " << result << "\n";
		int tmp = 0;
	}

	{
		int result = target_func2(3, { 1.0f, 2.0f });
		std::cout << "target_func2 (1): " << result << "\n";
		int tmp = 0;
	}

	{
		int c = 1;

		auto replacement = [c](int a, Vector2 v, auto original) -> bool
		{
			return original(a, v) ? (c - 1) : (c + 1);
		};

		bool(*volatile target)(int, Vector2) = &target_func2;

		sl::detour<bool(int, Vector2)> a(target, replacement);

		int result = target(3, { 1.0f, 2.0f });
		std::cout << "target_func2 (0): " << result << "\n";
		int tmp = 0;
	}

	{
		int c = 10;

		auto replacement = [c](int a, int b, auto original) -> int
		{
			return original(a, b) + c;
		};

		int(*volatile target)(int, int) = &target_func1;

		sl::detour<int(int, int)> a(target, replacement);

		int result = target(3, 5);
		std::cout << "target_func (18): " << result << "\n";
		int tmp = 0;
	}

	{
		int c = 10;

		auto replacement = [c](int a, int b, auto original) -> int
		{
			return original(a, b) + c;
		};

		volatile void* target_tmp = (void*)&target_func1;
		sl::detour<int(int, int)> a(target_tmp, replacement);

		int(* volatile target)(int, int) = &target_func1;
		int result = ((int(*volatile)(int, int))target)(3, 5);
		std::cout << "target_func (18): " << result << "\n";
		int tmp = 0;
	}

	{
		int c = 10;
		sl::lambda<int(int, int)> a([c](int a, int b) -> int { return a + b + c; });

		int (*ptr)(int, int) = a.get_static_pointer();

		int result = ptr(3, 5);
		std::cout << "target_func (18): " << result << "\n";
		int tmp = 0;
	}

	{
		int c = 10;

		auto replacement = [c](int a, int b, auto original) -> int
		{
			return original(a, b) + c;
		};

		int(*volatile target)(int, int) = &target_func3;

		std::cout << "target_func3 before (26): " << target(3, 5) << "\n";

		sl::detour<int(int, int)> a(target, replacement);

		int result = target(3, 5);
		std::cout << "target_func3 after  (36): " << result << "\n";
		int tmp = 0;
	}
}
