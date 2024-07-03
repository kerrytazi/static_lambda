#pragma once

#include "sysapi.hpp"

#if 0
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
#ifdef _WIN32
inline void *operator new (unsigned long long, void *p) noexcept
#else
inline void *operator new (unsigned long, void *p) noexcept
#endif // _WIN32
{
	return p;
}

inline void operator delete(void *, void *) noexcept
{
	return;
}
#endif // __PLACEMENT_NEW_INLINE
#endif // 0

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

template <typename T>
constexpr bool is_lvalue_reference_v = false;

template <typename T>
constexpr bool is_lvalue_reference_v<T &> = true;

template <typename A, typename B>
constexpr bool is_same = false;

template <typename A>
constexpr bool is_same<A, A> = true;


template <typename T>
constexpr bool always_false = false;


template <class _Ty>
[[nodiscard]]
constexpr _Ty &&forward(remove_reference_t<_Ty> &_Arg)
{
	return static_cast<_Ty &&>(_Arg);
}

template <class _Ty>
[[nodiscard]] constexpr _Ty &&forward(remove_reference_t<_Ty> &&_Arg)
{
	static_assert(!is_lvalue_reference_v<_Ty>, "bad forward call");
	return static_cast<_Ty &&>(_Arg);
}

template <typename T, typename... TArgs>
constexpr T *construct_at(void *ptr, TArgs&&... args)
{
	struct new_helper
	{
		constexpr new_helper(TArgs&&... args)
			: val{ forward<TArgs>(args)... }
		{
		}

#ifdef _MSC_VER
		constexpr void *operator new(unsigned long long, void *ptr)
#else
		constexpr void *operator new(unsigned long, void *ptr)
#endif // _MSC_VER
		{
			return ptr;
		}

		T val;
	};

	return &(new (ptr) new_helper(forward<TArgs>(args)...))->val;
}

template <typename T>
constexpr T declval() { static_assert(always_false<T>, "declval not allowed in an evaluated context"); }

struct CAligned
{
	void *mem;
	unsigned long long size;
};

inline CAligned align_mem_down_to(void *mem, unsigned long long size, unsigned long long align)
{
	unsigned long long mask = align - 1;
	unsigned long long memu = reinterpret_cast<unsigned long long>(mem);

	if (memu & mask)
	{
		mem = reinterpret_cast<void *>(memu - (memu & mask));
		size = size + (memu & mask);
	}

	return CAligned { .mem = mem, .size = size };
}

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

struct alignas(16) smem_base
{
	unsigned char trampoline[64];
	unsigned char original[64];
	unsigned char save_target_code[64];
	void *save_target;
	unsigned long long save_target_size;
	unsigned long long alloc_size;
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
			smem<FL> *mem = reinterpret_cast<smem<FL> *>(_sl::_get_rax());
			return mem->func(args...);
		}

		static TRet func_detour(TArgs... args)
		{
			smem<FL> *mem = reinterpret_cast<smem<FL> *>(_sl::_get_rax());
			function_pointer_type original = reinterpret_cast<function_pointer_type>(static_cast<void *>(mem->original));
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
