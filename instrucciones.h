#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "maquinaV.h"

void jump_if(MaquinaVirtual *vm, bool condition);

// CABECERAS INTRUCCIONES
void MOV(MaquinaVirtual *vm);
void ADD(MaquinaVirtual *vm);
void SUB(MaquinaVirtual *vm);
void MUL(MaquinaVirtual *vm);
void DIV(MaquinaVirtual *vm);
void CMP(MaquinaVirtual *vm);
void AND(MaquinaVirtual *vm);
void OR(MaquinaVirtual *vm);
void XOR(MaquinaVirtual *vm);
void SWAP(MaquinaVirtual *vm);
void SHL(MaquinaVirtual *vm);
void SHR(MaquinaVirtual *vm);
void SAR(MaquinaVirtual *vm);
void LDL(MaquinaVirtual *vm);
void LDH(MaquinaVirtual *vm);
void RND(MaquinaVirtual *vm);
void JMP(MaquinaVirtual *vm);
void JP(MaquinaVirtual *vm);
void JN(MaquinaVirtual *vm);
void JZ(MaquinaVirtual *vm);
void JC(MaquinaVirtual *vm);
void JV(MaquinaVirtual *vm);
void JNP(MaquinaVirtual *vm);
void JNN(MaquinaVirtual *vm);
void JNZ(MaquinaVirtual *vm);
void NOT(MaquinaVirtual *vm);
void SYS(MaquinaVirtual *vm);
void STOP(MaquinaVirtual *vm);
void INVALID(MaquinaVirtual *vm);

typedef void (*PInstrucciones)(MaquinaVirtual *mv);
PInstrucciones vectorIns[32] = {
    [0x00] = SYS,     [0x01] = JMP,     [0x02] = JP,      [0x03] = JN,
    [0x04] = JZ,      [0x05] = JC,      [0x06] = JV,      [0x07] = JNP,
    [0x08] = JNN,     [0x09] = JNZ,     [0x0A] = NOT,     [0x0B] = INVALID,
    [0x0C] = INVALID, [0x0D] = INVALID, [0x0E] = INVALID, [0x0F] = STOP,
    [0x10] = MOV,     [0x11] = ADD,     [0x12] = SUB,     [0x13] = MUL,
    [0x14] = DIV,     [0x15] = CMP,     [0x16] = AND,     [0x17] = OR,
    [0x18] = XOR,     [0x19] = SWAP,    [0x1A] = SHL,     [0x1B] = SHR,
    [0x1C] = SAR,     [0x1D] = LDL,     [0x1E] = LDH,     [0x1F] = RND
};