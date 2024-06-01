
.code

_sl_get_rax proc
	ret
_sl_get_rax endp

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


end
