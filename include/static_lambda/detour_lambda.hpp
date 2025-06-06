#pragma once

#include "common.hpp"
#include "static_lambda.hpp"

namespace sl
{


template <typename F>
struct detour
{
	template <typename FL>
	detour(_sl::helper<F>::function_pointer_type target, FL &&func)
		: _lambda{ _sl::detour_tag<std::remove_reference_t<FL>>{} }
	{
		uint8_t* t = static_cast<uint8_t*>(_sl::unjump(reinterpret_cast<void*>(target)));

		_lambda.template _init<std::remove_reference_t<FL>>(static_cast<std::remove_reference_t<FL> &&>(func), reinterpret_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::func_detour), static_cast<void*>(t));

		size_t lambda_pointer = reinterpret_cast<size_t>(_lambda.get_static_pointer());

		size_t target_pointer = reinterpret_cast<size_t>(t);
		size_t _s = lambda_pointer - target_pointer - 5;
		uint8_t* s = reinterpret_cast<uint8_t*>(&_s);

		const size_t min_patch_size = 5;

		unsigned char patch[32] = {
			// jmp <lambda_pointer>
			0xE9, s[0], s[1], s[2], s[3],
		};

		size_t code_size = 0;

		{
			size_t _s = reinterpret_cast<size_t>(t) - reinterpret_cast<size_t>(_lambda._mem->original) - 5;
			uint8_t* s = reinterpret_cast<uint8_t*>(&_s);

			const uint8_t redirect[] = {
				// jmp <continue of original>
				0xE9, s[0], s[1], s[2], s[3],
			};

			{
				uint8_t* pbSrc = t;
				uint8_t* pbTrampoline = _lambda._mem->original;

				while (pbSrc - t < min_patch_size)
				{
					uint8_t* pbOp = pbSrc;
					long lExtra = 0;

					pbSrc = (uint8_t*)_sl::_DetourCopyInstructionX64(pbTrampoline, nullptr, pbSrc, nullptr, &lExtra);

					pbTrampoline += (pbSrc - pbOp) + lExtra;

					if (_sl::_detour_does_code_end_function(pbOp))
						break;
				}

				// sometimes small functions have padding after them
				while (pbSrc - t < min_patch_size)
				{
					if (*pbSrc == 0xCC || *pbSrc == 0x90) // int3, nop
					{
						*pbTrampoline++ = *pbSrc++;
					}
				}

				code_size = pbSrc - t;
			}

			memcpy(_lambda._mem->original + code_size, redirect, sizeof(redirect));
		}

		for (int i = 0; i < code_size - min_patch_size; ++i)
		{
			patch[min_patch_size + i] = 0x90; // noop
		}

		{
			auto [prot_mem, prot_size] = _sl::align_mem_down_to(t, code_size, 4096);
			_sl::_protect(prot_mem, prot_size);
		}

		memcpy(_lambda._mem->save_target_code, t, code_size);
		_lambda._mem->save_target = t;
		_lambda._mem->save_target_size = code_size;

		memcpy(t, patch, code_size);
	}

	~detour()
	{
		if (_lambda._mem)
		{
			memcpy(_lambda._mem->save_target, _lambda._mem->save_target_code, _lambda._mem->save_target_size);
		}
	}

	lambda<F> _lambda;
};


} // namespace sl
