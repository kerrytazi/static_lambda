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
	detour(_helper<F>::function_pointer_type target, FL &&func)
		: _sl{ static_cast<_remove_reference_t<FL> &&>(func), _detour_tag{} }
	{
		unsigned char *t = reinterpret_cast<unsigned char *>(target);

		while (*t == 0xE9)
		{
			int offset = 0;
			_memcpy(&offset, t + 1, sizeof(offset));
			t = t + 5 + offset;
		}

		unsigned char const *_s = reinterpret_cast<unsigned char const *>(_sl.get_static_pointer());
		unsigned char const *s = reinterpret_cast<unsigned char const *>(&_s);

		const unsigned long long min_patch_size = 13;

		unsigned char patch[32] = {
			// mov rax, <static_lambda>
			0x48, 0xB8, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
			// jmp rax
			0xFF, 0xE0,

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

		auto save_target_code = static_cast<unsigned char *>(_sl._mem) + _SAVE_TARGET_CODE_OFFSET;
		auto save_target = static_cast<unsigned char *>(_sl._mem) + _SAVE_TARGET_OFFSET;
		auto save_target_size = static_cast<unsigned char *>(_sl._mem) + _SAVE_TARGET_SIZE_OFFSET;

		_memcpy(save_target_code, t, 16);
		_memcpy(save_target, &t, 8);
		_memcpy(save_target_size, &code_size, 8);

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

			auto copy_of_original = static_cast<unsigned char *>(_sl._mem) + _COPY_OF_ORIGINAL_OFFSET;
			_memcpy(copy_of_original, t, code_size);
			_memcpy(copy_of_original + code_size, redirect, sizeof(redirect));
		}

		_memcpy(t, patch, code_size);
	}

	~detour()
	{
		if (_sl._mem)
		{
			auto save_target_code = static_cast<unsigned char *>(_sl._mem) + _SAVE_TARGET_CODE_OFFSET;
			auto save_target = static_cast<unsigned char *>(_sl._mem) + _SAVE_TARGET_OFFSET;
			auto save_target_size = static_cast<unsigned char *>(_sl._mem) + _SAVE_TARGET_SIZE_OFFSET;

			void *t = *reinterpret_cast<void **>(save_target);
			unsigned long long code_size = *reinterpret_cast<unsigned long long *>(save_target_size);
			_memcpy(t, save_target_code, code_size);
		}
	}

	lambda<F> _sl;
};


} // namespace sl
