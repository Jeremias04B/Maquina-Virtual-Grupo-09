#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "instrucciones.h"

void run_mv(MaquinaVirtual *vm) {
    while (!vm->registros[REG_IP] == -1) {
        // 1. Fetch & Decode (completa OPC, OP1, OP2 y avanza IP
        // ...
        
        uint8_t codIns = vm->registros[REG_OPC] & 0x1F;

        // 2. Dispatch a la instrucción correspondiente
        if (vectorIns[codIns] != NULL) {
            vectorIns[codIns](vm);
        } else {
            exec_INVALID(vm);
        }

        // Si el IP sale del segmento de código finaliza la VM
        if ((uint32_t)vm->registros[REG_IP] == 0xFFFFFFFF) {
            exit;
        }
    }
}