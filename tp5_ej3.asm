;push string
;call str_a_num
;add sp, 4
;devuelve en eax

str_a_num: push bp
	mov bp, sp
	push ebx
	push ecx
	mov ebx, [bp+8]
	mov eax, 0
	mov cl, b[ebx]  ;lee un bit
bucle:	mov cl, b[ebx]  ;lee un bit
	cmp cl, 0
	jz fin_SaN ;si es el fin de la cadena (\n)
	add ebx, 1
	sub cl, 48 ;convierto a ascci
	mul eax, 10
	add eax,cl
	jmp bucle
fin_SaN: pop ecx
	pop ebx	
	mov sp,bp
	pop bp
	ret

;__________________________________________________
;inciso B
;__________________________________________________
;push num
;push puntero del string a guardar
;call num_a_str
;add sp, 4

;se una una pila

num_a_str: PUSH BP            
    MOV BP, SP

    ; Guardamos registros auxiliares
    PUSH EAX            ; Número original
    PUSH EBX            ; Auxiliar para el resto
    PUSH ECX            ; Contador de dígitos apilados
    PUSH EDX            ; Puntero al string
    PUSH EEX            ; Auxiliar para multiplicación

    MOV EAX, [BP+12]    ; EAX = Número
    MOV EDX, [BP+8]     ; EDX = Puntero al string

    XOR ECX, ECX        ; ECX = 0 (Contador de dígitos)

    ; Caso especial: si el número es 0
    CMP EAX, 0
    JNZ extraer_digitos
    MOV b[EDX], '0'     ; Escribimos el carácter '0'
    ADD EDX, 1          ; Avanzamos el puntero
    JMP fin_string      ; Vamos directo a poner el terminador

extraer_digitos: CMP EAX, 0          ; ¿Ya no quedan más dígitos por dividir?
    JZ escribir_string  ; Si EAX es 0, terminamos de extraer y pasamos a escribir

    ; --- Cálculo del Resto en MV2 ---
    MOV EBX, EAX        ; Copiamos el número actual a EBX
    DIV EAX, 10         ; EAX = Cociente (Num / 10)
    MOV EEX, EAX        ; EEX = Cociente
    MUL EEX, 10         ; EEX = Cociente * 10
    SUB EBX, EEX        ; EBX = Num - (Cociente * 10) -> ¡Esto es el Resto!

    PUSH EBX            ; Apilamos el resto (el dígito) para invertir su orden
    ADD ECX, 1          ; Incrementamos el contador de dígitos
    JMP extraer_digitos

escribir_string: CMP ECX, 0          ; ¿Escribimos todos los dígitos apilados?
    JZ fin_string       ; Si el contador es 0, terminamos

    POP EBX             ; Sacamos el dígito más significativo de la pila
    ADD EBX, 48         ; Lo convertimos a ASCII sumando 48 (o '0')
    
    MOV b[EDX], BL      ; Escribimos 1 byte (el carácter) en la memoria
    ADD EDX, 1          ; Avanzamos el puntero del string
    SUB ECX, 1          ; Descontamos el dígito escrito
    JMP escribir_string

fin_string: MOV b[EDX], 0       ; Agregamos el terminador '\0' al final del string

    ; Restauramos registros
    POP EEX
    POP EDX
    POP ECX
    POP EBX
    POP EAX
    
    POP BP
    RET



