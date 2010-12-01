.586P
.MODEL FLAT, STDCALL

include \masm32\include\user32.inc
include \masm32\include\kernel32.inc

includelib \masm32\lib\user32.lib
includelib \masm32\lib\kernel32.lib

.data

Ttl db "После долгих попыток...", 0h
Msg db "Оно всё-таки работает! :)",0h

.code

start: 
     push 0h 
         push offset Ttl 
         push offset Msg 
         push 0h 
         call MessageBoxA 
         push 0h 
         call ExitProcess 
end    start