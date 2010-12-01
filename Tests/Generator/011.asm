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
	push 1
	push 2
	pop EBX
	pop EAX
	add EAX, EBX
	push EAX
	push 3
	pop EBX
	pop EAX
	xor EDX, EDX
	imul EBX
	push EAX
	push 4
	push 5
	pop EBX
	pop EAX
	sub EAX, EBX
	push EAX
	push 6
	pop EBX
	pop EAX
	xor EDX, EDX
	imul EBX
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
	invoke ExitProcess, 0
end main
