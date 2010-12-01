.686
.model flat,stdcall
option casemap :none
include include\windows.inc
include include\kernel32.inc
include include\masm32.inc
includelib lib\masm32.lib
includelib lib\kernel32.lib
.data
	G_X dd 0
	G_writebuf dd 256 dup(0)
.code
main:
	push G_X
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 10
	mov G_writebuf + 4, 0
	invoke StdOut, offset G_writebuf
	push G_X
	push 99
	pop EAX
	mov G_X, EAX
	push 99
	pop EAX
	xor EBX, EBX
	sub EBX, EAX
	mov EAX, EBX
	push EAX
	pop EAX
	xor EBX, EBX
	sub EBX, EAX
	mov EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push G_X
	pop EAX
	xor EBX, EBX
	sub EBX, EAX
	mov EAX, EBX
	push EAX
	pop EAX
	xor EBX, EBX
	sub EBX, EAX
	mov EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push G_X
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push 100
	pop EAX
	xor EBX, EBX
	sub EBX, EAX
	mov EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	mov G_writebuf, 'rrrt'
	mov G_writebuf + 4, 'r'
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
