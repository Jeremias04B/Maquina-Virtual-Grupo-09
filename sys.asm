main:   push bp
        mov bp, sp

        MOV EAX, 0x01    
        MOV EDX, DS        
        MOV ECX, -1  
        SYS 0x03

        MOV EDX, DS  
        SYS 0x0F         
        SYS 0x04

        MOV SP, BP
        POP BP
        STOP