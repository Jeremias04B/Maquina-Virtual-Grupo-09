#ifndef FUNCIONESL_H
#define FUNCIONESL_H
 
#include <stdint.h>
#include "maquinaV.h"
 
uint16_t traducirD(MaquinaVirtual *vm, uint32_t dirLogica, uint16_t access_bytes);
int32_t  read_mem32(MaquinaVirtual *vm, uint16_t dirFisica);
void     write_mem32(MaquinaVirtual *vm, uint16_t dirFisica, int32_t val);
int32_t  getValorOP(MaquinaVirtual *vm, uint32_t reg_op);
void     setValor(MaquinaVirtual *vm, uint32_t reg_op, int32_t val);
void     actualizarCC(MaquinaVirtual *vm, int64_t result, bool check_overflow, bool carry);
char     configmodo(int16_t v);
 
#endif /* FUNCIONESL_H */