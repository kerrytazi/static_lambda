#pragma once

#include <cstddef>

namespace sl
{

[[nodiscard]]
void* detour_static_create(void** inout_target, const void* patch, size_t extra_size = 0);
void detour_static_enable(void** inout_target);
void detour_static_disable(void** inout_target);
void detour_static_destroy(void** inout_target);

} // namespace sl
