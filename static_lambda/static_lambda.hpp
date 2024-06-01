// Must disable JMC:
//   Project -> Properties -> C/C++ -> General -> Support Just My Code Debugging -> No.

#pragma once

#include "common.hpp"
#include "slwinapi.hpp"


namespace sl
{

template <typename F>
struct _helper;

template <typename TRet, typename... TArgs>
struct _helper<TRet(TArgs...)>
{
	using function_pointer_type = TRet (*)(TArgs...);

	template <typename TOther>
	static constexpr bool is_compatible = _is_same<decltype(_declval<TOther>()(_declval<TArgs>()...)), TRet>;

	template <typename FL>
	struct proxy
	{
		static TRet func(TArgs... args)
		{
			unsigned char *ptr = reinterpret_cast<unsigned char *>(_slwinapi::_sl_get_rax());
			FL *func = reinterpret_cast<FL *>(ptr);
			return (*func)(args...);
		}

		static void destroy(FL *func)
		{
			func->~FL();
		}
	};
};

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
	lambda(FL &&func)
		: _mem{ _slwinapi::_sl_VirtualAlloc(nullptr, _FL_OFFSET + sizeof(FL), _slwinapi::_MEM_COMMIT | _slwinapi::_MEM_RESERVE, _slwinapi::_PAGE_EXECUTE_READWRITE) }
	{
		static_assert(_helper<F>{}.is_compatible<FL>, "Lambda type is not compatible with declaration");

		unsigned char *ptr = static_cast<unsigned char *>(_mem);

		{
			unsigned char const *_t = reinterpret_cast<unsigned char const *>(ptr + _FL_OFFSET);
			unsigned char const *t = reinterpret_cast<unsigned char const *>(&_t);
			unsigned char *_f = reinterpret_cast<unsigned char *>(&_helper<F>::proxy<FL>::func);
			unsigned char *f = reinterpret_cast<unsigned char *>(&_f);

			unsigned char const opcodes[] = {
				// push rbx ; rbx must be saved by callee (if changed)
				0x53,
				// push rbx ; do it twice to align stack to 16
				0x53,

				// mov rax, <func> ; hidden argument passed in rax (non standart calling convention)
				0x48, 0xB8, t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],

				// mov rbx, <proxy>
				0x48, 0xBB, f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7],

				// call rbx
				0xFF, 0xD3,

				// pop rbx ; restore rbx
				0x5B,
				// pop rbx ; restore stack alignment
				0x5B,

				// ret
				0xC3,
			};

			_memcpy(ptr, opcodes, sizeof(opcodes));
		}

		{
			new (ptr + _FL_OFFSET) FL(static_cast<FL &&>(func));
		}

		{
			*reinterpret_cast<void **>(ptr + _DESTROY_OFFSET) = static_cast<void *>(&_helper<F>::proxy<FL>::destroy);
		}
	}

	~lambda()
	{
		if (_mem)
		{
			auto destroy = *reinterpret_cast<void (**)(void *)>(static_cast<unsigned char *>(_mem) + _DESTROY_OFFSET);
			destroy(static_cast<unsigned char *>(_mem) + _FL_OFFSET);
			_slwinapi::_sl_VirtualFree(_mem, 0, _slwinapi::_MEM_RELEASE);
		}
	}

	auto get_static_pointer() const { return static_cast<_helper<F>::function_pointer_type>(_mem); }

	void *_mem = 0;
};


} // namespace sl
