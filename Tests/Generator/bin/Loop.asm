.586P
.model flat, stdcall

include \MASM32\INCLUDE\user32.inc
include \MASM32\INCLUDE\kernel32.inc

includelib \MASM32\LIB\user32.lib
includelib \MASM32\LIB\kernel32.lib

.data

bufferforstring db 10 dup(0)
titlestring db "Ñ÷¸ò÷èê", 0
szformat db "%d", 0

.code

start:

    mov ecx, 10d
    jcxz m1

cycle:

print "ya"
 
    loop cycle

m1:

push 0h
call ExitProcess

end start