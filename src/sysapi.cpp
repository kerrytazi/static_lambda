#include "sysapi.hpp"
#include "common.hpp"

#include <cstdint>

#if defined _MSC_VER
#pragma section(".text")
__declspec(allocate(".text"))
#else
__attribute__((section(".text")))
#endif
const uint8_t _sl_get_rip_code[]
{
	// pop rax
	0x58,
	// jmp rax
	0xff, 0xe0,
};

size_t(*const _sl::_get_rip)() = (size_t(*)())&_sl_get_rip_code;

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

void* _sl::_alloc(const void *_target, size_t _size)
{
	auto [target, size] = align_mem_down_to(_target, _size, 4096);

	if (size & (4096-1))
		size = (size & ~(4096-1)) + 4096;

#if 0
	// 2Gib down
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib

		const uint8_t* off_target = static_cast<const uint8_t*>(target) - offset;

		MEMORY_BASIC_INFORMATION info = {};
		if (!VirtualQuery((const void*)off_target, &info, sizeof(info)))
		{
			offset += 4096;
			continue;
		}

		if (info.State & MEM_FREE)
		{
			if (void* result = VirtualAlloc((void*)off_target, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
				return result;
		}
	}

	// 2Gib up
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib

		const uint8_t* off_target = static_cast<const uint8_t*>(target) + offset;

		MEMORY_BASIC_INFORMATION info = {};
		if (!VirtualQuery((const void*)off_target, &info, sizeof(info)))
		{
			offset += 4096;
			continue;
		}

		if (info.State & MEM_FREE)
		{
			if (void* result = VirtualAlloc((void*)off_target, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
				return result;
		}
	}
#else
	// 2Gib up
	for (size_t offset = 0; offset < size_t(2) * 1024 * 1024 * 1024;)
	{
		const uint8_t* off_target = static_cast<const uint8_t*>(target) + offset;

		MEMORY_BASIC_INFORMATION info = {};
		if (!VirtualQuery(off_target, &info, sizeof(info)))
		{
			offset += 4096;
			continue;
		}

		if (info.State & MEM_FREE)
		{
			if (void* result = VirtualAlloc((void*)off_target, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
				return result;
		}

		offset = static_cast<const uint8_t*>(info.BaseAddress) + info.RegionSize - static_cast<const uint8_t*>(target);
	}

	// 2Gib down
	for (size_t offset = 0; offset < size_t(2) * 1024 * 1024 * 1024;)
	{
		const uint8_t* off_target = static_cast<const uint8_t*>(target) - offset;

		MEMORY_BASIC_INFORMATION info = {};
		if (!VirtualQuery(off_target, &info, sizeof(info)))
		{
			offset += 4096;
			continue;
		}

		if (info.State & MEM_FREE)
		{
			if (void* result = VirtualAlloc((void*)off_target, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))
				return result;
		}

		offset += off_target - (static_cast<const uint8_t*>(info.AllocationBase) - 4096);
	}
#endif

	return nullptr;
}

void _sl::_free(void* ptr, size_t size)
{
	(void)size;
	VirtualFree(ptr, 0, MEM_RELEASE);
}

void _sl::_protect(void* ptr, size_t size)
{
	DWORD old_protection = 0;
	VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &old_protection);
}

#else

#include <sys/mman.h>

void* _sl::_alloc(const void* _target, size_t _size)
{
	auto [target, size] = align_mem_down_to(_target, _size, 4096);

	if (size & (4096-1))
		size = (size & ~(4096-1)) + 4096;

	// 2Gib up
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib
		void* result = mmap((void*)(static_cast<const uint8_t*>(target) + offset), size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (result != MAP_FAILED)
			return result;
	}

	// 2Gib down
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib
		void* result = mmap((void*)(static_cast<const uint8_t*>(target) - offset), size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (result != MAP_FAILED)
			return result;
	}

	return nullptr;
}

void _sl::_free(void* ptr, size_t size)
{
	munmap(ptr, size);
}

void _sl::_protect(void* ptr, size_t size)
{
	mprotect(ptr, size, PROT_EXEC | PROT_READ | PROT_WRITE);
}
#endif // _WIN32
