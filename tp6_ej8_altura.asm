;push <<raiz*>>
;call altura
;add sp, 4
;ret en eax

altura: PUSH BP
        MOV BP, SP

        PUSH EBX
        PUSH ECX
        PUSH EDX
        MOV EBX, [BP+8]
        
        MOV EAX, 0 ;la altura inicial es 0
        CMP EBX, NULL
        JZ altura_fin 

        ;bajo por la izquierda
        PUSH [EBX+izq]
        CALL altura
        ADD SP, 4
        MOV ECX, EAX

        ;bajo por la derecha
        PUSH [EBX+der]
        CALL altura
        ADD SP, 4
        MOV EDX, EAX

        CMP ECX, EDX
        JN alt_der_mayor
        JNN alt_izq_mayor  ;opcional ponerlo

alt_izq_mayor: MOV EAX, ECX
        ADD EAX, 1
        JMP altura_fin

alt_der_mayor: MOV EAX, EDX
        ADD EAX, 1
        JMP altura_fin  ;opcional ponerlo
    
altura_fin: POP EDX
        POP ECX
        POP EBX
        MOV SP, BP
        POP BP
        RET