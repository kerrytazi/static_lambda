#pragma once

#include "slwinapi.hpp"
#include "common.hpp"

namespace sl
{


template <typename F>
struct lambda
{
	lambda(lambda const &) = delete;
	lambda &operator = (lambda const &) = delete;

	lambda(lambda &&other)
	{
		_mem = other._mem;
		other._mem = nullptr;
	}

	lambda &operator = (lambda &&other)
	{
		auto tmp = _mem;
		_mem = other._mem;
		other._mem = tmp;
		return *this;
	}

	template <typename FL>
	void _init(FL &&func, void *proxy_func, void *target)
	{
		if (!target)
			target = proxy_func;

		{
			for (unsigned long long i = 1; _mem == nullptr; ++i)
			{
				unsigned long long offset = i * 32 * 1024 * 1024;
				_slwinapi::_sl_MEMORY_BASIC_INFORMATION info = {};
				_slwinapi::_sl_VirtualQuery(static_cast<unsigned char *>(target) + offset, &info, sizeof(info));

				if (info.State & _slwinapi::_MEM_COMMIT)
					continue;

				_mem = static_cast<_sl::smem_base *>(_slwinapi::_sl_VirtualAlloc(static_cast<unsigned char *>(target) + offset, sizeof(_sl::smem<_sl::remove_reference_t<FL>>), _slwinapi::_MEM_COMMIT | _slwinapi::_MEM_RESERVE, _slwinapi::_PAGE_EXECUTE_READWRITE));
			}
		}
		//else
		//{
		//	_mem = static_cast<_sl::smem_base *>(_slwinapi::_sl_VirtualAlloc(nullptr, sizeof(_sl::smem<_sl::remove_reference_t<FL>>), _slwinapi::_MEM_COMMIT | _slwinapi::_MEM_RESERVE, _slwinapi::_PAGE_EXECUTE_READWRITE));
		//}

		{
			unsigned char const *_t = reinterpret_cast<unsigned char const *>(_mem);
			unsigned char const *t = reinterpret_cast<unsigned char const *>(&_t);
			unsigned long long _f = reinterpret_cast<unsigned long long>(proxy_func) - reinterpret_cast<unsigned long long>(_mem->trampoline) - 10 - 5; // size: 10 - mov, 5 - jmp
			unsigned char *f = reinterpret_cast<unsigned char *>(&_f);

			unsigned char const opcodes[] = {
				// mov rax, <mem> ; hidden argument passed in rax (non standard calling convention)
				0x48, 0xB8, t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],

				// jmp <proxy>
				0xE9, f[0], f[1], f[2], f[3],
			};

			_sl::memcpy(_mem->trampoline, opcodes, sizeof(opcodes));
		}

		{
			_mem->destroy = static_cast<void (*)(void *)>(static_cast<void *>(&_sl::helper<F>::proxy<_sl::remove_reference_t<FL>>::destroy));
		}

		{
			new (_mem + 1) _sl::remove_reference_t<FL>(static_cast<_sl::remove_reference_t<FL> &&>(func));
		}
	}

	template <typename FL>
	lambda(FL &&func)
	{
		static_assert(_sl::helper<F>{}.is_compatible<_sl::remove_reference_t<FL>>, "Lambda type is not compatible with declaration");
		_init<_sl::remove_reference_t<FL>>(static_cast<_sl::remove_reference_t<FL> &&>(func), &_sl::helper<F>::proxy<_sl::remove_reference_t<FL>>::func, nullptr);
	}

	template <typename FL>
	lambda(_sl::detour_tag<FL> &&)
	{
		static_assert(_sl::helper<F>{}.is_compatible_detour<_sl::remove_reference_t<FL>>, "Lambda type is not compatible with declaration");
		// init later
	}

	~lambda()
	{
		if (_mem)
		{
			_mem->destroy(_mem + 1);
			_slwinapi::_sl_VirtualFree(_mem, 0, _slwinapi::_MEM_RELEASE);
		}
	}

	auto get_static_pointer() const { return static_cast<_sl::helper<F>::function_pointer_type>(static_cast<void *>(_mem->trampoline)); }

	_sl::smem_base *_mem = nullptr;
};


} // namespace sl
