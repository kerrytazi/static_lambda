// Must enable MASM to compile slwinapi.asm
//   Project dependencies:
//     Project -> Build Dependencies -> Build Customizations... -> masm (checkbox on)
//   File slwinapi.asm:
//     File -> Properties -> General -> Item Type -> Microsoft Macro Assembler

#pragma once

namespace _slwinapi
{


const unsigned _MEM_COMMIT = 0x00001000;
const unsigned _MEM_RESERVE = 0x00002000;
const unsigned _PAGE_EXECUTE_READWRITE = 0x40;
const unsigned _MEM_RELEASE = 0x00008000;

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


extern "C" unsigned long long _sl_get_rax();


} // namespace slwinapi
