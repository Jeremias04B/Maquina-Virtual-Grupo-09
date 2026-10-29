;push <<raiz*>>
;call mostrar_postorden
;add sp, 4

mostrar_postorden: PUSH BP    
        MOV BP, SP

        PUSH EBX
        MOV EBX, [BP+8]

        CMP EBX, NULL
        JZ fin_postorden
        
        ;mostrar subarbol izq
        PUSH [EBX+izq]
        CALL mostrar_postorden
        ADD SP, 4

        ;mostrar subarbol der
        PUSH [EBX+der]
        CALL mostrar_postorden
        ADD SP, 4

        ;visitar raiz
        MOV [DS], [EBX+VAL]
        MOV EDX, DS
        MOV EAX, 0x01
        LDL ECX, 0x01
        LDH ECX, 0x04
        SYS 0x2

fin_postorden: POP EBX
    MOV SP, BP
    POP BP
    RET