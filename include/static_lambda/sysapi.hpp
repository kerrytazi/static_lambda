#pragma once

#include <cstddef>

namespace _sl
{

#if defined(_MSC_VER)
extern "C" size_t _sl_get_rip();
size_t(*const _get_rip)() = &_sl_get_rip;
#else
size_t _get_rip();
#endif

void* _alloc(const void* target, size_t size);
void _free(void* ptr, size_t size);
void _protect(void* ptr, size_t size);

} // namespace _sl

