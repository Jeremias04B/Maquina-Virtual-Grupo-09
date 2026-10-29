;push <<raiz*>>
;call mostrar_inorden
;add sp, 4

mostrar_inorden: PUSH BP    
        MOV BP, SP

        PUSH EBX
        MOV EBX, [BP+8]

        CMP EBX, NULL
        JZ fin_inorden

        ;mostrar subarbol izq
        PUSH [EBX+izq]
        CALL mostrar_inorden
        ADD SP, 4

        ;visitar raiz
        MOV [DS], [EBX+VAL]
        MOV EDX, DS
        MOV EAX, 0x01
        LDL ECX, 0x01
        LDH ECX, 0x04
        SYS 0x2

        ;mostrar subarbol der
        PUSH [EBX+der]
        CALL mostrar_inorden
        ADD SP, 4

fin_inorden: POP EBX
    MOV SP, BP
    POP BP
    RET