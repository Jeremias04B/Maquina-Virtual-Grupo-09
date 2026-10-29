\\DATA 1024
\\STACK 2048
\\INCLUDE "tp5_ej3.asm"
\\INCLUDE "operaciones.asm"
IGUAL EQU "= "
SALTO EQU "\n"

main:	push bp
	mov bp, sp
	; 1. Recuperar el puntero al arreglo de parámetros (argv)
    	MOV EBX, [BP+12]     ; EBX = Puntero al inicio del array argv

    	; 2. Leer y convertir OP1 ("45") -> Está en argv (EBX + 4)
   	MOV EDX, [EBX+4]     ; EDX = Puntero a "45"
   	PUSH EDX
   	CALL str_a_num      ; Devuelve OP1 en EAX
  	ADD SP, 4
   	MOV ECX, EAX        ; Guardamos OP1 en ECX

   	; 3. Leer y convertir OP2 ("20") -> Está en argv (EBX + 12)
   	MOV EDX, [EBX+12]   ; EDX = Puntero a "20"
   	PUSH EDX
   	CALL str_a_num      ; Devuelve OP2 en EAX
  	ADD SP, 4
  	MOV EEX, EAX        ; Guardamos OP2 en EEX

	; 4. Preparamos los parámetros para las subrutinas matemáticas
   	PUSH ECX            ; Apilamos OP1
    	PUSH EEX            ; Apilamos OP2

   	; 5. Leer el operador ("+") -> Está en argv (EBX + 8)
   	MOV EDX, [EBX+8]    ; EDX = Puntero a "+"
    	MOV AL, b[EDX]      ; Leemos 1 byte (el carácter del operador)

  	CMP AL, '+'
  	JZ hacer_suma
   	CMP AL, '-'
	JZ hacer_resta
   	CMP AL, '*'
   	JZ hacer_multi
  	CMP AL, '/'
   	JZ hacer_divi
  	JMP fin_prog        ; Si no es ninguno, salimos
	
hacer_suma: CALL suma
    	JMP mostrar_resultado
hacer_resta: CALL sub
    	JMP mostrar_resultado
hacer_multi: CALL multi
    	JMP mostrar_resultado
hacer_divi: CALL divi
    	JMP mostrar_resultado

mostrar_resultado: ADD SP, 8   ; Limpiamos los operandos de la pila

    	; Imprimir "= "
    	MOV EDX, KS
    	ADD EDX, IGUAL
    	SYS 0x04            ; Imprime el string "= "

    	; Guardar el resultado en DS e imprimirlo en decimal
    	MOV EDX, DS         ; EDX apunta al inicio de DS (Segmento 3)
    	MOV [EDX], EAX      ; Guardamos EAX en la dirección DS:0000

    	MOV EAX, 0x1        ; Modo decimal
    	LDH ECX, 0x04       ; Cantidad = 1, Tamaño = 4 bytes (long)
    	LDL ECX, 0x01
    	SYS 0x02            ; Imprime el entero ubicado en [EDX]

    	; Salto de línea final
    	MOV EDX, KS
    	ADD EDX, SALTO
    	SYS 0x04

fin_prog: STOP
