#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include "maquinaV.h"
#include "instrucciones.h"
#include "funciones.h"

uint16_t traducirD(MaquinaVirtual *vm, uint32_t dirLogica, uint16_t access_bytes) { // access_bytes representa la cantidad de bytes que se van a leer o escribir en la memoria
    uint16_t segmento = (dirLogica >> 16) & 0xFFFF; // 2 bytes +sig
    uint16_t offset = dirLogica & 0xFFFF;          // 2 bytes -sig

    if (segmento >= 8 || vm->TablaSegmentos[segmento].base == 0xFFFF) {
        fprintf(stderr, "Error: Fallo de segmentoo (segmentoo inválido)\n"); //stderr a revisar!
        exit(1);
    }

    uint16_t base = vm->TablaSegmentos[segmento].base;
    uint16_t size = vm->TablaSegmentos[segmento].size;

    if (offset + access_bytes > size) {
        fprintf(stderr, "Error: Fallo de segmento (Fuera de límites)\n");
        exit(1);
    }

    uint16_t dirFisica = base + offset;

    // Actualiza registros hardware de control de memoria
    vm->registros[REG_LAR] = dirLogica;
    vm->registros[REG_MAR] = ((uint32_t)access_bytes << 16) | dirFisica;

    return dirFisica;
} 
// Lectura de 32 bits en Big-Endian (MSB en la dirección base)
int32_t read_mem32(MaquinaVirtual *vm, uint16_t dirFisica) {
    int32_t val = 0;
    val |= ((uint32_t)vm->memoria[dirFisica])     << 24;
    val |= ((uint32_t)vm->memoria[dirFisica + 1]) << 16;
    val |= ((uint32_t)vm->memoria[dirFisica + 2]) << 8;
    val |= ((uint32_t)vm->memoria[dirFisica + 3]);
    
    vm->registros[REG_MBR] = val;
    return val;
}

// Escritura de 32 bits en Big-Endian (MSB en la dirección base)
void write_mem32(MaquinaVirtual *vm, uint16_t dirFisica, int32_t val) {
    vm->registros[REG_MBR] = val;
    vm->memoria[dirFisica]     = (val >> 24) & 0xFF;
    vm->memoria[dirFisica + 1] = (val >> 16) & 0xFF;
    vm->memoria[dirFisica + 2] = (val >> 8)  & 0xFF;
    vm->memoria[dirFisica + 3] =  val        & 0xFF;
}



// Auxiliar para resolver la dirección lógica y la cantidad de bytes de un operando de memoria MV2
uint32_t resolver_dir_memoria_mv2(MaquinaVirtual *vm, uint32_t op_val, uint16_t *out_cant_bytes) {
    uint8_t cod_reg = op_val & 0x1F;                     // Bits 0-4: Código del registro base
    uint8_t bits_cant = (op_val >> 5) & 0x07;            // Bits 5-7: Modificador de tamaño (b, w, l)
    int16_t offset = (int16_t)((op_val >> 8) & 0xFFFF);  // Bits 8-23: Desplazamiento con signo

    // Mapear modificador binario a cantidad de bytes real
    if (bits_cant == 1)      *out_cant_bytes = 1; // 'b' (1 byte)
    else if (bits_cant == 2) *out_cant_bytes = 2; // 'w' (2 bytes / word)
    else                     *out_cant_bytes = 4; // 'l' (4 bytes / long, por defecto)

    return vm->registros[cod_reg] + offset;
}

// Obtener valor de operando según su tipo en OP1/OP2 (Actualizado para MV2)
int32_t getValorOP(MaquinaVirtual *vm, uint32_t reg_op) {
    uint8_t tipo = (reg_op >> 24) & 0xFF;
    uint32_t val = reg_op & 0x00FFFFFF;  

    switch (tipo) {
        case 0x01: { // Registro (Soporte para subsectores AL, AH, AX, EAX)
            uint8_t cod_reg = val & 0x1F;
            uint8_t sec_reg = (val >> 5) & 0x03; // Bits 5-6: Sector de registro
            int32_t reg_val = vm->registros[cod_reg];

            switch (sec_reg) {
                case 0: return reg_val;                        // 00: Registro completo (32 bits, ej: EAX)
                case 1: return (int8_t)(reg_val & 0xFF);       // 01: Byte bajo (ej: AL)
                case 2: return (int8_t)((reg_val >> 8) & 0xFF);// 10: Byte alto (ej: AH)
                case 3: return (int16_t)(reg_val & 0xFFFF);    // 11: Word bajo (16 bits, ej: AX)
                default: return reg_val;
            }
        }
        case 0x02: // Inmediato (16 bits signed)
            return (int16_t)(val & 0xFFFF);

        case 0x03: { // Memoria (Soporte para modificadores b, w, l)
            uint16_t cant_bytes = 4;
            uint32_t dirLogica = resolver_dir_memoria_mv2(vm, val, &cant_bytes);
            uint16_t phys = traducirD(vm, dirLogica, cant_bytes);

            int32_t dato = 0;
            if (cant_bytes == 1) {
                dato = (int8_t)vm->memoria[phys];
            } else if (cant_bytes == 2) {
                dato = (int16_t)(vm->memoria[phys] | (vm->memoria[phys + 1] << 8));
            } else {
                dato = read_mem32(vm, phys);
            }

            vm->registros[REG_MBR] = dato; // Cargar MBR según especificación
            return dato;
        }
        default:
            return 0;
    }
}

