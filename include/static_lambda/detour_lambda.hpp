// Must disable JMC:
//   Project -> Properties -> C/C++ -> General -> Support Just My Code Debugging -> No.

#pragma once

#include "slwinapi.hpp"
#include "common.hpp"
#include "static_lambda.hpp"
#include "ldisasm.hpp"


namespace sl
{


template <typename F>
struct detour
{
	template <typename FL>
	detour(_sl::helper<F>::function_pointer_type target, FL &&func)
		: _lambda{ _sl::detour_tag<_sl::remove_reference_t<FL>>{} }
	{
		unsigned char *t = static_cast<unsigned char *>(_sl::unjump(target));

		_lambda._init<_sl::remove_reference_t<FL>>(static_cast<_sl::remove_reference_t<FL> &&>(func), &_sl::helper<F>::proxy<_sl::remove_reference_t<FL>>::func_detour, t);

		unsigned long long lambda_pointer = reinterpret_cast<unsigned long long>(_lambda.get_static_pointer());
		unsigned long long target_pointer = reinterpret_cast<unsigned long long>(t);
		unsigned long long _s = lambda_pointer - target_pointer - 5;
		unsigned char *s = reinterpret_cast<unsigned char *>(&_s);

		const unsigned long long min_patch_size = 6;

		unsigned char patch[16] = {
			// jmp <lambda_pointer>
			0xE9, s[0], s[1], s[2], s[3],

			// pop rax ; needed to restore rax after redirecting from "copy of original" function
			0x58,
		};

		unsigned char *tmp = t;

		while (tmp - t < min_patch_size)
			tmp += ldisasm(tmp);

		unsigned long long code_size = tmp - t;

		for (int i = 0; i < code_size - min_patch_size; ++i)
		{
			patch[min_patch_size - 1 + i] = 0x90; // noop
			patch[min_patch_size - 1 + i + 1] = 0x58; // pop rax
		}

		{
			unsigned old_protection = 0;
			_slwinapi::_sl_VirtualProtect(t, code_size, _slwinapi::_PAGE_EXECUTE_READWRITE, &old_protection);
		}

		_sl::memcpy(_lambda._mem->save_target_code, t, 16);
		_lambda._mem->save_target = &t;
		_lambda._mem->save_target_size = code_size;

		{
			unsigned char const *_o = reinterpret_cast<unsigned char const *>(t) + code_size - 1;
			unsigned char const *o = reinterpret_cast<unsigned char const *>(&_o);

			unsigned char const redirect[] = {
				// push rax
				0x50,
				// mov rax, <original + offst to real code - 1 (for pop rax)>
				0x48, 0xB8, o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7],
				// jmp rax
				0xFF, 0xE0,
			};

			_sl::memcpy(_lambda._mem->original, t, code_size);
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
