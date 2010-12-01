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
	push 5
	push 13
	push 10
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
	invoke ExitProcess, 0
end main
