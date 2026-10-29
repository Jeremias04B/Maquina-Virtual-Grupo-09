inicio: mov eax, 10
        mov ebx, 0
        div eax, ebx  ; Falla: intento de dividir por cero
        stop