// Escribir resultado en el destino (Registro o Memoria - Actualizado para MV2)
void setValor(MaquinaVirtual *vm, uint32_t reg_op, int32_t val) {
    uint8_t tipo = (reg_op >> 24) & 0xFF;
    uint32_t target = reg_op & 0x00FFFFFF;

    if (tipo == 0x01) { // Registro (Soporte para subsectores)
        uint8_t cod_reg = target & 0x1F;
        uint8_t sec_reg = (target >> 5) & 0x03;

        switch (sec_reg) {
            case 0: // 00: Registro completo (32 bits)
                vm->registros[cod_reg] = val;
                break;
            case 1: // 01: Byte bajo (AL) - Preserva los 24 bits superiores
                vm->registros[cod_reg] = (vm->registros[cod_reg] & 0xFFFFFF00) | (val & 0xFF);
                break;
            case 2: // 10: Byte alto (AH) - Preserva los demás bits
                vm->registros[cod_reg] = (vm->registros[cod_reg] & 0xFFFF00FF) | ((val & 0xFF) << 8);
                break;
            case 3: // 11: Word bajo (AX) - Preserva los 16 bits superiores
                vm->registros[cod_reg] = (vm->registros[cod_reg] & 0xFFFF0000) | (val & 0xFFFF);
                break;
        }
    } else if (tipo == 0x03) { // Memoria (Soporte para 1, 2 o 4 bytes)
        uint16_t cant_bytes = 4;
        uint32_t dirLogica = resolver_dir_memoria_mv2(vm, target, &cant_bytes);
        uint16_t phys = traducirD(vm, dirLogica, cant_bytes);

        vm->registros[REG_MBR] = val; // Cargar MBR según especificación

        if (cant_bytes == 1) {
            vm->memoria[phys] = val & 0xFF;
        } else if (cant_bytes == 2) {
            vm->memoria[phys] = val & 0xFF;
            vm->memoria[phys + 1] = (val >> 8) & 0xFF;
        } else {
            write_mem32(vm, phys, val);
        }
    }
}


void actualizarCC(MaquinaVirtual *vm, int64_t result, bool check_overflow, bool carry) {
    uint32_t cc = 0;
    int32_t res32 = (int32_t)result;

    if (res32 < 0) cc |= FLAG_N;
    if (res32 == 0) cc |= FLAG_Z;
    if (carry) cc |= FLAG_C;
    if (check_overflow) cc |= FLAG_V;

    vm->registros[REG_CC] = cc;
}

char configmodo(int16_t v){
    switch (v){
        case (16): return 'B';
        break;
        case(8):return 'X';
        break;
        case(4): return 'O';
        break;
        case(2): return 'c';
        break;
        case(1): return 'd';
        break;
        
        default: printf("MODO INVALIDO");
    }
    return 0;
}

void guardar_imagen_vmi(MaquinaVirtual *vm, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: No se pudo crear o abrir el archivo de imagen %s\n", filename);
        return;
    }

    fwrite("VMI26", sizeof(char), 5, file);

    uint8_t version = 1;
    fwrite(&version, sizeof(uint8_t), 1, file);

    uint16_t ram_kib = (uint16_t)(vm->tam_memoria / 1024);
    fwrite(&ram_kib, sizeof(uint16_t), 1, file);

    fwrite(vm->registros, sizeof(int32_t), 32, file);

    fwrite(vm->TablaSegmentos, sizeof(SegmentDescriptor), 8, file);

    fwrite(vm->memoria, sizeof(uint8_t), vm->tam_memoria, file);

    fclose(file);
}

