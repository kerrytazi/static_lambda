#include "../include/static_lambda/sysapi.hpp"
#include "../include/static_lambda/common.hpp"

#include <cstdint>

#if defined(_SL_SINGLEHEADER_)
#define _SL_INLINE_ inline
#else
#define _SL_INLINE_
#endif

#if defined _MSC_VER
#pragma section(".text")
__declspec(allocate(".text"))
#else
__attribute__((section(".text")))
#endif
_SL_INLINE_
const uint8_t _sl_get_rip_code[]
{
	// pop rax
	0x58,
	// jmp rax
	0xff, 0xe0,
};

_SL_INLINE_
constinit size_t(*const _sl::_get_rip)() = (size_t(*)())&_sl_get_rip_code;

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

_SL_INLINE_
void *_sl::_alloc(const void *target, size_t size)
{
	// 2Gib down
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024;
		MEMORY_BASIC_INFORMATION info = {};
		VirtualQuery(static_cast<const uint8_t*>(target) + offset, &info, sizeof(info));

		if (info.State & MEM_COMMIT)
			continue;

		return VirtualAlloc((void*)(static_cast<const uint8_t*>(target) + offset), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}

	// 2Gib up
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024;
		MEMORY_BASIC_INFORMATION info = {};
		VirtualQuery(static_cast<const uint8_t*>(target) - offset, &info, sizeof(info));

		if (info.State & MEM_COMMIT)
			continue;

		return VirtualAlloc((void*)(static_cast<const uint8_t*>(target) - offset), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}

	return nullptr;
}

_SL_INLINE_
void _sl::_free(void *ptr, size_t size)
{
	(void)size;
	VirtualFree(ptr, 0, MEM_RELEASE);
}

_SL_INLINE_
void _sl::_protect(void *ptr, size_t size)
{
	DWORD old_protection = 0;
	VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &old_protection);
}

#else

#include <sys/mman.h>

_SL_INLINE_
void *_sl::_alloc(const void *_target, size_t size)
{
	auto [target, _] = align_mem_down_to(_target, size, 4096);

	// 2Gib down
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib
		void *result = mmap((void*)(static_cast<const uint8_t*>(target) + offset), size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (result != MAP_FAILED)
			return result;
	}

	// 2Gib up
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib
		void *result = mmap((void*)(static_cast<const uint8_t*>(target) - offset), size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (result != MAP_FAILED)
			return result;
	}

	return nullptr;
}

_SL_INLINE_
void _sl::_free(void *ptr, size_t size)
{
	munmap(ptr, size);
}

_SL_INLINE_
void _sl::_protect(void *ptr, size_t size)
{
	mprotect(ptr, size, PROT_EXEC | PROT_READ | PROT_WRITE);
}
#endif // _WIN32
