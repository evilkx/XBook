;----
;file:		lib/munmap.asm
;auther:	Jason Hu
;time:		2019/7/29
;copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
;----

[bits 32]
[section .text]


%include "sys/syscall.inc"

global munmap

; int munmap(uint32_t addr, uint32_t len);
munmap:
	

	mov eax, SYS_MUNMAP
	mov ebx, [esp + 4]
	mov ecx, [esp + 8]
	int INT_VECTOR_SYS_CALL
	
	
	ret