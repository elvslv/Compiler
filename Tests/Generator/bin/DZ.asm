.586P
.MODEL FLAT, STDCALL; сплошная модель памяти; функции сами выкидывают из стека свои аргументы

include \MASM32\INCLUDE\gdi32.inc
include \MASM32\INCLUDE\user32.inc
include \MASM32\INCLUDE\kernel32.inc
includelib \MASM32\LIB\gdi32.lib
includelib \MASM32\LIB\user32.lib
includelib \MASM32\LIB\kernel32.lib


.data

a   dd 54
b   dd 81
d   dd -26; исходные переменные, задаются в коде
e   dd 20
f   dd 110

bufferforstring db 10 dup(0); выделение буфера для сконвертированной строки
titlestring db "Результат", 0; заголовок окна
szformat db "%d", 0; указывает на конвертирование числа в строку

.code

start:

mov    eax, a
add    eax, b; (a + b)
push   eax

mov    eax, d
sub    eax, e; (d - e)

mov    ebx, eax
pop    eax
xor    edx, edx; чистим регистр edx для остатка от деления
idiv   ebx; (a + b) / (d - e)

imul   eax, f

invoke wsprintfA, ADDR bufferforstring, ADDR szformat, eax; конвертация
invoke MessageBox, 0, ADDR bufferforstring, ADDR titlestring, 0h; вывод окна
;дескриптор окна владельца, которое создаёт окно сообщения, текст, заголовок, MB_OK 

push   0h
call   ExitProcess

end    start