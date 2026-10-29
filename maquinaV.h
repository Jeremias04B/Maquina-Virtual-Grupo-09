
#ifndef MAQUINAV_H
#define MAQUINAV_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define RAM_SIZE 16384 // 16 KiB por defecto

// =====================================================================
// ÍNDICES DE REGISTROS DE LA MÁQUINA VIRTUAL (32 posiciones) [cite: 132]
// =====================================================================
#define REG_IP  0
#define REG_OPC 1
#define REG_OP1 2
#define REG_OP2 3
#define REG_LAR 4
#define REG_MAR 5
#define REG_MBR 6
#define REG_SP  7
#define REG_BP  8

#define REG_EAX 10
#define REG_EBX 11
#define REG_ECX 12
#define REG_EDX 13
#define REG_EEX 14
#define REG_EFX 15

#define REG_AC  16
#define REG_CC  17

#define REG_CS  26
#define REG_DS  27
#define REG_ES  28
#define REG_SS  29
#define REG_KS  30
#define REG_PS  31

// =====================================================================
// MÁSCARAS PARA EL REGISTRO CC (Flags) [cite: 6]
// =====================================================================
#define FLAG_N (1U << 31)
#define FLAG_Z (1U << 30)
#define FLAG_C (1U << 29)
#define FLAG_V (1U << 28)

// =====================================================================
// ÍNDICES DE SEGMENTOS EN LA TABLA DE DESCRIPTORES
// =====================================================================
#define SEG_PS 0
#define SEG_KS 1
#define SEG_CS 2
#define SEG_DS 3
#define SEG_ES 4
#define SEG_SS 5

// =====================================================================
// ESTRUCTURAS PRINCIPALES
// =====================================================================
typedef struct {
    uint16_t base;   // Dirección física base (2 bytes) [cite: 121]
    uint16_t size;   // Tamaño del segmento en bytes (2 bytes) [cite: 121]
} SegmentDescriptor;

typedef struct {
    uint8_t *memoria;             // RAM de 16 KiB [cite: 120]
    uint32_t tam_memoria;
    int32_t registros[32];                 // 32 Registros de 32 bits [cite: 120]
    SegmentDescriptor TablaSegmentos[8];    // Tabla de descriptores de segmentos [cite: 121]
    char vmi_filename[256]; 
    uint8_t PAP;
} MaquinaVirtual;
#endif /* MAQUINAV\_H \*/