#pragma once

#include "../../src/common.hpp"
#include "../../src/sysapi.hpp"
#include "../../src/disasm_wrapper.hpp"

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
	void _init(FL&& func, const void* proxy_func, const void* proxy_call, const void* target)
	{
		if (!target)
			target = proxy_func;

		size_t alloc_size = sizeof(_sl::smem<std::remove_reference_t<FL>>);

		_mem = static_cast<_sl::smem_base*>(_sl::_alloc(target, alloc_size));

		if (_mem == nullptr)
			throw 1; // Not enough memory?

		{
			const uint8_t* pbSrc = static_cast<const uint8_t*>(proxy_func);
			uint8_t* pbTrampoline = _mem->trampoline;

			bool end_code = false;

			while (pbSrc - static_cast<const uint8_t*>(proxy_func) < sizeof(_mem->trampoline))
			{
				const uint8_t* pbOp = pbSrc;
				long lExtra = 0;

				pbSrc = (const uint8_t*)_sl::_DetourCopyInstruction(pbTrampoline, nullptr, pbSrc, nullptr, &lExtra);

				pbTrampoline += (pbSrc - pbOp); // + lExtra; // FIXME

				if (_sl::_detour_does_code_end_function(pbOp))
				{
					end_code = true;
					break;
				}
			}

			if (!end_code)
				throw 1; // func/func_detour is too large?
		}

		replace_pattern(_mem, intptr_t(0x0123456789abcdef), intptr_t(_mem));
		replace_pattern(_mem, intptr_t(0xfedcba9876543210), intptr_t(proxy_call));

		_mem->alloc_size = alloc_size;

		_mem->destroy = reinterpret_cast<void(*)(void*)>(reinterpret_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::destroy));

		new (_mem + 1) FL(std::move(func));
	}

	template <typename FL>
	lambda(FL&& func)
	{
		static_assert(_sl::helper<F>::template is_compatible<std::remove_reference_t<FL>>, "Lambda type is not compatible with declaration");

		_init<std::remove_reference_t<FL>>(
			static_cast<std::remove_reference_t<FL>&&>(func),
			std::bit_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::func),
			std::bit_cast<void*>(&_sl::helper<F>::template proxy<std::remove_reference_t<FL>>::call),
			nullptr
		);
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
		return std::bit_cast<typename _sl::helper<F>::function_pointer_type>(&_mem->trampoline);
	}

	_sl::smem_base* _mem = nullptr;
};


} // namespace sl
