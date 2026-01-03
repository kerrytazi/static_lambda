#include "disasm_wrapper.hpp"
#include "windows_types.hpp"

#include "disasm.cpp"

namespace _Detour
{

static ULONG WINAPI DetourGetModuleSize(_In_opt_ HMODULE hModule)
{
    return 0;
}

}

static BOOL detour_does_code_end_function(PBYTE pbCode)
{
    if (pbCode[0] == 0xeb ||    // jmp +imm8
        pbCode[0] == 0xe9 ||    // jmp +imm32
        pbCode[0] == 0xe0 ||    // jmp eax
        pbCode[0] == 0xc2 ||    // ret +imm8
        pbCode[0] == 0xc3 ||    // ret
        pbCode[0] == 0xcc) {    // brk
        return TRUE;
    }
    else if (pbCode[0] == 0xf3 && pbCode[1] == 0xc3) {  // rep ret
        return TRUE;
    }
    else if (pbCode[0] == 0xff && pbCode[1] == 0x25) {  // jmp [+imm32]
        return TRUE;
    }
    else if ((pbCode[0] == 0x26 ||      // jmp es:
              pbCode[0] == 0x2e ||      // jmp cs:
              pbCode[0] == 0x36 ||      // jmp ss:
              pbCode[0] == 0x3e ||      // jmp ds:
              pbCode[0] == 0x64 ||      // jmp fs:
              pbCode[0] == 0x65) &&     // jmp gs:
             pbCode[1] == 0xff &&       // jmp [+imm32]
             pbCode[2] == 0x25) {
        return TRUE;
    }
    return FALSE;
}

namespace _sl
{

void* _DetourCopyInstruction(void* pDst, void** ppDstPool, const void* pSrc, void** ppTarget, long* plExtra)
{
	return _Detour::_xDetourCopyInstruction(pDst, ppDstPool, (void*)pSrc, ppTarget, plExtra);
}

bool _detour_does_code_end_function(const unsigned char* pbCode)
{
	return detour_does_code_end_function((PBYTE)pbCode);
}

}
