#pragma once


constexpr void *operator new (unsigned long long, void *p)
{
	return p;
}

namespace sl
{

const unsigned long long _DESTROY_OFFSET = 64;
const unsigned long long _SAVE_TARGET_CODE_OFFSET = 128;
const unsigned long long _SAVE_TARGET_OFFSET = _SAVE_TARGET_CODE_OFFSET + 16;
const unsigned long long _COPY_OF_ORIGINAL_OFFSET = _SAVE_TARGET_OFFSET + 16;
const unsigned long long _FL_OFFSET = 256;

template <typename A, typename B>
constexpr bool _is_same = false;

template <typename A>
constexpr bool _is_same<A, A> = true;

template <typename T>
constexpr bool _always_false = false;

template <typename T>
constexpr T _declval() { static_assert(_always_false<T>, "declval not allowed in an evaluated context"); }

void _memcpy(void *dst, void const *src, unsigned long long size)
{
	unsigned char *_dst = static_cast<unsigned char *>(dst);
	unsigned char const *_src = static_cast<unsigned char const *>(src);
	unsigned char const *end = _src + size;

	while (_src != end)
		*_dst++ = *_src++;
}

} // namespace sl
