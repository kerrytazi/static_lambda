#pragma once

#include "common.hpp"
#include "sysapi.hpp"

#include <new>
#include <utility>
#include <cstring>

namespace sl
{


template <typename F>
struct lambda
{
	lambda(const lambda&) = delete;
	lambda& operator=(const lambda&) = delete;

	lambda(lambda&& other)
	{
		_mem = other._mem;
		other._mem = nullptr;
	}

	lambda& operator=(lambda&& other)
	{
		auto tmp = _mem;
		_mem = other._mem;
		other._mem = tmp;
		return *this;
	}

	template <typename FL>
	void _init(FL&& func, void* proxy_func, void* target)
	{
		if (!target)
			target = proxy_func;

		auto alloc_size = sizeof(_sl::smem<std::remove_reference_t<FL>>);

		_mem = static_cast<_sl::smem_base*>(_sl::_alloc(target, alloc_size));

		{
			const uint8_t* _t = reinterpret_cast<const uint8_t*>(_mem);
			const uint8_t* t = reinterpret_cast<const uint8_t*>(&_t);
			size_t _f = reinterpret_cast<size_t>(proxy_func) - reinterpret_cast<size_t>(_mem->trampoline) - 10 - 5; // size: 10 - mov, 5 - jmp
			uint8_t* f = reinterpret_cast<uint8_t*>(&_f);

			unsigned char const opcodes[] = {
				// mov rax, <mem> ; hidden argument passed in rax (non standard calling convention)
				0x48, 0xB8, t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],

				// jmp <proxy>
				0xE9, f[0], f[1], f[2], f[3],
			};

			memcpy(_mem->trampoline, opcodes, sizeof(opcodes));
		}

		_mem->alloc_size = alloc_size;

		_mem->destroy = reinterpret_cast<void (*)(void*)>(reinterpret_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::destroy));

		new (_mem + 1) FL(std::move(func));
	}

	template <typename FL>
	lambda(FL&& func)
	{
		static_assert(_sl::helper<F>::template is_compatible<std::remove_reference_t<FL>>, "Lambda type is not compatible with declaration");
		_init<std::remove_reference_t<FL>>(static_cast<std::remove_reference_t<FL>&&>(func), reinterpret_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::func), nullptr);
	}

	template <typename FL>
	lambda(_sl::detour_tag<FL>&&)
	{
		static_assert(_sl::helper<F>::template is_compatible_detour<std::remove_reference_t<FL>>, "Lambda type is not compatible with declaration");
		// init later
	}

	~lambda()
	{
		if (_mem)
		{
			_mem->destroy(_mem + 1);
			_sl::_free(_mem, _mem->alloc_size);
		}
	}

	auto get_static_pointer() const
	{
		return reinterpret_cast<_sl::helper<F>::function_pointer_type>(static_cast<void*>(_mem->trampoline));
	}

	_sl::smem_base* _mem = nullptr;
};


} // namespace sl
