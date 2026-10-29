;PUSH << nodo*>>
;push << raiz**>>
;call ins_abb
;add sp, 8

ins_abb: PUSH BP
        MOV BP, SP
        
        PUSH EAX
        PUSH EBX
        PUSH EDX

        MOV EDX, [BP+8] ;puntero doble a la raiz
        MOV EBX, [EDX] ;puntero simple a la raiz
        MOV EAX, [BP+12];puntero simple al nodo

        CMP EBX, NULL
        JZ insertar ;cuando se cae inserta

        CMP [EBX+val], [EAX+val]
        JP ins_abb_izq

        ;else entro a ins_abb_der

ins_abb_der: ADD EBX, der ;ebx queda siendo un puntero doble  ESENCIAL
        PUSH EAX ;puntero al nodo
        PUSH EBX
        CALL ins_abb
        ADD SP, 8
        JMP ins_abb_fin

ins_abb_izq: ADD EBX, izq  ;ESENCIAL
        PUSH EAX
        PUSH EBX
        CALL ins_abb
        ADD SP, 8
        JMP ins_abb_fin

insertar:  MOV [EDX], EAX ;la actual raiz apunta al nuevo nodo *raiz = *nodo  ESENCIAL

ins_abb_fin: POP EDX
        POP EBX
        POP EAX
        MOV SP, BP
        POP BP
        RET
