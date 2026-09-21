
#include "maquinaV.h"
#include "funciones.h"

void SUB(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
    int64_t res = (int64_t)a - b;

    bool v = ((a > 0 && b < 0 && (int32_t)res < 0) || (a < 0 && b > 0 && (int32_t)res >= 0));
    bool c = (uint32_t)a < (uint32_t)b;

    setValor(vm, vm->registros[REG_OP1], (int32_t)res);
    actualizarCC(vm, res, v, c);
}

void MUL(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
    int64_t res = (int64_t)a * b;

    bool overflow = (res > INT32_MAX || res < INT32_MIN);

    setValor(vm, vm->registros[REG_OP1], (int32_t)res);
    actualizarCC(vm, res, overflow, overflow);
}

void DIV(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);

    if (b == 0) {
        fprintf(stderr, "Error: División por cero\n");
        exit(1);
    }

    int32_t res = a / b;
    int32_t remainder = a % b;

    vm->registros[REG_AC] = remainder;
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void CMP(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
    int64_t res = (int64_t)a - b;

    bool v = ((a > 0 && b < 0 && (int32_t)res < 0) || (a < 0 && b > 0 && (int32_t)res >= 0));
    bool c = (uint32_t)a < (uint32_t)b;

    actualizarCC(vm, res, v, c);
}

