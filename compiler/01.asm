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
	G_I dd 0
	G_J dd 0
	G_N dd 0
	tmp dd 0
	format0 db '%d %f %d', 10, 0
.code
main:
	finit

	push offset G_N
	push 15
	pop EAX
	pop EBX
	mov [EBX + 0], EAX
	push offset G_J
	mov tmp, 41cc0000h
	fld tmp
	sub ESP, 4
	fstp dword ptr[ESP + 0]
	pop EAX
	pop EBX
	mov [EBX + 0], EAX
	label0:
	fld G_J
	sub ESP, 4
	fstp dword ptr[ESP + 0]
	push G_N
	pop EAX
	mov tmp, EAX
	fild tmp
	sub ESP, 4
	fstp dword ptr[ESP + 0]
	finit

	pop EAX
	pop EBX
	cmp EAX, EBX
	mov EAX, 0
	mov ECX, 1
	cmovl EAX, ECX
	push EAX
	pop EAX
	test EAX, EAX
	jz label1
	label4:
	push 20
	push G_N
	pop EAX
	pop EBX
	cmp EAX, EBX
	mov EAX, 0
	mov ECX, 1
	cmove EAX, ECX
	push EAX
	pop EAX
	test EAX, EAX
	jz label2
	push offset G_N
	push 1
	push G_N
	pop EAX
	pop EBX
	add EAX, EBX
	push EAX
	pop EAX
	pop EBX
	mov [EBX + 0], EAX
	jmp label0
	jmp label2
	label2:
	push offset G_I
	push 1
	push G_I
	pop EAX
	pop EBX
	add EAX, EBX
	push EAX
	pop EAX
	pop EBX
	mov [EBX + 0], EAX
	push offset G_N
	push 5
	push G_N
	pop EAX
	pop EBX
	add EAX, EBX
	push EAX
	pop EAX
	pop EBX
	mov [EBX + 0], EAX
	jmp label0
	label1:
	push G_I
	fld G_J
	sub ESP, 4
	fstp dword ptr[ESP + 0]
	fld dword ptr[ESP + 0]
	sub ESP, 4
	fstp qword ptr[ESP + 0]
	push G_N
	push offset format0
	call crt_printf

	add ESP, 20
	invoke ExitProcess, 0
end main
