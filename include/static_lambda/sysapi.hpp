#pragma once

namespace _sl
{


unsigned long long _get_rax();
void *_alloc(void *target, unsigned long long size);
void _free(void *ptr, unsigned long long size);
void _protect(void *ptr, unsigned long long size);


} // namespace _sl
