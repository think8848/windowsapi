.code

GetCPUID proc
	mov r8, rcx
	mov eax, edx
	cpuid
	mov dword ptr [r8], eax
	mov dword ptr [r8 + 0Ch], edx	
	ret
GetCPUID endp

end