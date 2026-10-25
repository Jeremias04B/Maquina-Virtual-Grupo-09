#pragma once

#ifndef MAQUINAV_H
#define MAQUINAV_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define RAM_SIZE 16384 // 16 KiB

// Índices de Registros
#define REG_IP  0
#define REG_OPC 1
#define REG_OP1 2
#define REG_OP2 3
#define REG_LAR 4
#define REG_MAR 5
#define REG_MBR 6
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

// Mascaras para el Registro CC
#define FLAG_N (1 << 31)
#define FLAG_Z (1 << 30)
#define FLAG_C (1 << 29)
#define FLAG_V (1 << 28)

//para la 2da parte
// Índices de segmentos en la tabla
#define SEG_CS 0
#define SEG_DS 1
#define SEG_KS 2
#define SEG_ES 3
#define SEG_SS 4
#define SEG_PS 5

// Índices de registros (asegúrate de que coincidan con tu maquinaV.h)
#define REG_IP 0
#define REG_SP 24 // Ejemplo, ajustar según tu enum
#define REG_BP 25
#define REG_CS 26
#define REG_DS 27
#define REG_KS 28
#define REG_ES 29
#define REG_SS 30
#define REG_PS 31


typedef struct {
    uint16_t base;   // 2 bytes
    uint16_t size;   // 2 bytes
} SegmentDescriptor;

//STRUCT PRINCIPAL MAQUINA VIRTUAL

typedef struct {
    uint8_t memoria[16384];                // RAM 16 KiB
    int32_t registros[32];                 // 32 Registros
    SegmentDescriptor TablaSegmentos[8];    // Tabla de segmentos- segmentdescriptor nombre provisorio
} MaquinaVirtual;

#endif /* MAQUINAV_H */
