#pragma once

#include "sysapi.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>
#include <cstring>
#include <bit>

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

inline
void replace_pattern(smem_base* mem)
{
	auto pattern = intptr_t(0x0123456789abcdef);
	auto p = (uint8_t*)&pattern;

	auto t = mem->trampoline;

	bool found_pattern = false;

	for (size_t i = 0; i < sizeof(mem->original); ++i)
	{
		bool found = true;

		for (size_t j = 0; j < sizeof(pattern) && i + j < sizeof(mem->original); ++j)
		{
			if (t[i + j] != p[j])
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			t += i;
			found_pattern = true;
			break;
		}
	}

	if (!found_pattern)
		throw 1;

	auto mem_ptr = intptr_t(mem);
	auto m = (uint8_t*)&mem_ptr;

	memcpy(t, m, sizeof(pattern));
}

template <typename F>
struct helper;

#define HELPER_IMPL(CALLING_CONVENTION) \
template <typename TRet, typename... TArgs> \
struct helper<TRet CALLING_CONVENTION (TArgs...)> \
{ \
	using function_pointer_type = TRet (CALLING_CONVENTION*)(TArgs...); \
\
	template <typename TOther> \
	static constexpr bool is_compatible = std::is_same_v<decltype(std::declval<TOther>()(std::declval<TArgs>()...)), TRet>; \
\
	template <typename TOther> \
	static constexpr bool is_compatible_detour = std::is_same_v<decltype(std::declval<TOther>()(std::declval<TArgs>()..., std::declval<function_pointer_type>())), TRet>; \
\
	template <typename FL> \
	struct proxy \
	{ \
		static TRet CALLING_CONVENTION func(TArgs... args) \
		{ \
			volatile auto mem = reinterpret_cast<smem<FL>*>(intptr_t(0x0123456789abcdef)); \
			return call(mem, args...); \
		} \
\
		static TRet CALLING_CONVENTION func_detour(TArgs... args) \
		{ \
			volatile auto mem = reinterpret_cast<smem<FL>*>(intptr_t(0x0123456789abcdef)); \
			return call_detour(mem, args...); \
		} \
\
		_SL_NO_INLINE \
		static TRet call(smem<FL>* mem, TArgs... args) \
		{ \
			return mem->func(args...); \
		} \
\
		_SL_NO_INLINE \
		static TRet call_detour(smem<FL>* mem, TArgs... args) \
		{ \
			function_pointer_type original = std::bit_cast<function_pointer_type>(&mem->original); \
			return mem->func(args..., original); \
		} \
\
		static void destroy(FL* func) \
		{ \
			func->~FL(); \
		} \
	}; \
}

HELPER_IMPL(__cdecl);
#if defined(_M_IX86) || defined(__i386__)
HELPER_IMPL(__stdcall);
HELPER_IMPL(__fastcall);
#endif // defined(_M_IX86) || defined(__i386__)
HELPER_IMPL(__vectorcall);

#undef HELPER_IMPL

#if defined(_M_IX86) || defined(__i386__)
#define HELPER_IMPL_CLASS(_CLASS, _CONST) \
template <typename TRet, typename _CLASS, typename... TArgs> \
struct helper<TRet __thiscall (_CONST _CLASS*, TArgs...)> \
{ \
	using function_pointer_type = TRet (__thiscall _CLASS::*)(TArgs...) _CONST; \
\
	template <typename TOther> \
	static constexpr bool is_compatible = std::is_same_v<decltype(std::declval<TOther>()(std::declval<_CONST _CLASS*>(), std::declval<TArgs>()...)), TRet>; \
\
	template <typename TOther> \
	static constexpr bool is_compatible_detour = std::is_same_v<decltype(std::declval<TOther>()(std::declval<_CONST _CLASS*>(), std::declval<TArgs>()..., std::declval<function_pointer_type>())), TRet>; \
\
	template <typename FL> \
	struct proxy \
	{ \
		TRet __thiscall func(TArgs... args) _CONST \
		{ \
			volatile auto mem = reinterpret_cast<smem<FL>*>(intptr_t(0x0123456789abcdef)); \
			return call(mem, std::bit_cast<_CONST _CLASS*>(this), args...); \
		} \
\
		TRet __thiscall func_detour(TArgs... args) _CONST \
		{ \
			volatile auto mem = reinterpret_cast<smem<FL>*>(intptr_t(0x0123456789abcdef)); \
			return call_detour(mem, std::bit_cast<_CONST _CLASS*>(this), args...); \
		} \
\
		_SL_NO_INLINE \
		static TRet call(smem<FL>* mem, _CONST _CLASS* _this, TArgs... args) \
		{ \
			return mem->func(_this, args...); \
		} \
\
		_SL_NO_INLINE \
		static TRet call_detour(smem<FL>* mem, _CONST _CLASS* _this, TArgs... args) \
		{ \
			function_pointer_type original = std::bit_cast<function_pointer_type>(&mem->original); \
			return mem->func(_this, args..., original); \
		} \
\
		static void destroy(FL* func) \
		{ \
			func->~FL(); \
		} \
	}; \
}

HELPER_IMPL_CLASS(TClass,);
HELPER_IMPL_CLASS(TClass, const);

#undef HELPER_IMPL_CLASS
#endif // defined(_M_IX86) || defined(__i386__)

template <typename T>
struct detour_tag {};


} // namespace sl
