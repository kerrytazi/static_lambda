#pragma once

#include <cstdint>
#include <cstddef>

namespace _sl
{


size_t _get_rax();
void* _alloc(void* target, size_t size);
void _free(void* ptr, size_t size);
void _protect(void* ptr, size_t size);


} // namespace _sl
