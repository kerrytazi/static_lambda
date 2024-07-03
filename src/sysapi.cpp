#include "sysapi.hpp"
#include "common.hpp"

#ifdef _WIN32
#include "slwinapi.hpp"

unsigned long long _sl::_get_rax()
{
	return _slwinapi::_sl_get_rax();
}

void *_sl::_alloc(void *target, unsigned long long size)
{
	// 2Gib down
	for (unsigned long long i = 1; i < 2040; ++i)
	{
		unsigned long long offset = i * 1024 * 1024;
		_slwinapi::_sl_MEMORY_BASIC_INFORMATION info = {};
		_slwinapi::_sl_VirtualQuery(static_cast<unsigned char *>(target) + offset, &info, sizeof(info));

		if (info.State & _slwinapi::_MEM_COMMIT)
			continue;

		return _slwinapi::_sl_VirtualAlloc(static_cast<unsigned char *>(target) + offset, size, _slwinapi::_MEM_COMMIT | _slwinapi::_MEM_RESERVE, _slwinapi::_PAGE_EXECUTE_READWRITE);
	}

	// 2Gib up
	for (unsigned long long i = 1; i < 2040; ++i)
	{
		unsigned long long offset = i * 1024 * 1024;
		_slwinapi::_sl_MEMORY_BASIC_INFORMATION info = {};
		_slwinapi::_sl_VirtualQuery(static_cast<unsigned char *>(target) - offset, &info, sizeof(info));

		if (info.State & _slwinapi::_MEM_COMMIT)
			continue;

		return _slwinapi::_sl_VirtualAlloc(static_cast<unsigned char *>(target) - offset, size, _slwinapi::_MEM_COMMIT | _slwinapi::_MEM_RESERVE, _slwinapi::_PAGE_EXECUTE_READWRITE);
	}

	return nullptr;
}

void _sl::_free(void *ptr, unsigned long long size)
{
	(void)size;
	_slwinapi::_sl_VirtualFree(ptr, 0, _slwinapi::_MEM_RELEASE);
}

void _sl::_protect(void *ptr, unsigned long long size)
{
	unsigned old_protection = 0;
	_slwinapi::_sl_VirtualProtect(ptr, size, _slwinapi::_PAGE_EXECUTE_READWRITE, &old_protection);
}

#else
#include "slunixapi.hpp"

unsigned long long _sl::_get_rax()
{
	return _slunixapi::_sl_get_rax();
}

void *_sl::_alloc(void *_target, unsigned long long size)
{
	auto [target, _] = align_mem_down_to(_target, size, 4096);

	// 2Gib down
	for (unsigned long long i = 1; i < 2040; ++i)
	{
		unsigned long long offset = i * 1024 * 1024; // check each 1Mib
		void *result = _slunixapi::_mmap(static_cast<unsigned char *>(target) + offset, size, _slunixapi::_PROT_EXEC | _slunixapi::_PROT_READ | _slunixapi::_PROT_WRITE, _slunixapi::_MAP_FIXED | _slunixapi::_MAP_PRIVATE | _slunixapi::_MAP_ANONYMOUS, -1, 0);

		if (result != _slunixapi::_MAP_FAILED)
			return result;
	}

	// 2Gib up
	for (unsigned long long i = 1; i < 2040; ++i)
	{
		unsigned long long offset = i * 1024 * 1024; // check each 1Mib
		void *result = _slunixapi::_mmap(static_cast<unsigned char *>(target) - offset, size, _slunixapi::_PROT_EXEC | _slunixapi::_PROT_READ | _slunixapi::_PROT_WRITE, _slunixapi::_MAP_FIXED | _slunixapi::_MAP_PRIVATE | _slunixapi::_MAP_ANONYMOUS, -1, 0);

		if (result != _slunixapi::_MAP_FAILED)
			return result;
	}

	return nullptr;
}

void _sl::_free(void *ptr, unsigned long long size)
{
	_slunixapi::_munmap(ptr, size);
}

void _sl::_protect(void *ptr, unsigned long long size)
{
	_slunixapi::_mprotect(ptr, size, _slunixapi::_PROT_EXEC | _slunixapi::_PROT_READ | _slunixapi::_PROT_WRITE);
}
#endif // _WIN32
