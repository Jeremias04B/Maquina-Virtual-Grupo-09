\\DATA 1024
\\STACK 2048
\\EXTRA 2048

nodoSize EQU 12
val EQU 0
izq EQU 4
der EQU 8
NULL EQU -1

\\INCLUDE "heap.asm"
\\INCLUDE "alloc.asm"
\\INCLUDE "tp6_ej8_crear_nodo.asm"
\\INCLUDE "tp6_ej8_buscar.asm"
\\INCLUDE "tp6_ej8_inorden.asm"
\\INCLUDE "tp6_ej8_preorden.asm"
\\INCLUDE "tp6_ej8_postorden.asm"
\\INCLUDE "tp6_ej8_insertar.asm"
\\INCLUDE "tp6_ej8_altura.asm"

main:   PUSH BP
        MOV BP, SP
        CALL heap_init


        MOV EDX, DS
        ADD EDX, 4
        MOV EAX, NULL
        MOV [EDX], EAX     

leer_otro: MOV EAX, 0x01    
        MOV EDX, DS        
        LDL ECX, 0x01       ; Cantidad = 1
        LDH ECX, 0x04       ; Tamaño = 4 bytes
        SYS 0x01            ; Syscall de lectura


        CMP [DS], 0
        JN leer_fin

        PUSH [DS]           
        CALL new_nodo      
        ADD SP, 4

   
        PUSH EAX          
        MOV EDX, DS
        ADD EDX, 4         
        PUSH EDX            
        CALL ins_abb
        ADD SP, 8

        JMP leer_otro       


leer_fin: MOV EDX, DS
        MOV EAX, [EDX+4]    
        PUSH EAX
        CALL mostrar_preorden
        ADD SP, 4

      
        MOV EDX, DS
        MOV EAX, [EDX+4]    
        PUSH EAX
        CALL mostrar_inorden
        ADD SP, 4

     
        MOV EDX, DS
        MOV EAX, [EDX+4]    
        PUSH EAX
        CALL mostrar_postorden
        ADD SP, 4

        MOV EDX, DS
        MOV EAX, [EDX+4]    
        PUSH EAX
        CALL altura        
        ADD SP, 4

        MOV EDX, DS
        MOV [EDX], EAX      
        MOV EAX, 0x01      
        LDL ECX, 0x01      
        LDH ECX, 0x04      
        SYS 0x02            ; Imprime la altura

       
        MOV EAX, 0x01
        MOV EDX, DS
        LDL ECX, 0x01
        LDH ECX, 0x04
        SYS 0x01           

        PUSH [DS]          
        MOV EDX, DS
        MOV EAX, [EDX+4]   
        PUSH EAX           
        CALL buscar         
        ADD SP, 8


        MOV EDX, DS
        MOV [EDX], EAX      
        MOV EAX, 0x01       
        LDL ECX, 0x01       
        LDH ECX, 0x04      
        SYS 0x02            ; Imprime 1 o 0

fin:    MOV SP, BP
        POP BP
        STOP