#pragma once

namespace _slunixapi
{

const unsigned long long _PROT_READ = 0x01;
const unsigned long long _PROT_WRITE = 0x02;
const unsigned long long _PROT_EXEC = 0x04;

const unsigned long long _MAP_FIXED = 0x0010;
const unsigned long long _MAP_PRIVATE = 0x0002;
const unsigned long long _MAP_ANONYMOUS = 0x0020;

const void *_MAP_FAILED = (void *)-1;


template<unsigned long long syscall_id, int arg_count>
struct _syscall;

template<unsigned long long syscall_id>
struct _syscall<syscall_id, 1> {
	static unsigned long long call(unsigned long long p1) {
		unsigned long long ret;
		asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(syscall_id), "D"(p1)
			: "rcx", "r11", "memory"
		);
		return ret;
	}
};

template<unsigned long long syscall_id>
struct _syscall<syscall_id, 2> {
	static unsigned long long call(unsigned long long p1,unsigned long long p2) {
		unsigned long long ret;
		asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(syscall_id), "D"(p1), "S"(p2)
			: "rcx", "r11", "memory"
		);
		return ret;
	}
};

template<unsigned long long syscall_id>
struct _syscall<syscall_id, 3> {
	static unsigned long long call(unsigned long long p1,unsigned long long p2,unsigned long long p3) {
		unsigned long long ret;
		asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(syscall_id), "D"(p1), "S"(p2), "d"(p3)
			: "rcx", "r11", "memory"
		);
		return ret;
	}
};

template<unsigned long long syscall_id>
struct _syscall<syscall_id, 4> {
	static unsigned long long call(unsigned long long p1,unsigned long long p2,unsigned long long p3,unsigned long long p4) {
		unsigned long long ret;
		register long r10 asm("r10") = p4;
		asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(syscall_id), "D"(p1), "S"(p2), "d"(p3), "r"(r10)
			: "rcx", "r11", "memory"
		);
		return ret;
	}
};

template<unsigned long long syscall_id>
struct _syscall<syscall_id, 5> {
	static unsigned long long call(unsigned long long p1,unsigned long long p2,unsigned long long p3,unsigned long long p4,unsigned long long p5) {
		unsigned long long ret;
		register long r10 asm("r10") = p4;
		register long r8 asm("r8") = p5;
		asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(syscall_id), "D"(p1), "S"(p2), "d"(p3), "r"(r10), "r"(r8)
			: "rcx", "r11", "memory"
		);
		return ret;
	}
};

template<unsigned long long syscall_id>
struct _syscall<syscall_id, 6> {
	static unsigned long long call(unsigned long long p1,unsigned long long p2,unsigned long long p3,unsigned long long p4,unsigned long long p5,unsigned long long p6) {
		unsigned long long ret;
		register long r10 asm("r10") = p4;
		register long r8 asm("r8") = p5;
		register long r9 asm("r9") = p6;
		asm volatile
		(
			"syscall"
			: "=a" (ret)
			: "0"(syscall_id), "D"(p1), "S"(p2), "d"(p3), "r"(r10), "r"(r8), "r"(r9)
			: "rcx", "r11", "memory"
		);
		return ret;
	}
};



unsigned long long _sl_get_rax()
{
	unsigned long long ret;
	asm volatile
	(
		""
		: "=a" (ret)
		:
		:
	);
	return ret;
}

void *_mmap(void *addr, unsigned long long length, int prot, int flags, int fd, unsigned long long offset)
{
	return reinterpret_cast<void *>(_syscall<9, 6>::call(reinterpret_cast<unsigned long long>(addr), length, prot, flags, fd, offset));
}

int _mprotect(void *addr, unsigned long long len, int prot)
{
	return _syscall<10, 3>::call(reinterpret_cast<unsigned long long>(addr), len, prot);
}

int _munmap(void *addr, unsigned long long length)
{
	return _syscall<11, 2>::call(reinterpret_cast<unsigned long long>(addr), length);
}


} // namespace _slunixapi
