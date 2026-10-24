#include "maquinaV.h"
#include "funciones.h"
#include "instrucciones.h"
PInstrucciones vectorIns[32] = {
    [0x00] = SYS,     [0x01] = JMP,     [0x02] = JP,      [0x03] = JN,
    [0x04] = JZ,      [0x05] = JC,      [0x06] = JV,      [0x07] = JNP,
    [0x08] = JNN,     [0x09] = JNZ,     [0x0A] = NOT,     [0x0B] = PUSH,
    [0x0C] = POP,     [0x0D] = CALL,    [0x0E] = RET,     [0x0F] = STOP,
    [0x10] = MOV,     [0x11] = ADD,     [0x12] = SUB,     [0x13] = MUL,
    [0x14] = DIV,     [0x15] = CMP,     [0x16] = AND,     [0x17] = OR,
    [0x18] = XOR,     [0x19] = SWAP,    [0x1A] = SHL,     [0x1B] = SHR,
    [0x1C] = SAR,     [0x1D] = LDL,     [0x1E] = LDH,     [0x1F] = RND
};
    void MOV(MaquinaVirtual *vm) {
        int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
        setValor(vm, vm->registros[REG_OP1], b);
        actualizarCC(vm, b, false, false); // MOV no genera acarreo ni desbordamiento, solo copia
    }
 
    void ADD(MaquinaVirtual *vm) {
        int32_t a = getValorOP(vm, vm->registros[REG_OP1]);
        int32_t b = getValorOP(vm, vm->registros[REG_OP2]);
        int64_t res = (int64_t)a + b;
 
        // desbordamiento con signo: mismo signo en A y B, pero el resultado cambia de signo
        bool v = ((a > 0 && b > 0 && (int32_t)res < 0) || (a < 0 && b < 0 && (int32_t)res >= 0));
        // acarreo: la suma sin signo de 32 bits no entra en 32 bits (se necesitaría un bit 33)
        bool c = (((uint64_t)(uint32_t)a + (uint32_t)b) > 0xFFFFFFFFULL);
 
        setValor(vm, vm->registros[REG_OP1], (int32_t)res);
        actualizarCC(vm, res, v, c);
    }

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

void PUSH(MaquinaVirtual *vm){
    vm->registros[REG_SP] -=4;
    if (vm->registros[REG_SP] < vm->registros[REG_SS]){
        fprintf(stderr, "Error: STACK OVERFLOW\n");
        STOP(vm);
        exit(1);
    }
    else{
        int32_t val = getValorOP(vm, vm->registros[REG_OP1]);
        int32_t dir= traducirD(vm,vm->registros[REG_SP],4);
        vm->memoria[dir]     = (val >> 24) & 0xFF;
        vm->memoria[dir + 1] = (val >> 16) & 0xFF;
        vm->memoria[dir + 2] = (val >> 8)  & 0xFF;
        vm->memoria[dir + 3] =  val        & 0xFF; 
    }
}

void POP(MaquinaVirtual *vm) {
    // 1. Validar STACK UNDERFLOW
    // Si SP es mayor o igual a (SS + tamaño del segmento de pila), la pila está vacía/desbordada
    uint16_t ss_segment = (vm->registros[REG_SS] >> 16) & 0xFFFF;
    uint16_t stack_size = vm->TablaSegmentos[ss_segment].size;
    
    // SP inicializado apunta a (SS + stack_size). Si al hacer POP supera o iguala dicho límite:
    if ((vm->registros[REG_SP] & 0xFFFF) + 4 > stack_size) {
        fprintf(stderr, "Error: STACK UNDERFLOW\n");
        STOP(vm);
        exit(1);
    }

    // 2. Traducir la dirección lógica del tope de la pila (SP) a dirección física
    uint16_t dir = traducirD(vm, vm->registros[REG_SP], 4);

    // 3. Extraer 4 bytes en orden BIG-ENDIAN (reconstruir el valor de 32 bits)
    // El byte en la dirección apuntada por SP es el más significativo (MSB)
    int32_t val = 0;
    val |= ((int32_t)vm->memoria[dir])     << 24;
    val |= ((int32_t)vm->memoria[dir + 1]) << 16;
    val |= ((int32_t)vm->memoria[dir + 2]) << 8;
    val |= ((int32_t)vm->memoria[dir + 3]);

    setValor(vm, vm->registros[REG_OP1], val);

    vm->registros[REG_SP] += 4;
}

