#pragma once

#include <cstddef>

namespace _sl
{

extern size_t(*const _get_rip)();

void* _alloc(const void* target, size_t size);
void _free(void* ptr, size_t size);
void _protect(void* ptr, size_t size);

} // namespace _sl

