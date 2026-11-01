#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>

void disassemble_proceso(const uint8_t *memoria, uint8_t version,
                         uint32_t baseConst, uint32_t tamConst,
                         uint32_t baseCode,  uint32_t tamCode,
                         uint32_t entryPoint);


void disassemble(const uint8_t *code, uint16_t codeSize);

#endif