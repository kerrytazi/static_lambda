#pragma once

#include <cstdint>
#include <cstddef>

namespace _sl
{

void* _alloc(const void* target, size_t size);
void _free(void* ptr, size_t size);
void _protect(void* ptr, size_t size);

} // namespace _sl

