#pragma once

namespace _sl
{

void* _DetourCopyInstruction(void* pDst, void** ppDstPool, const void* pSrc, void** ppTarget, long* plExtra);
bool _detour_does_code_end_function(const unsigned char*  pbCode);

}
