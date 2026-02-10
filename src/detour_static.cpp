#include "../include/static_lambda/detour_static.hpp"

#include "common.hpp"
#include "sysapi.hpp"
#include "disasm_wrapper.hpp"

#include <atomic>

template <size_t N, typename T>
static void fast_memcpy(T* to, const T* from)
{
	std::atomic_thread_fence(std::memory_order::acquire);

	[&]<size_t... I>(std::index_sequence<I...>) {
		((to[I] = from[I]), ...);
	}(std::make_index_sequence<N>());

	std::atomic_thread_fence(std::memory_order::release);
}

template <size_t N, typename T>
static void fast_memcpy(T* to, const T (&from)[N])
{
	fast_memcpy<N>(to, (const T*)from);
}

static constexpr size_t JMP_SIZE = 5;

struct alignas(16) detour_data
{
	uint8_t new_target[32];

#if !defined(_M_IX86) && !defined(__i386__)
	uint8_t trampoline[16];
#endif

	uint8_t jmp_patch[JMP_SIZE];
	uint8_t old_target[JMP_SIZE];

	uint8_t* target;
	void* old_inout_target;
	size_t alloc_size;
};

static size_t copy_instructions(uint8_t* to, const uint8_t* from, size_t at_least_bytes)
{
	const uint8_t* from_begin = from;

	while (from - from_begin < at_least_bytes)
	{
		const uint8_t* prev_from = from;
		long lExtra = 0;

		from = (const uint8_t*)_sl::_DetourCopyInstruction(to, nullptr, from, nullptr, &lExtra);

		to += (from - prev_from); // + lExtra; // FIXME

		if (_sl::_detour_does_code_end_function(prev_from))
			break;
	}

	// Sometimes small functions have padding after them.
	// We can use it if not enough instructions in target.
	while (from - from_begin < at_least_bytes)
	{
		if (*from == 0xCC || *from == 0x90) // int3, nop
		{
			*to++ = *from++;
			continue;
		}

		break;
	}

	return from - from_begin;
}

static void fill_jmp_rel(const void* _from, const void* _to, uint8_t* buffer)
{
	const uint8_t* from = (const uint8_t*)_from;
	const uint8_t* to = (const uint8_t*)_to;

	ptrdiff_t _s = (ptrdiff_t)(to - from - 5);
	uint8_t* s = (uint8_t*)&_s;

	buffer[0] = 0xE9;
	buffer[1] = s[0];
	buffer[2] = s[1];
	buffer[3] = s[2];
	buffer[4] = s[3];
}

static void fill_jmp_abs(uint8_t* from, const void* _to)
{
	const uint8_t* to = (const uint8_t*)_to;

	// jmp    QWORD PTR [rip+0x0]
	// ff 25 00 00 00 00
	from[0] = 0xFF;
	from[1] = 0x25;
	from[2] = 0x00;
	from[3] = 0x00;
	from[4] = 0x00;
	from[5] = 0x00;

	uint8_t* s = (uint8_t*)&to;
	// xx xx xx xx xx xx xx xx
	from[ 6] = s[0];
	from[ 7] = s[1];
	from[ 8] = s[2];
	from[ 9] = s[3];
	from[10] = s[4];
	from[11] = s[5];
	from[12] = s[6];
	from[13] = s[7];
}

void* sl::detour_static_create(void** inout_target, const void* patch, size_t extra_size /*= 0*/)
{
	uint8_t* target = (uint8_t*)_sl::unjump(*inout_target);
	size_t alloc_size = sizeof(detour_data) + extra_size;
	detour_data* data = (detour_data*)_sl::_alloc(target, alloc_size, true);

	if (!data)
		return nullptr;

	auto copied = copy_instructions(data->new_target, target, sizeof(data->jmp_patch));

	if (copied < sizeof(data->jmp_patch))
	{
		_sl::_free(data, alloc_size);
		return nullptr;
	}

	memcpy(data->old_target, target, sizeof(data->old_target));

#if defined(_M_IX86) || defined(__i386__)
	fill_jmp_rel(target, patch, data->jmp_patch);
	fill_jmp_rel(data->new_target + copied, target + copied, data->new_target + copied);
#else
	fill_jmp_rel(target, data->trampoline, data->jmp_patch);
	fill_jmp_abs(data->trampoline, patch);
	fill_jmp_abs(data->new_target + copied, target + copied);
#endif

	{
		auto [prot_mem, prot_size] = _sl::align_mem_down_to(target, sizeof(data->jmp_patch), 4096);
		_sl::_protect((void*)prot_mem, prot_size);
	}

	data->alloc_size = alloc_size;
	data->target = target;
	data->old_inout_target = *inout_target;

	*inout_target = data;
	return data + 1;
}

void sl::detour_static_enable(void** inout_target)
{
	detour_data* data = (detour_data*)*inout_target;
	fast_memcpy(data->target, data->jmp_patch);
}

void sl::detour_static_disable(void** inout_target)
{
	detour_data* data = (detour_data*)*inout_target;
	fast_memcpy(data->target, data->old_target);
}

void sl::detour_static_destroy(void** inout_target)
{
	detour_data* data = (detour_data*)*inout_target;
	*inout_target = data->old_inout_target;
	_sl::_free(data, data->alloc_size);
}
