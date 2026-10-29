;push <<numero>>
;push <<raiz*>>
;call buscar
;add sp, 8
;retorna en eax si lo encontro (1 = si / 0 = no)

buscar: PUSH BP
        MOV BP, SP

        PUSH EBX
        PUSH ECX

        MOV EBX, [BP+8] ;puntero a la raiz
        MOV ECX, [BP+12] ;valor a buscar
        MOV EAX, 0 ;inicializo retorno

        CMP EBX, NULL
        JZ buscar_fin

        CMP [EBX+val], ECX
        JZ buscar_encontro
        JP buscar_izq 
        JN buscar_der  ;opcional ponerlo

buscar_der: PUSH ECX
        PUSH [EBX+der]
        CALL buscar
        ADD SP, 8
        ;RETORNA EN EAX
        JMP buscar_fin

buscar_izq: PUSH ECX
        PUSH [EBX+izq]
        CALL buscar
        ADD SP, 8
        ;RETORNA EN EAX
        JMP buscar_fin

buscar_encontro: MOV EAX, 1

buscar_fin: POP ECX
        POP EBX
        MOV SP, BP
        POP BP
        RET

