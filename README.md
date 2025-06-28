Helps convert C++ lambdas with captures to function pointers.

# Contents
- [static_lambda](#static_lambda)
- [detour_lambda](#detour_lambda)
- [How to import](#how-to-import)
  - [cmake](#cmake)
- [Build test project](#build-test-project)
  - [cmake](#cmake-1)
  - [Windows (Visual Studio)](#windows-visual-studio)
- [Currently supported platforms](#currently-supported-platforms)

# static_lambda
Lightweight way to convert lambdas to function pointers.

Current limitations:
- no arm support
- no copy-constructor

```cpp
#include "static_lambda/static_lambda.hpp"
```
```cpp
int c = 10;
sl::lambda<int(int, int)> a([c](int a, int b) -> int { return a + b + c; });

int (*ptr)(int, int) = a.get_static_pointer();

int result = ptr(3, 5); // 3 + 5 + 10 = 18
```

# detour_lambda
    Detour - a deviation from a direct course or the usual procedure.
    A hook (Or "detour") is called when a function is called, and they're used to change how the function works.

Lightweight way to detour functions with lambdas.

Current limitations:
- no thread safety
- no arm support
- no copy-constructor


```cpp
#include "static_lambda/detour_lambda.hpp"
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

int result = target_func(3, 5); // 3 + 5 + 10 = 18
```

# How to import

Go to your project directory
```
cd myproject
```

git clone
```
git clone https://github.com/kerrytazi/static_lambda.git lib/ccall
```

## cmake

```
add_subdirectory(lib/static_lambda)
target_link_libraries(myproject static_lambda)
```

# Build test project

## cmake

```
cmake -DSTATIC_LAMBDA_TEST=ON -B out
cmake --build out
```

## Windows (Visual Studio)

Add `cmake` component via `Visual Studio Installer`.

'Open a local folder' in root.

# Currently supported platforms

- Windows x86, x64
- Linux x86, x64
