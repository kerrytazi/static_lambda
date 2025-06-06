#pragma once

#include "sysapi.hpp"

#include <type_traits>
#include <utility>
#include <cstring>


namespace _sl
{

void* _DetourCopyInstructionX64(void* pDst, void** ppDstPool, void* pSrc, void** ppTarget, long* plExtra);
bool _detour_does_code_end_function(const unsigned char*  pbCode);

struct CAligned
{
	void* mem;
	size_t size;
};

inline
CAligned align_mem_down_to(void* mem, size_t size, size_t align)
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
void* unjump(void* ptr)
{
	uint8_t* t = static_cast<uint8_t*>(ptr);

	while (*t == 0xE9)
	{
		int offset = 0;
		memcpy(&offset, t + 1, sizeof(offset));
		t = t + 5 + offset;
	}

	return static_cast<void*>(t);
}

struct alignas(16) smem_base
{
	uint8_t trampoline[64];
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
			smem<FL> *mem = reinterpret_cast<smem<FL> *>(_sl::_get_rax());
			return mem->func(args...);
		}

		static TRet func_detour(TArgs... args)
		{
			smem<FL> *mem = reinterpret_cast<smem<FL>*>(_sl::_get_rax());
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
