;push <<valor>>
;call new_nodo
;add sp, 4
;retorna el puntero al nodo en eax

new_nodo: PUSH BP   
        MOV BP, SP

        PUSH nodoSize
        CALL alloc
        ADD SP, 4
        ;RETORNA EN EAX
        CMP EAX, NULL
        JZ new_fin

        MOV [EAX+val], [BP+8]
        MOV [EAX+der], NULL
        MOV [EAX+izq], NULL

new_fin: MOV SP, BP
        POP BP
        RET
        