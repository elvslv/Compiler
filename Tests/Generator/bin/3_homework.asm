.386
.model flat

.data 

	dat db 1h, 1h, 1h

.code

WinMain PROC
	
	mov ecx, 0
	mov ebx, 0
	lea edx, dat
	mov al, [edx]
next_num:
	add edx, 1
	mov ah, [edx] 	

	cmp al, 1111b
	
	je not_add
	add ebx, 1
not_add:
	add ecx, 1
	cmp ecx, 3
	je next_num

	ret
WinMain ENDP

end WinMain
