.386
.MODEL flat, stdcall

.data

	arr dd 1, 2, 3, 4, 5, 6, 5, 4, 3, 2

.code

main proc

		mov ch, 9
loop_cl:
		mov cl, ch
loop_ch:
		
		xor ebx, ebx
		mov bl, cl
		mov eax, arr[ebx]
		add ebx, 4
		mov edx, arr[ebx]
		cmp eax, edx
		jle not_change
		
		mov arr[ebx], eax
		sub ebx, 4
		mov arr[ebx], edx
		
not_change:
		inc cl		
		cmp cl, 10
		jle loop_ch

		dec ch
		cmp ch, 0
		jge loop_cl	

		ret

watch:
		mov ebx, 0 
		mov eax, arr[ebx]
		cmp ebx, 10
		jle watch

main endp
          
end main

