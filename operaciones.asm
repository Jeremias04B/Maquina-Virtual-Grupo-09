;push op1
;push op2
;call suma
;add sp 8
;retora en eax
suma: 	push bp
	mov bp,sp
	mov eax, [bp+12]
	add eax, [bp+8]
	mov sp,bp
	pop bp
	ret

;push op1
;push op2
;call resta
;add sp 8
;retora en eax
sub: 	push bp
	mov bp,sp
	mov eax, [bp+12]
	sub eax, [bp+8]
	mov sp,bp
	pop bp
	ret

;push op1
;push op2
;call multi
;add sp 8
;retora en eax
multi: 	push bp
	mov bp,sp
	mov eax, [bp+12]
	mul eax, [bp+8]
	mov sp,bp
	pop bp
	ret

;push op1
;push op2
;call divi
;add sp 8
;retora en eax
divi: 	push bp
	mov bp,sp
	mov eax, [bp+12]
	div eax, [bp+8]
	mov sp,bp
	pop bp
	ret