;push <<*raiz>>
;call mostrar_preorden
;add sp, 4

mostrar_preorden: PUSH BP    
        MOV BP, SP

        PUSH EBX
        MOV EBX, [BP+8]

        CMP EBX, NULL
        JZ fin_inorden

        ;visitar raiz
        MOV [DS], [EBX+VAL]
        MOV EDX, DS
        MOV EAX, 0x01
        LDL ECX, 0x01
        LDH ECX, 0x04
        SYS 0x2
        
        ;mostrar subarbol izq
        PUSH [EBX+izq]
        CALL mostrar_preorden
        ADD SP, 4

        ;mostrar subarbol der
        PUSH [EBX+der]
        CALL mostrar_preorden
        ADD SP, 4

fin_preorden: POP EBX
    MOV SP, BP
    POP BP
    RET