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

namespace _sl
{


const unsigned long long DESTROY_OFFSET = 64;
const unsigned long long SAVE_TARGET_CODE_OFFSET = 128;
const unsigned long long SAVE_TARGET_OFFSET = SAVE_TARGET_CODE_OFFSET + 16;
const unsigned long long SAVE_TARGET_SIZE_OFFSET = SAVE_TARGET_OFFSET + 8;
const unsigned long long COPY_OF_ORIGINAL_OFFSET = SAVE_TARGET_SIZE_OFFSET + 8;
const unsigned long long FL_OFFSET = 256;


template <typename T>
struct remove_reference { using type = T; };

template <typename T>
struct remove_reference<T &> { using type = T; };

template <typename T>
struct remove_reference<T &&> { using type = T; };

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;


template <typename A, typename B>
constexpr bool is_same = false;

template <typename A>
constexpr bool is_same<A, A> = true;


template <typename T>
constexpr bool always_false = false;


template <typename T>
constexpr T declval() { static_assert(always_false<T>, "declval not allowed in an evaluated context"); }

inline
void memcpy(void *dst, void const *src, unsigned long long size)
{
	unsigned char *_dst = static_cast<unsigned char *>(dst);
	unsigned char const *_src = static_cast<unsigned char const *>(src);
	unsigned char const *end = _src + size;

	while (_src != end)
		*_dst++ = *_src++;
}

inline
void *unjump(void *ptr)
{
	unsigned char *t = static_cast<unsigned char *>(ptr);

	while (*t == 0xE9)
	{
		int offset = 0;
		memcpy(&offset, t + 1, sizeof(offset));
		t = t + 5 + offset;
	}

	return static_cast<void *>(t);
}

template <typename F>
struct helper;

template <typename TRet, typename... TArgs>
struct helper<TRet(TArgs...)>
{
	using function_pointer_type = TRet (*)(TArgs...);

	using function_pointer_detour_type = TRet (*)(function_pointer_type, TArgs...);

	template <typename TOther>
	static constexpr bool is_compatible = is_same<decltype(declval<TOther>()(declval<TArgs>()...)), TRet>;

	template <typename TOther>
	static constexpr bool is_compatible_detour = is_same<decltype(declval<TOther>()(declval<function_pointer_type>(), declval<TArgs>()...)), TRet>;

	template <typename FL>
	struct proxy
	{
		static TRet func(TArgs... args)
		{
			unsigned char *ptr = reinterpret_cast<unsigned char *>(_slwinapi::_sl_get_rax());
			FL *func = reinterpret_cast<FL *>(ptr + FL_OFFSET);
			return (*func)(args...);
		}

		static TRet func_detour(TArgs... args)
		{
			unsigned char *mem = reinterpret_cast<unsigned char *>(_slwinapi::_sl_get_rax());
			FL *func = reinterpret_cast<FL *>(mem + FL_OFFSET);
			function_pointer_type original = reinterpret_cast<function_pointer_type>(mem + COPY_OF_ORIGINAL_OFFSET);
			return (*func)(original, args...);
		}

		static void destroy(FL *func)
		{
			func->~FL();
		}
	};
};

template <typename T>
struct detour_tag {};


} // namespace sl
