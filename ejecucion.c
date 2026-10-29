#include <stdio.h>
#include <stdlib.h>
#include "maquinaV.h"
#include "instrucciones.h"

// Prototipo de sys_breakpoint si está implementado en otro módulo
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

    // [IP != 0xFFFFFFFF] indica que el proceso sigue activo (STOP pone IP en 0xFFFFFFFF)
    while((uint32_t)vm->registros[REG_IP] != 0xFFFFFFFF){

        // 1. Traducción de la dirección lógica (IP) a física
        uint16_t segmento = (vm->registros[REG_IP] >> 16) & 0xFFFF; // 2 bytes altos: Segmento
        uint16_t offset   = vm->registros[REG_IP] & 0xFFFF;         // 2 bytes bajos: Desplazamiento

        // Validar que el segmento de código sea válido y el IP esté dentro de sus límites
        if (segmento >= 8 || vm->TablaSegmentos[segmento].base == 0xFFFF) {
            fprintf(stderr, "Error: Fallo de segmento en IP (segmento invalido)\n");
            break;
        }

        if (offset >= vm->TablaSegmentos[segmento].size) {
            // El IP superó el tamaño del segmento de código -> fin normal del proceso
            break;
        }

        // Dirección física del byte de la instrucción actual
        uint32_t dir_fisica = vm->TablaSegmentos[segmento].base + offset;

        // FETCH: Leer el primer byte de la instrucción
        uint8_t primer_byte = vm->memoria[dir_fisica];

        int bit4 = (primer_byte >> 4) & 1;
        uint32_t tipo_opA, tipo_opB;

        // DECODE: Extraer opcode y tipos de operandos
        if (bit4) {
            // Dos operandos: tipo B en bits 7-6, tipo A en bits 5-4
            vm->registros[REG_OPC] = 0x10 | (primer_byte & 0x0F);
            tipo_opB = (primer_byte >> 6) & 0x3;
            tipo_opA = (primer_byte >> 4) & 0x3;
        } else {
            // Uno o cero operandos: el tipo está en bits 7-6
            uint8_t op5 = primer_byte & 0x1F;
            vm->registros[REG_OPC] = op5;
            tipo_opB = 0; // No existe operando B
            tipo_opA = (op5 == 0x0F) ? 0 : ((primer_byte >> 6) & 0x3); // STOP (0x0F) no lleva operandos
        }

        // Cargar los tipos en el byte más alto de OP1 y OP2 (bits 31..24)
        vm->registros[REG_OP1] = tipo_opA << 24;
        vm->registros[REG_OP2] = tipo_opB << 24;

        int offset_instruccion = 1; 

        // 1. Extraer Operando B (se lee primero por codificación en orden inverso)
        uint32_t valor_opB = 0;
         for (uint32_t i = 0; i < tipo_opB; i++) { 
            if (offset + offset_instruccion < vm->TablaSegmentos[segmento].size) {
                valor_opB = (valor_opB << 8) | vm->memoria[dir_fisica + offset_instruccion];
                offset_instruccion++;
            }
        }
        vm->registros[REG_OP2] |= valor_opB;

        // 2. Extraer Operando A (se lee después)
        uint32_t valor_opA = 0;
          for (uint32_t i = 0; i < tipo_opA; i++) {
            if (offset + offset_instruccion < vm->TablaSegmentos[segmento].size) {
                valor_opA = (valor_opA << 8) | vm->memoria[dir_fisica + offset_instruccion];
                offset_instruccion++;
            }
        }
        vm->registros[REG_OP1] |= valor_opA;
        
        // UPDATE IP: incrementar IP con el tamaño exacto de la instrucción antes de ejecutar
        vm->registros[REG_IP] += offset_instruccion; 
        
        // EXECUTE: Ejecutar la instrucción
        ejecutar_instruccion(vm);

        // Verificación de Paso a Paso / Breakpoint
        if (vm->PAP == 1) {
            sys_breakpoint(vm);
        }
    }
}