.586P
.MODEL FLAT, STDCALL; �������� ������ ������; ������� ���� ���������� �� ����� ���� ���������

include \MASM32\INCLUDE\gdi32.inc
include \MASM32\INCLUDE\user32.inc
include \MASM32\INCLUDE\kernel32.inc
includelib \MASM32\LIB\gdi32.lib
includelib \MASM32\LIB\user32.lib
includelib \MASM32\LIB\kernel32.lib


.data

a   dd 54
b   dd 81
d   dd -26; �������� ����������, �������� � ����
e   dd 20
f   dd 110

bufferforstring db 10 dup(0); ��������� ������ ��� ����������������� ������
titlestring db "���������", 0; ��������� ����
szformat db "%d", 0; ��������� �� ��������������� ����� � ������

.code

start:

mov    eax, a
add    eax, b; (a + b)
push   eax

mov    eax, d
sub    eax, e; (d - e)

mov    ebx, eax
pop    eax
xor    edx, edx; ������ ������� edx ��� ������� �� �������
idiv   ebx; (a + b) / (d - e)

imul   eax, f

invoke wsprintfA, ADDR bufferforstring, ADDR szformat, eax; �����������
invoke MessageBox, 0, ADDR bufferforstring, ADDR titlestring, 0h; ����� ����
;���������� ���� ���������, ������� ������ ���� ���������, �����, ���������, MB_OK 

push   0h
call   ExitProcess

end    start