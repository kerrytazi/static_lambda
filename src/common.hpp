#pragma once

#include "sysapi.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>
#include <cstring>

#if defined(_MSC_VER)
#define _SL_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#define _SL_NO_INLINE __attribute__((noinline))
#else
#define _SL_NO_INLINE
#pragma warning("_SL_NO_INLINE not supported for this compiler")
#endif

namespace _sl
{

struct CAligned
{
	const void* mem;
	size_t size;
};

inline
CAligned align_mem_down_to(const void* mem, size_t size, size_t align)
{
	size_t mask = align - 1;
	size_t memu = reinterpret_cast<size_t>(mem);

	if (memu & mask)
	{
		mem = reinterpret_cast<void* >(memu - (memu & mask));
		size = size + (memu & mask);
	}

	return CAligned { .mem = mem, .size = size };
}

inline
const void* unjump(const void* ptr)
{
	const uint8_t* t = static_cast<const uint8_t*>(ptr);

	while (*t == 0xE9)
	{
		int offset = 0;
		memcpy(&offset, t + 1, sizeof(offset));
		t = t + 5 + offset;
	}

	return static_cast<const void*>(t);
}

struct alignas(16) smem_base
{
	uint8_t trampoline[256];
	uint8_t original[64];
	uint8_t save_target_code[64];
	void* save_target;
	size_t save_target_size;
	size_t alloc_size;
	void (*destroy)(void* fl);
};

template <typename FL>
struct smem : smem_base
{
	FL func;
};

template <typename F>
struct helper;

template <typename TRet, typename... TArgs>
struct helper<TRet(TArgs...)>
{
	using function_pointer_type = TRet (*)(TArgs...);

	using function_pointer_detour_type = TRet (*)(function_pointer_type, TArgs...);

	template <typename TOther>
	static constexpr bool is_compatible = std::is_same_v<decltype(std::declval<TOther>()(std::declval<TArgs>()...)), TRet>;

	template <typename TOther>
	static constexpr bool is_compatible_detour = std::is_same_v<decltype(std::declval<TOther>()(std::declval<function_pointer_type>(), std::declval<TArgs>()...)), TRet>;

	template <typename FL>
	struct proxy
	{
		static TRet func(TArgs... args)
		{
			// _get_rip must be called inside this function
			auto mem = reinterpret_cast<smem<FL> *>(_sl::_get_rip() & (~size_t(0x0FFF)));
			// noinline, otherwise compiler may inline lambda call.
			// but we need to keep this function small for faster copy
			return call(args..., mem);
		}

		static TRet func_detour(TArgs... args)
		{
			// _get_rip must be called inside this function
			auto mem = reinterpret_cast<smem<FL>*>(_sl::_get_rip() & (~size_t(0x0FFF)));
			// noinline, otherwise compiler may inline lambda call.
			// but we need to keep this function small for faster copy
			return call_detour(mem, args...);
		}

		_SL_NO_INLINE
		static TRet call(TArgs... args, smem<FL>* mem)
		{
			return mem->func(args...);
		}

		_SL_NO_INLINE
		static TRet call_detour(smem<FL>* mem, TArgs... args)
		{
			function_pointer_type original = reinterpret_cast<function_pointer_type>(static_cast<void*>(mem->original));
			return mem->func(original, args...);
		}

		static void destroy(FL* func)
		{
			func->~FL();
		}
	};
};

template <typename T>
struct detour_tag {};


} // namespace sl
