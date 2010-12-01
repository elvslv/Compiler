.686
.model flat,stdcall
option casemap :none
include include\windows.inc
include include\kernel32.inc
include include\masm32.inc
includelib lib\masm32.lib
includelib lib\kernel32.lib
.data
	G_writebuf dd 256 dup(0)
.code
main:
	push 24
	push 55
	pop EBX
	pop EAX
	xor EDX, EDX
	imul EBX
	push EAX
	push 67
	push 15
	pop EBX
	pop EAX
	xor EDX, EDX
	idiv EBX
	mov EAX, EDX
	push EAX
	pop EBX
	pop EAX
	sub EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	push 12
	push 10
	pop EBX
	pop EAX
	and EAX, EBX
	push EAX
	push 16
	pop EBX
	pop EAX
	sub EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	push 2
	push 4
	pop EBX
	pop EAX
	xor EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
