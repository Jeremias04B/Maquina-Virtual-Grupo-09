#include <stdio.h>
#include <stdlib.h>
#include "maquinaV.h"
#include "instrucciones.h"

void sys_breakpoint(MaquinaVirtual *vm);

void ejecutar_instruccion(MaquinaVirtual *vm) {
    uint32_t opcode = vm->registros[REG_OPC];
    if (opcode < 32) {
        vectorIns[opcode](vm);
    } else {
        INVALID(vm); 
    }
}

void ciclo_ejecucion(MaquinaVirtual *vm) {

    // STOP pone IP en 0xFFFFFFFF
    while((uint32_t)vm->registros[REG_IP] != 0xFFFFFFFF){

         // verificacion de breakpoint
        if (vm->PAP == 1) {
            sys_breakpoint(vm);
        }

        // traduccion de la dirección logica a fisica
        uint16_t segmento = (vm->registros[REG_IP] >> 16) & 0xFFFF; 
        uint16_t offset   = vm->registros[REG_IP] & 0xFFFF;         

        // validar que el segmento de codigo sea valido y el IP este dentro de sus iímites
        if (segmento >= 8 || vm->TablaSegmentos[segmento].base == 0xFFFF) {
            fprintf(stderr, "Error: Fallo de segmento en IP (segmento invalido)\n");
            break;
        }

        if (offset >= vm->TablaSegmentos[segmento].size) {
            // el IP supero el tamaño del segmento de código = fin del proceso
            break;
        }

        // direccion física del byte de la instrucción actual
        uint32_t dir_fisica = vm->TablaSegmentos[segmento].base + offset;

        // leer el primer byte de la instruccion
        uint8_t primer_byte = vm->memoria[dir_fisica];

        int bit4 = (primer_byte >> 4) & 1;
        uint32_t tipo_opA, tipo_opB;

        //extraer opc y tipos de operandos
        if (bit4) {
            vm->registros[REG_OPC] = 0x10 | (primer_byte & 0x0F);
            tipo_opB = (primer_byte >> 6) & 0x3;
            tipo_opA = (primer_byte >> 4) & 0x3;
        } else {
            uint8_t op5 = primer_byte & 0x1F;
            vm->registros[REG_OPC] = op5;
            tipo_opB = 0; 
            tipo_opA = (op5 == 0x0F) ? 0 : ((primer_byte >> 6) & 0x3); // STOP (0x0F) no lleva operandos
        }

        // cargar los tipos en el byte mas alto de OP1 y OP2 
        vm->registros[REG_OP1] = tipo_opA << 24;
        vm->registros[REG_OP2] = tipo_opB << 24;

        int offset_instruccion = 1; 

        // extraer opb
        uint32_t valor_opB = 0;
         for (uint32_t i = 0; i < tipo_opB; i++) { 
            if (offset + offset_instruccion < vm->TablaSegmentos[segmento].size) {
                valor_opB = (valor_opB << 8) | vm->memoria[dir_fisica + offset_instruccion];
                offset_instruccion++;
            }
        }
        vm->registros[REG_OP2] |= valor_opB;

        // extraer opa
        uint32_t valor_opA = 0;
          for (uint32_t i = 0; i < tipo_opA; i++) {
            if (offset + offset_instruccion < vm->TablaSegmentos[segmento].size) {
                valor_opA = (valor_opA << 8) | vm->memoria[dir_fisica + offset_instruccion];
                offset_instruccion++;
            }
        }
        vm->registros[REG_OP1] |= valor_opA;
        
        //incrementar IP con el tamaño exacto de la instruccion antes de ejecutar
        vm->registros[REG_IP] += offset_instruccion; 
        
        // ejecutar la instruccion
        ejecutar_instruccion(vm);

    }
}