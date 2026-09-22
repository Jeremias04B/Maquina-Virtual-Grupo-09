#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H
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

extern PInstrucciones vectorIns[32]; 
#endif