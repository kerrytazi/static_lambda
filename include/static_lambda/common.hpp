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

struct smem_base
{
	unsigned char trampoline[64];
	unsigned char original[64];
	unsigned char save_target_code[64];
	void *save_target;
	unsigned long long save_target_size;
	void (*destroy)(void *fl);
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
	static constexpr bool is_compatible = is_same<decltype(declval<TOther>()(declval<TArgs>()...)), TRet>;

	template <typename TOther>
	static constexpr bool is_compatible_detour = is_same<decltype(declval<TOther>()(declval<function_pointer_type>(), declval<TArgs>()...)), TRet>;

	template <typename FL>
	struct proxy
	{
		static TRet func(TArgs... args)
		{
			smem<FL> *mem = reinterpret_cast<smem<FL> *>(_slwinapi::_sl_get_rax());
			return mem->func(args...);
		}

		static TRet func_detour(TArgs... args)
		{
			smem<FL> *mem = reinterpret_cast<smem<FL> *>(_slwinapi::_sl_get_rax());
			function_pointer_type original = static_cast<function_pointer_type>(static_cast<void *>(mem->original));
			return mem->func(original, args...);
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
