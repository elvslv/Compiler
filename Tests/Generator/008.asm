.686
.model flat,stdcall
option casemap :none
include include\windows.inc
include include\kernel32.inc
include include\masm32.inc
includelib lib\masm32.lib
includelib lib\kernel32.lib
.data
	G_B dd 0
	G_D dd 0
	G_writebuf dd 256 dup(0)
.code
main:
	push G_D
	push 20
	pop EAX
	xor EBX, EBX
	sub EBX, EAX
	mov EAX, EBX
	push EAX
	pop EAX
	mov G_D, EAX
	push G_B
	push G_D
	push 10
	push G_D
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
	mov G_B, EAX
	push G_D
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push G_B
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
