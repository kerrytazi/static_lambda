
.code

_sl_NtAllocateVirtualMemory proc
	mov r10, rcx
	mov eax, 18h
	syscall
	ret
_sl_NtAllocateVirtualMemory endp

_sl_NtFreeVirtualMemory proc
	mov r10, rcx
	mov eax, 1eh
	syscall
	ret
_sl_NtFreeVirtualMemory endp

_sl_NtProtectVirtualMemory proc
	mov r10, rcx
	mov eax, 50h
	syscall
	ret
_sl_NtProtectVirtualMemory endp

_sl_NtQueryVirtualMemory proc
	mov r10, rcx
	mov eax, 23h
	syscall
	ret
_sl_NtQueryVirtualMemory endp

_sl_get_rax proc
	ret
_sl_get_rax endp

_sl_copy_aligned_16 proc
	movaps xmm0, [rdx]
	movdqa [rcx], xmm0
	ret
_sl_copy_aligned_16 endp


end
