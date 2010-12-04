.686
.model flat,stdcall
option casemap :none
include include\windows.inc
include include\msvcrt.inc
include include\kernel32.inc
include include\masm32.inc
include include\fpu.inc
includelib lib\masm32.lib
includelib lib\msvcrt.lib
includelib lib\kernel32.lib
includelib lib\fpu.lib
.data
	G_A dd 0
	G_B dd 0
	G_K dd 0
	G_L dd 0
	tmp dd 0
	format0 db '%f %f%f%f', 10, 0
.code
main:
	finit

	push offset G_K
	mov tmp, 3f000000h
	fld tmp
	sub ESP, 4
	fstp dword ptr[ESP + 0]
	pop EAX
	pop EBX
	mov [EBX + 0], EAX
	push offset G_L
	fld1

	mov tmp, 437fc7aeh
	fld tmp
	fld G_K
	fdiv st(0), st(1)
	sub ESP, 4
	fstp dword ptr[ESP + 0]
	pop EAX
	pop EBX
	mov [EBX + 0], EAX
	fld1

	fld G_L
	fld G_K
	fadd st(0), st(1)
	sub ESP, 8
	fstp qword ptr[ESP + 0]
	fld1

	push 234
	pop EAX
	mov tmp, EAX
	fild tmp
	fld G_L
	fdiv st(0), st(1)
	sub ESP, 8
	fstp qword ptr[ESP + 0]
	fld G_K
	sub ESP, 8
	fstp qword ptr[ESP + 0]
	fld G_L
	sub ESP, 8
	fstp qword ptr[ESP + 0]
	push offset format0
	call crt_printf

	add ESP, 28
	invoke ExitProcess, 0
end main
