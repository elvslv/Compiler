.686
.model flat,stdcall
option casemap :none
include include\windows.inc
include include\kernel32.inc
include include\masm32.inc
includelib lib\masm32.lib
includelib lib\kernel32.lib
.data
	G_A dd 0
	G_B dd 0
	G_C dd 0
	G_D dd 0
	G_writebuf dd 256 dup(0)
.code
main:
	push G_A
	push G_A
	push 78
	pop EBX
	pop EAX
	add EAX, EBX
	push EAX
	pop EAX
	mov G_A, EAX
	push G_B
	push G_A
	push 156
	pop EBX
	pop EAX
	sub EAX, EBX
	push EAX
	pop EAX
	mov G_B, EAX
	push G_C
	push G_A
	push G_B
	pop EBX
	pop EAX
	xor EDX, EDX
	idiv EBX
	mov EAX, EDX
	push EAX
	pop EAX
	xor EBX, EBX
	sub EBX, EAX
	mov EAX, EBX
	push EAX
	push 44
	push 13
	push G_C
	pop EBX
	pop EAX
	sub EAX, EBX
	push EAX
	pop EBX
	pop EAX
	xor EDX, EDX
	imul EBX
	push EAX
	pop EBX
	pop EAX
	add EAX, EBX
	push EAX
	pop EAX
	mov G_C, EAX
	push G_A
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	push G_B
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	push G_C
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
