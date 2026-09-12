#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint8_t memoria[16384];                // RAM 16 KiB
    int32_t registros[32];                 // 32 Registros
    SegmentDescriptor TablaSegmentos[8];    // Tabla de segmentos- segmentdescriptor nombre provisorio
} MaquinaVirtual;