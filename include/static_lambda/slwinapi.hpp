// Must enable MASM to compile slwinapi.asm
//   Project dependencies:
//     Project -> Build Dependencies -> Build Customizations... -> masm (checkbox on)
//   File slwinapi.asm:
//     File -> Properties -> General -> Item Type -> Microsoft Macro Assembler

#pragma once

namespace _slwinapi
{


const unsigned MEM_COMMIT = 0x00001000;
const unsigned MEM_RESERVE = 0x00002000;
const unsigned PAGE_EXECUTE_READWRITE = 0x40;
const unsigned MEM_RELEASE = 0x00008000;

extern "C"
void *
_sl_NtAllocateVirtualMemory(
	void *hProcess,
	void *lpAddress,
	unsigned long long *lpZeroBits,
	unsigned long long *dwSize,
	unsigned flAllocationType,
	unsigned flProtect
);

extern "C"
int
_sl_NtFreeVirtualMemory(
	void *hProcess,
	void *lpAddress,
	unsigned long long *dwSize,
	unsigned dwFreeType
);

extern "C"
int
_sl_NtProtectVirtualMemory(
	void *hProcess,
	void *lpAddress,
	unsigned long long *dwSize,
	unsigned flNewProtect,
	unsigned *lpflOldProtect
);

extern "C"
int
_sl_NtQueryVirtualMemory(
	void *hProcess,
	void const *BaseAddress,
	int MemoryInformationClass,
	void *MemoryInformation,
	unsigned long long MemoryInformationLength,
	unsigned long long *ReturnLength
);


inline
void *
_sl_VirtualAlloc(
	void *lpAddress,
	unsigned long long dwSize,
	unsigned flAllocationType,
	unsigned flProtect)
{
	_sl_NtAllocateVirtualMemory(
		(void *)-1,
		&lpAddress,
		nullptr,
		&dwSize,
		flAllocationType & 0xFFFFFFC0,
		flProtect
	);

	return lpAddress;
}

inline
int
_sl_VirtualFree(
	void *lpAddress,
	unsigned long long dwSize,
	unsigned dwFreeType)
{
	int result = _sl_NtFreeVirtualMemory(
		(void *)-1,
		&lpAddress,
		&dwSize,
		dwFreeType
	);

	return result >= 0 ? 1 : 0;
}

inline
int
_sl_VirtualProtect(
	void *lpAddress,
	unsigned long long dwSize,
	unsigned flNewProtect,
	unsigned *lpflOldProtect)
{
	int result = _sl_NtProtectVirtualMemory(
		(void*)-1,
		&lpAddress,
		&dwSize,
		flNewProtect,
		lpflOldProtect
	);

	return result >= 0 ? 1 : 0;
}

struct _sl_MEMORY_BASIC_INFORMATION
{
	void *BaseAddress;
	void *AllocationBase;
	unsigned AllocationProtect;
	unsigned short PartitionId;
	unsigned long long RegionSize;
	unsigned State;
	unsigned Protect;
	unsigned Type;
};

inline
unsigned long long
_sl_VirtualQuery(
	void const *lpAddress,
	_sl_MEMORY_BASIC_INFORMATION *lpBuffer,
	unsigned long long dwLength
)
{
	unsigned long long ResultLength = 0;

	unsigned long long result = _sl_NtQueryVirtualMemory(
		(void *)-1,
		lpAddress,
		0, // MemoryBasicInformation
		lpBuffer,
		dwLength,
		&ResultLength
	);

	if (result >= 0)
		return ResultLength;

	return 0;
}


extern "C" unsigned long long _sl_get_rax();


} // namespace slwinapi
