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
	G_writebuf dd 256 dup(0)
.code
main:
	push G_A
	push 3
	pop EAX
	mov G_A, EAX
	push G_B
	push 6
	pop EAX
	mov G_B, EAX
	push 10
	pop EAX
	not EAX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push G_A
	push G_B
	pop EBX
	pop EAX
	and EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push G_B
	push G_A
	pop EBX
	pop EAX
	or EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	mov G_writebuf, ' '
	invoke StdOut, offset G_writebuf
	push G_A
	push 6
	pop EBX
	pop EAX
	xor EAX, EBX
	push EAX
	pop EAX
	invoke dwtoa, EAX, offset G_writebuf
	invoke StdOut, offset G_writebuf
	invoke ExitProcess, 0
end main
