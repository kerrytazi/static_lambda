#pragma once

#include "../../src/common.hpp"
#include "static_lambda.hpp"

namespace sl
{


template <typename F>
struct detour
{
	template <typename FL>
	detour(auto target, FL&& func)
		: _lambda{ _sl::detour_tag<std::remove_reference_t<FL>>{} }
	{
		const uint8_t* t = static_cast<const uint8_t*>(_sl::unjump(std::bit_cast<const void*>(target)));

		_lambda.template _init<std::remove_reference_t<FL>>(
			static_cast<std::remove_reference_t<FL>&&>(func),
			std::bit_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::func_detour),
			std::bit_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::call_detour),
			static_cast<const void*>(t)
		);

		intptr_t lambda_pointer = std::bit_cast<intptr_t>(_lambda.get_static_pointer());

		intptr_t target_pointer = reinterpret_cast<intptr_t>(t);
		intptr_t _s = lambda_pointer - target_pointer - 5;
		uint8_t* s = reinterpret_cast<uint8_t*>(&_s);

		const size_t min_patch_size = 5;

		unsigned char patch[32] = {
			// jmp <lambda_pointer>
			0xE9, s[0], s[1], s[2], s[3],
		};

		size_t code_size = 0;

		{
			intptr_t _s = reinterpret_cast<intptr_t>(t) - reinterpret_cast<intptr_t>(&_lambda._mem->original) - 5;
			uint8_t* s = reinterpret_cast<uint8_t*>(&_s);

			const uint8_t redirect[] = {
				// jmp <continue of original>
				0xE9, s[0], s[1], s[2], s[3],
			};

			{
				const uint8_t* pbSrc = t;
				uint8_t* pbTrampoline = _lambda._mem->original;

				while (pbSrc - t < min_patch_size)
				{
					const uint8_t* pbOp = pbSrc;
					long lExtra = 0;

					pbSrc = (const uint8_t*)_sl::_DetourCopyInstruction(pbTrampoline, nullptr, pbSrc, nullptr, &lExtra);

					pbTrampoline += (pbSrc - pbOp); // + lExtra; // FIXME

					if (_sl::_detour_does_code_end_function(pbOp))
						break;
				}

				// sometimes small functions have padding after them
				while (pbSrc - t < min_patch_size)
				{
					if (*pbSrc == 0xCC || *pbSrc == 0x90) // int3, nop
					{
						*pbTrampoline++ = *pbSrc++;
						continue;
					}

					break;
				}

				code_size = pbSrc - t;
			}

			if (code_size < min_patch_size)
				throw 1;

			memcpy(_lambda._mem->original + code_size, redirect, sizeof(redirect));
		}

		for (int i = 0; i < code_size - min_patch_size; ++i)
			patch[min_patch_size + i] = 0x90; // noop

		{
			auto [prot_mem, prot_size] = _sl::align_mem_down_to(t, code_size, 4096);
			_sl::_protect((void*)prot_mem, prot_size);
		}

		memcpy(_lambda._mem->save_target_code, t, code_size);
		_lambda._mem->save_target = (void*)t;
		_lambda._mem->save_target_size = code_size;

		memcpy((void*)t, patch, code_size);
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