void CALL(MaquinaVirtual *vm) {
    int32_t offset_destino = getValorOP(vm, vm->registros[REG_OP1]);

    vm->registros[REG_SP] -= 4;

    if (vm->registros[REG_SP] < vm->registros[REG_SS]) {
        fprintf(stderr, "Error: STACK OVERFLOW\n");
        STOP(vm);
        exit(1);
    }

    uint16_t dir = traducirD(vm, vm->registros[REG_SP], 4);
    int32_t ret_ip = vm->registros[REG_IP];

    vm->memoria[dir]     = (ret_ip >> 24) & 0xFF;
    vm->memoria[dir + 1] = (ret_ip >> 16) & 0xFF;
    vm->memoria[dir + 2] = (ret_ip >> 8)  & 0xFF;
    vm->memoria[dir + 3] =  ret_ip        & 0xFF;

    vm->registros[REG_IP] = (vm->registros[REG_CS] & 0xFFFF0000) | (offset_destino & 0x0000FFFF);
}

void RET(MaquinaVirtual *vm) {
    uint16_t ss_segment = (vm->registros[REG_SS] >> 16) & 0xFFFF;
    uint16_t stack_size = vm->TablaSegmentos[ss_segment].size;

    if ((vm->registros[REG_SP] & 0xFFFF) + 4 > stack_size) {
        fprintf(stderr, "Error: STACK UNDERFLOW\n");
        STOP(vm);
        exit(1);
    }

    uint16_t dir = translate_address(vm, vm->registros[REG_SP], 4);

    int32_t ret_ip = 0;
    ret_ip |= ((int32_t)vm-><memoria>[dir])     << 24;
    ret_ip |= ((int32_t)vm->memoria[dir + 1]) << 16;
    ret_ip |= ((int32_t)vm->memoria[dir + 2]) << 8;
    ret_ip |= ((int32_t)vm->memoria[dir + 3]);

    vm->registros[REG_IP] = ret_ip;
    vm->registros[REG_SP] += 4;
}

void sys_read(MaquinaVirtual *vm) {
    uint32_t logical_base = vm->registros[REG_EDX];
    uint16_t cantidad = vm->registros[REG_ECX] & 0xFFFF;
    uint16_t tam_bytes = (vm->registros[REG_ECX] >> 16) & 0xFFFF;
    uint32_t modo = vm->registros[REG_EAX];

    for (uint16_t i = 0; i < cantidad; i++) {
        uint32_t logical_addr = logical_base + (i * tam_bytes);
        uint16_t dir = traducirD(vm, logical_addr, tam_bytes);

        printf("[%04X]: ", dir);

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
            vm->memoria[dir] = (uint8_t)valor;
        } else if (tam_bytes == 2) {
            vm->memoria[dir] = valor & 0xFF;
            vm->memoria[dir + 1] = (valor >> 8) & 0xFF;
        } else {
            write_mem32(vm, dir, valor);
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
        uint16_t dir = traducirD(vm, logical_addr, tam_bytes);

        // Lectura del valor según el tamaño configurado
        int32_t valor = 0;
        if (tam_bytes == 1) {
            valor = vm->memoria[dir];
        } else if (tam_bytes == 2) {
            valor = vm->memoria[dir] | (vm->memoria[dir + 1] << 8);
        } else {
            valor = read_mem32(vm, dir);
        }

        printf("[%04X]: ", dir);

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
    } 
}

    void STOP(MaquinaVirtual *vm) {
        vm->registros[REG_IP] = -1;
    }

void INVALID(MaquinaVirtual *vm) {
    fprintf(stderr, "Error: Instrucción inválida\n");
    exit(1);
}