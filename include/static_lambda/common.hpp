#pragma once

#include "slwinapi.hpp"

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
constexpr inline void *operator new (unsigned long long, void *p) noexcept
{
	return p;
}

constexpr inline void operator delete(void *, void *) noexcept
{
	return;
}
#endif // __PLACEMENT_NEW_INLINE

namespace sl
{

const unsigned long long _DESTROY_OFFSET = 64;
const unsigned long long _SAVE_TARGET_CODE_OFFSET = 128;
const unsigned long long _SAVE_TARGET_OFFSET = _SAVE_TARGET_CODE_OFFSET + 16;
const unsigned long long _SAVE_TARGET_SIZE_OFFSET = _SAVE_TARGET_OFFSET + 8;
const unsigned long long _COPY_OF_ORIGINAL_OFFSET = _SAVE_TARGET_SIZE_OFFSET + 8;
const unsigned long long _FL_OFFSET = 256;


template <typename T>
struct _remove_reference { using type = T; };

template <typename T>
struct _remove_reference<T &> { using type = T; };

template <typename T>
struct _remove_reference<T &&> { using type = T; };

template <typename T>
using _remove_reference_t = typename _remove_reference<T>::type;


template <typename A, typename B>
constexpr bool _is_same = false;

template <typename A>
constexpr bool _is_same<A, A> = true;


template <typename T>
constexpr bool _always_false = false;


template <typename T>
constexpr T _declval() { static_assert(_always_false<T>, "declval not allowed in an evaluated context"); }

void _memcpy(void *dst, void const *src, unsigned long long size)
{
	unsigned char *_dst = static_cast<unsigned char *>(dst);
	unsigned char const *_src = static_cast<unsigned char const *>(src);
	unsigned char const *end = _src + size;

	while (_src != end)
		*_dst++ = *_src++;
}

template <typename F>
struct _helper;

template <typename TRet, typename... TArgs>
struct _helper<TRet(TArgs...)>
{
	using function_pointer_type = TRet (*)(TArgs...);

	using function_pointer_detour_type = TRet (*)(function_pointer_type, TArgs...);

	template <typename TOther>
	static constexpr bool is_compatible = _is_same<decltype(_declval<TOther>()(_declval<TArgs>()...)), TRet>;

	template <typename TOther>
	static constexpr bool is_compatible_detour = _is_same<decltype(_declval<TOther>()(_declval<function_pointer_type>(), _declval<TArgs>()...)), TRet>;

	template <typename FL>
	struct proxy
	{
		static TRet func(TArgs... args)
		{
			unsigned char *ptr = reinterpret_cast<unsigned char *>(_slwinapi::_sl_get_rax());
			FL *func = reinterpret_cast<FL *>(ptr + _FL_OFFSET);
			return (*func)(args...);
		}

		static TRet func_detour(TArgs... args)
		{
			unsigned char *mem = reinterpret_cast<unsigned char *>(_slwinapi::_sl_get_rax());
			FL *func = reinterpret_cast<FL *>(mem + _FL_OFFSET);
			function_pointer_type original = reinterpret_cast<function_pointer_type>(mem + _COPY_OF_ORIGINAL_OFFSET);
			return (*func)(original, args...);
		}

		static void destroy(FL *func)
		{
			func->~FL();
		}
	};
};


struct _detour_tag {};


} // namespace sl
