inicio:
	mov edx, ds
	mov eax, 0x01
	ldh ecx, 0x04
	ldl ecx, 0x01
	sys 0x1
	mov efx, [0]
	mov ecx, 2
	cmp efx, 2
	jn noEsPrimo
primo:
	cmp efx, ecx 
	jz esPrimo  ;ya probe todos los posibles divisores y no fue ninguno
	mov ebx, efx

dividir: 
	sub ebx,ecx 
	jz noEsPrimo
	jp dividir
	;si dio negativo
	add ecx,1 
	jmp primo  ;vuelvo a probar

noEsPrimo:
	mov eex, 0
	jmp fin
EsPrimo:
	mov eex, 1
	jmp fin

fin:
	mov [0], eex 
	mov edx, ds
	mov eax, 0x01
	ldh ecx, 0x04
	ldl ecx,0x01
	sys 0x02
	
		
	stop