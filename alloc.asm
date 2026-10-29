;push <<cant bytes>>
;call alloc
;add sp, 4
;retorna en eax

alloc: PUSH BP  
    MOV BP, SP  
    PUSH BX
    
    MOV EAX, NULL
    MOV BX, [ES] ;guardo el offset
    ADD BX, [BP+8] ;le sumo la cant de bytes
    CMP BX, 2048
    JP alloc_fin    ;error se quiere guardar mas memoria de la que hay
    MOV EAX, [ES]
    ADD [ES], [BP+8]     


alloc_fin: POP BX   
    MOV SP, BP  
    POP BP
    RET