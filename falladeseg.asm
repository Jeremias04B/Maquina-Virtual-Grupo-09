inicio: mov edx, 0xFFFFFFFF ; Carga un puntero con segmento -1 (inválido)
        mov eax, [edx]      ; Falla: intenta acceder a memoria con puntero inválido
        stop


;tambien se puede haceer un mov edx[ds+80000] para q tire el error de fuera de limites