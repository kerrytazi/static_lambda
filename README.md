Helps convert C++ lambdas with captures to function pointers.

# Contents
- [static_lambda](#static_lambda)
- [detour_lambda](#detour_lambda)
- [Requirements](#requirements-visual-studio)

# static_lambda
Lightweight way to convert lambdas to function pointers.

Current limitations:
- windows x86_64 only
- no copy-constructor

```cpp
#include "static_lambda.hpp"
```
```cpp
int c = 10;
sl::lambda<int(int, int)> a([c](int a, int b) -> int { return a + b + c; });

int (*ptr)(int, int) = a.get_static_pointer();

int result = ptr(3, 5);
```

# detour_lambda
    Detour - a deviation from a direct course or the usual procedure.
    A hook (Or "detour") is called when a function is called, and they're used to change how the function works.

Lightweight way to detour functions with lambdas.

Current limitations:
- UNSAFE AS F***
- windows x86_64 only
- no copy-constructor


```cpp
#include "detour_lambda.hpp"
```
```cpp
int target_func(int a, int b)
{
    return a + b;
}
```
```cpp
int c = 10;

auto replacement = [c](auto original, int a, int b) -> int
{
    return original(a, b) + c;
};

sl::detour<int(int, int)> a(&target_func, replacement);

int result = target_func(3, 5);
```

# Requirements (Visual Studio)
- [c++ 20](#c-20)
- [Enable masm](#enable-masm)
- [Disable JMC](#disable-jmc)

## c++ 20
Project -> Properties -> Configuration Properties -> General -> C++ Language Standart -> ISO C++20 Standard (/std:c++20)

## Enable masm
Project -> Build Dependencies -> Build Customizations... -> masm (checkbox on)

File slwinapi.asm -> Properties -> General -> Item Type -> Microsoft Macro Assembler

## Disable JMC
Project -> Properties -> Configuration Properties -> C/C++ -> General -> Support Just My Code Debugging -> No