void AND(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = a & b;
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void OR(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = a | b;
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void XOR(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = a ^ b;
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void SWAP(MaquinaVirtual *vm) {
    int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
    setValor(vm, vm->registros[REG_OP1], b);
    setValor(vm, vm->registros[REG_OP2], a);
    actualizarCC(vm, a, false, false);
}

void SHL(MaquinaVirtual *vm) {
    int32_t val = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t shift = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = val << shift;
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void SHR(MaquinaVirtual *vm) {
    uint32_t val = (uint32_t)getValorOP(vm, vm->registros[REG_OP1]);
    int32_t shift = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = (int32_t)(val >> shift);
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void SAR(MaquinaVirtual *vm) {
    int32_t val = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t shift = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = val >> shift;
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void LDL(MaquinaVirtual *vm) {
    int32_t dest = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t src = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = (dest & 0xFFFF0000) | (src & 0x0000FFFF);
    setValor(vm, vm->registros[REG_OP1], res);
}

void LDH(MaquinaVirtual *vm) {
    int32_t dest = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t src = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = (dest & 0x0000FFFF) | ((src & 0x0000FFFF) << 16);
    setValor(vm, vm->registros[REG_OP1], res);
}

void RND(MaquinaVirtual *vm) {
    int32_t max_val = getValorOP(vm, vm->registros[REG_OP2]);
    int32_t res = (max_val > 0) ? rand() % (max_val + 1) : 0;
    setValor(vm, vm->registros[REG_OP1], res);
}

void JMP(MaquinaVirtual *vm) {
    int32_t offset = getValorOP(vm, vm->registros[REG_OP1]);
    vm->registros[REG_IP] = (vm->registros[REG_CS] & 0xFFFF0000) | (offset & 0xFFFF);
}

void jump_if(MaquinaVirtual *vm, bool condition) {
    if (condition) {
        JMP(vm);
    }
}

void JP(MaquinaVirtual *vm) {
    uint32_t cc = vm->registros[REG_CC];
    jump_if(vm, !(cc & FLAG_N) && !(cc & FLAG_Z));
}

void JN(MaquinaVirtual *vm) {
    jump_if(vm, vm->registros[REG_CC] & FLAG_N);
}

void JZ(MaquinaVirtual *vm) {
    jump_if(vm, vm->registros[REG_CC] & FLAG_Z);
}

void JC(MaquinaVirtual *vm) {
    jump_if(vm, vm->registros[REG_CC] & FLAG_C);
}

void JV(MaquinaVirtual *vm) {
    jump_if(vm, vm->registros[REG_CC] & FLAG_V);
}

void JNP(MaquinaVirtual *vm) {
    uint32_t cc = vm->registros[REG_CC];
    jump_if(vm, (cc & FLAG_N) || (cc & FLAG_Z));
}

void JNN(MaquinaVirtual *vm) {
    jump_if(vm, !(vm->registros[REG_CC] & FLAG_N));
}

void JNZ(MaquinaVirtual *vm) {
    jump_if(vm, !(vm->registros[REG_CC] & FLAG_Z));
}

void NOT(MaquinaVirtual *vm) {
    int32_t val = getValorOP(vm, vm->registros[REG_OP1]);
    int32_t res = ~val;
    setValor(vm, vm->registros[REG_OP1], res);
    actualizarCC(vm, res, false, false);
}

void sys_read(MaquinaVirtual *vm) {
    uint32_t logical_base = vm->registros[REG_EDX];
    uint16_t cantidad = vm->registros[REG_ECX] & 0xFFFF;
    uint16_t tam_bytes = (vm->registros[REG_ECX] >> 16) & 0xFFFF;
    uint32_t modo = vm->registros[REG_EAX];

    for (uint16_t i = 0; i < cantidad; i++) {
        uint32_t logical_addr = logical_base + (i * tam_bytes);
        uint16_t phys_addr = traducirD(vm, logical_addr, tam_bytes);

        printf("[%04X]: ", phys_addr);

        int32_t valor = 0;
        if (modo & 0x02) { // Modo Carácter
            char c = getchar();
            if (c == '\n') c = getchar(); // Ignorar salto de línea previo
            valor = (int32_t)c;
        } else if (modo & 0x01) { // Modo Decimal
            scanf("%d", &valor);
        } else if (modo & 0x04) { // Modo Octal
            scanf("%o", &valor);
        } else if (modo & 0x08) { // Modo Hexadecimal
            scanf("%x", &valor);
        }

        // Escritura del valor leído en la memoria RAM
        if (tam_bytes == 1) {
            vm->memoria[phys_addr] = (uint8_t)valor;
        } else if (tam_bytes == 2) {
            vm->memoria[phys_addr] = valor & 0xFF;
            vm->memoria[phys_addr + 1] = (valor >> 8) & 0xFF;
        } else {
            write_mem32(vm, phys_addr, valor);
        }
    }
}

void sys_write(MaquinaVirtual *vm) {
    uint32_t logical_base = vm->registros[REG_EDX];
    uint16_t cantidad = vm->registros[REG_ECX] & 0xFFFF;
    uint16_t tam_bytes = (vm->registros[REG_ECX] >> 16) & 0xFFFF;
    uint32_t modo = vm->registros[REG_EAX];

    for (uint16_t i = 0; i < cantidad; i++) {
        uint32_t logical_addr = logical_base + (i * tam_bytes);
        uint16_t phys_addr = traducirD(vm, logical_addr, tam_bytes);

        // Lectura del valor según el tamaño configurado
        int32_t valor = 0;
        if (tam_bytes == 1) {
            valor = vm->memoria[phys_addr];
        } else if (tam_bytes == 2) {
            valor = vm->memoria[phys_addr] | (vm->memoria[phys_addr + 1] << 8);
        } else {
            valor = read_mem32(vm, phys_addr);
        }

        printf("[%04X]: ", phys_addr);

        // Puede imprimir en múltiples formatos simultáneamente según los bits de EAX
        if (modo & 0x10) { // Binario (Bit 4)
            printf("0b");
            for (int b = (tam_bytes * 8) - 1; b >= 0; b--) {
                printf("%d", (valor >> b) & 1);
            }
            printf(" ");
        }
        if (modo & 0x08) { // Hexadecimal (Bit 3)
            printf("0x%X ", valor);
        }
        if (modo & 0x04) { // Octal (Bit 2)
            printf("0o%o ", valor);
        }
        if (modo & 0x02) { // Caracteres (Bit 1)
            char c = (char)valor;
            printf("%c ", (c >= 32 && c <= 126) ? c : '.'); // Muestra '.' si no es imprimible
        }
        if (modo & 0x01) { // Decimal (Bit 0)
            printf("%d ", valor);
        }
        printf("\n");
    }
}

void SYS(MaquinaVirtual *vm) {
    int32_t sys_code = getValorOP(vm, vm->registros[REG_OP1]);

    if (sys_code == 1) {
        sys_read(vm);
    } else if (sys_code == 2) {
        sys_write(vm);
    } else {
        fprintf(stderr, "Error: Llamada al sistema inválida (%d)\n", sys_code);
        exit(1);
    }
}

void STOP(MaquinaVirtual *vm) {
    vm->registros[REG_IP] = -1;
}

void INVALID(MaquinaVirtual *vm) {
    fprintf(stderr, "Error: Instrucción inválida\n");
    exit(1);
}