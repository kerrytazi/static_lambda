#include "sysapi.hpp"
#include "common.hpp"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

extern "C" size_t _sl_get_rax();

size_t _sl::_get_rax()
{
	return _sl_get_rax();
}

void *_sl::_alloc(void *target, size_t size)
{
	// 2Gib down
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024;
		MEMORY_BASIC_INFORMATION info = {};
		VirtualQuery(static_cast<unsigned char *>(target) + offset, &info, sizeof(info));

		if (info.State & MEM_COMMIT)
			continue;

		return VirtualAlloc(static_cast<unsigned char *>(target) + offset, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}

	// 2Gib up
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024;
		MEMORY_BASIC_INFORMATION info = {};
		VirtualQuery(static_cast<unsigned char *>(target) - offset, &info, sizeof(info));

		if (info.State & MEM_COMMIT)
			continue;

		return VirtualAlloc(static_cast<unsigned char *>(target) - offset, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}

	return nullptr;
}

void _sl::_free(void *ptr, size_t size)
{
	(void)size;
	VirtualFree(ptr, 0, MEM_RELEASE);
}

void _sl::_protect(void *ptr, size_t size)
{
	DWORD old_protection = 0;
	VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &old_protection);
}

#else

#include <sys/mman.h>

size_t _sl::_get_rax()
{
	size_t ret;
	asm volatile
	(
		""
		: "=a" (ret)
		:
		:
	);
	return ret;
}

void *_sl::_alloc(void *_target, size_t size)
{
	auto [target, _] = align_mem_down_to(_target, size, 4096);

	// 2Gib down
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib
		void *result = mmap(static_cast<unsigned char *>(target) + offset, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (result != MAP_FAILED)
			return result;
	}

	// 2Gib up
	for (size_t i = 1; i < 2040; ++i)
	{
		size_t offset = i * 1024 * 1024; // check each 1Mib
		void *result = mmap(static_cast<unsigned char *>(target) - offset, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (result != MAP_FAILED)
			return result;
	}

	return nullptr;
}

void _sl::_free(void *ptr, size_t size)
{
	munmap(ptr, size);
}

void _sl::_protect(void *ptr, size_t size)
{
	mprotect(ptr, size, PROT_EXEC | PROT_READ | PROT_WRITE);
}
#endif // _WIN32
