
#include "maquinaV.h"


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

void SYS(MaquinaVirtual *vm) {
    int32_t sys_code = getValorOP(vm, vm->registros[REG_OP1]);
}

void STOP(MaquinaVirtual *vm) {
    vm->registros[REG_IP] = -1;
}

void INVALID(MaquinaVirtual *vm) {
    fprintf(stderr, "Error: Instrucción inválida\n");
    exit(1);
}