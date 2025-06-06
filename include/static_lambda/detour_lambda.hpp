#pragma once

#include "common.hpp"
#include "static_lambda.hpp"
#include "ldisasm.hpp"

extern "C"
void* __stdcall DetourCopyInstructionX64(void* pDst,
                                   void** ppDstPool,
                                   void* pSrc,
                                   void** ppTarget,
                                   long* plExtra);

bool detour_does_code_end_function(unsigned char* pbCode);

namespace sl
{


template <typename F>
struct detour
{
	template <typename FL>
	detour(_sl::helper<F>::function_pointer_type target, FL &&func)
		: _lambda{ _sl::detour_tag<_sl::remove_reference_t<FL>>{} }
	{
		unsigned char* t = static_cast<unsigned char*>(_sl::unjump(reinterpret_cast<void*>(target)));

		_lambda.template _init<_sl::remove_reference_t<FL>>(static_cast<_sl::remove_reference_t<FL> &&>(func), reinterpret_cast<void*>(&_sl::helper<F>::template proxy<_sl::remove_reference_t<FL>>::func_detour), static_cast<void*>(t));

		unsigned long long lambda_pointer = reinterpret_cast<unsigned long long>(_lambda.get_static_pointer());

		unsigned long long target_pointer = reinterpret_cast<unsigned long long>(t);
		unsigned long long _s = lambda_pointer - target_pointer - 5;
		unsigned char* s = reinterpret_cast<unsigned char*>(&_s);

		const unsigned long long min_patch_size = 5;

		unsigned char patch[32] = {
			// jmp <lambda_pointer>
			0xE9, s[0], s[1], s[2], s[3],
		};

		unsigned char* tmp = t;

		while (tmp - t < min_patch_size)
			tmp += ldisasm(tmp);

		unsigned long long code_size = tmp - t;

		for (int i = 0; i < code_size - min_patch_size; ++i)
		{
			patch[min_patch_size + i] = 0x90; // noop
		}

		{
			auto [prot_mem, prot_size] = _sl::align_mem_down_to(t, code_size, 4096);
			_sl::_protect(prot_mem, prot_size);
		}

		_sl::memcpy(_lambda._mem->save_target_code, t, 16);
		_lambda._mem->save_target = t;
		_lambda._mem->save_target_size = code_size;

		{
			unsigned long long _s = reinterpret_cast<unsigned long long>(t) - reinterpret_cast<unsigned long long>(_lambda._mem->original) - 5;
			unsigned char* s = reinterpret_cast<unsigned char* >(&_s);

			unsigned char const redirect[] = {
				// jmp <continue of original>
				0xE9, s[0], s[1], s[2], s[3],
			};

			if (true)
			{
				unsigned char* pbSrc = (unsigned char*)t; // pbTarget;
				unsigned char* pbTrampoline = (unsigned char*)_lambda._mem->original; // pTrampoline->rbCode;
				unsigned long long cbTarget = 0;
				unsigned long long cbJump = 5; // SIZE_OF_JMP;
				unsigned long long nAlign = 0;

				while (cbTarget < cbJump)
				{
					unsigned char* pbOp = pbSrc;
					long lExtra = 0;

					pbSrc = (unsigned char*)DetourCopyInstructionX64(pbTrampoline, nullptr, pbSrc, nullptr, &lExtra);

					pbTrampoline += (pbSrc - pbOp) + lExtra;
					cbTarget = (unsigned long long)(pbSrc - (unsigned char*)t);

					if (detour_does_code_end_function(pbOp)) {
						break;
					}
				}
			}
			else
			{
				_sl::memcpy(_lambda._mem->original, t, code_size);
			}

			_sl::memcpy(_lambda._mem->original + code_size, redirect, sizeof(redirect));
		}

		_sl::memcpy(t, patch, code_size);
	}

	~detour()
	{
		if (_lambda._mem)
		{
			_sl::memcpy(_lambda._mem->save_target, _lambda._mem->save_target_code, _lambda._mem->save_target_size);
		}
	}

	lambda<F> _lambda;
};


} // namespace sl
