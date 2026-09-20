#include <stdio.h>
#include <stdlib.h>
#include "maquinaV.h"
#include "instrucciones.h"

void ciclo_ejecucion(MaquinaVirtual *vm){

    //[0] del registro es el IP. el STOP pone el ip en -1
    while ( (vm->registros[REG_IP] != 0xFFFFFFFF) && ((vm->registros[REG_IP] & 0xFFFF) < vm->TablaSegmentos[REG_IP].tamano) ) {

        // 1. traduccion de la direccion logica (IP) a fisica
        uint16_t segmento = (vm->registros[REG_IP] >> 16) & 0xFFFF; // extrae los 2 bytes altos (Segmento)
        uint16_t offset   = vm->registros[REG_IP] & 0xFFFF;         // extrae los 2 bytes bajos (Desplazamiento)
        // se toma la base desde la tabla y se le suma el desplazamiento
        uint32_t dir_fisica = vm->TablaSegmentos[segmento].base + offset;

        uint8_t primer_byte = vm->memoria[dir_fisica];

        //guardo el codigo de operacion
        vm->registros[REG_OPC]=primer_byte & 0x1F;  

        uint32_t tipo_opA = (primer_byte & 0x30) >> 4;
        // //ubicarlos en el byte más alto de OP1
        vm->registros[REG_OP1] = tipo_opA << 24;

        uint32_t tipo_opB = (primer_byte & 0xC0) >> 6;
        //ubicarlos en el byte ms alto de OP2
        vm->registros[REG_OP2] = tipo_opB << 24;

        int offset_instruccion = 1; 
        // 1. extraer opB
        uint32_t valor_opB = 0;
        // el tamaño del operando en bytes coincide con su codigo numerico 
        for (int i = 0; i < tipo_opB; i++) { 
            // desplazamos 8 bits a la izquierda y unimos el nuevo byte de memoria fisica
            valor_opB = (valor_opB << 8) | vm->memoria[dir_fisica + offset_instruccion];
            offset_instruccion++;
        }
        vm->registros[REG_OP2] = vm->registros[REG_OP2] | valor_opB;

        // 2. extraer opA
        uint32_t valor_opA = 0;
        for (int i = 0; i < tipo_opA; i++) {
            valor_opA = (valor_opA << 8) | vm->memoria[dir_fisica + offset_instruccion];
            offset_instruccion++;
        }
        vm->registros[REG_OP1] = vm->registros[REG_OP1] | valor_opA;
        
        //offset_instruccion contiene el tamaño total exacto de la instruccion
        // incremento IP
        vm->registros[REG_IP] += offset_instruccion; 
        
        ejecutar_instruccion(vm);
    }
    
    void ejecutar_instruccion(MaquinaVirtual *vm) {
        uint32_t opcode = vm->registros[REG_OPC];
        if (opcode < 32) {
            vectorIns[opcode](vm);
        } 
        else {
            INVALID(vm); 
        }
    }
}