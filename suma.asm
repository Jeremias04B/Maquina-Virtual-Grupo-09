inicio:
    MOV   EAX, 0x01        ; Modo decimal
    MOV   EDX, DS          ; Carga la base lógica del segmento de datos
    ADD   EDX, 4           ; Posición inicial segura (offset 4)
    LDH   ECX, 4           ; 4 bytes por número
    LDL   ECX, 2           ; Se leen 2 números por teclado
    SYS   0x1              ; Almacena en [EDX] y [EDX+4]

    MOV   EBX, [EDX]       ; Carga el primer número leído
    ADD   EBX, [EDX+4]     ; Le suma el segundo número

    ADD   EDX, 8           ; EDX avanza al offset 12 (4 + 8 = 12)
    MOV   [EDX], EBX       ; Guarda la suma en memoria usando el puntero [EDX]

    MOV   EAX, 0x01        ; Modo decimal para imprimir
    LDH   ECX, 4           ; Tamaño de 4 bytes
    LDL   ECX, 1           ; 1 solo número a imprimir
    SYS   0x2              ; Imprime el dato apuntado por EDX
    STOP                   ; Detiene la ejecución