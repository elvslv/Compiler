.686
.model flat,stdcall
option casemap :none
include include\windows.inc
include include\kernel32.inc
include include\masm32.inc
includelib lib\masm32.lib
includelib lib\kernel32.lib
.data
	G_I dd 0
	G_J dd 0
	G_writebuf dd 256 dup(0)
.code
main:
	push G_I
	push 158
	push 15
	push 7
	pop EBX
	pop EAX
	xor EDX, EDX
	idiv EBX
	push EAX
	pop EBX
	pop EAX
	sub EAX, EBX
	push EAX
	pop EAX
	mov G_I, EAX
	push G_J
	push G_I
	push 5
	pop EBX
	pop EAX
	xor EDX, EDX
	idiv EBX
	mov EAX, EDX
	push EAX
	pop EAX
	mov G_J, EAX
	push G_I
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push G_J
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 'trat'
	mov G_writebuf + 4, 'ta'
	invoke StdOut, offset G_writebuf
	push G_I
	push G_J
	pop EBX
	pop EAX
	xor EDX, EDX
	imul EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
