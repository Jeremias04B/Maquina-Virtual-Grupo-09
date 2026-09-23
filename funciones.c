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

int32_t read_mem32(MaquinaVirtual *vm, uint16_t dirFisica) {
    int32_t val = 0;
    for (int i = 0; i < 4; i++) {
        val |= ((uint32_t)vm->memoria[dirFisica + i]) << (i * 8);
    } //leer byte a byte
    vm->registros[REG_MBR] = val;
    return val;
} 

void write_mem32(MaquinaVirtual *vm, uint16_t dirFisica, int32_t val) {
    vm->registros[REG_MBR] = val;
    for (int i = 0; i < 4; i++) {
        vm->memoria[dirFisica + i] = (val >> (i * 8)) & 0xFF;
    }
}

// Función auxiliar para resolver la dirección lógica real de un operando de memoria
uint32_t resolver_direccion_logica(MaquinaVirtual *vm, uint32_t op_val) {
    uint8_t cod_reg = op_val & 0x1F;                 // 5 bits menos significativos (registro)
    int16_t offset  = (int16_t)((op_val >> 8) & 0xFFFF); // 16 bits de desplazamiento con signo
    
    // Obtener el puntero lógico contenido en el registro y sumarle el desplazamiento
    uint32_t dir_base = vm->registros[cod_reg];
    return dir_base + offset;
}

int32_t getValorOP(MaquinaVirtual *vm, uint32_t reg_op) {
    uint8_t tipo = (reg_op >> 24) & 0xFF;
    uint32_t val = reg_op & 0x00FFFFFF;  

    switch (tipo) {
        case 0x01: // Registro (5 ult bits)
            return vm->registros[val & 0x1F];
        case 0x02: // Inmediato (16 bits signed)
            return (int16_t)(val & 0xFFFF);
        case 0x03: // Memoria (3 bytes empaquetados)
        {
            uint32_t dirLogica = resolver_direccion_logica(vm, val);
            uint16_t phys = traducirD(vm, dirLogica, 4);
            
            int32_t dato = read_mem32(vm, phys);
            vm->registros[REG_MBR] = dato; // Cargar MBR según especificación
            return dato;
        }
        default:
            return 0;
    }
}

// Escribir resultado en el destino (Registro o Memoria)
void setValor(MaquinaVirtual *vm, uint32_t reg_op, int32_t val) {
    uint8_t tipo = (reg_op >> 24) & 0xFF;
    uint32_t target = reg_op & 0x00FFFFFF;

    if (tipo == 0x01) { // Registro
        vm->registros[target & 0x1F] = val;
    } else if (tipo == 0x03) { // Memoria
        uint32_t dirLogica = resolver_direccion_logica(vm, target);
        uint16_t phys = traducirD(vm, dirLogica, 4);
        
        vm->registros[REG_MBR] = val; // Cargar MBR según especificación
        write_mem32(vm, phys, val);
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
}

