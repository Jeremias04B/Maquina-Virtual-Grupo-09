#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>

/* Muestra el Const Segment (cadenas) y el Code Segment (instrucciones)
 * de un programa YA CARGADO en memoria, con el formato del enunciado.
 *
 *   memoria     : RAM completa de la VM (el array donde se cargaron los segmentos)
 *   version     : 1 o 2 (byte 5 del header del .vmx)
 *   baseConst   : dirección FÍSICA donde empieza el Const Segment
 *   tamConst    : tamaño del Const Segment en bytes (0 si no existe)
 *   baseCode    : dirección FÍSICA donde empieza el Code Segment
 *   tamCode     : tamaño del Code Segment en bytes
 *   entryPoint  : offset del entry point dentro del Code Segment
 *                 (bytes 16-17 del header v2; se ignora en versión 1)
 *
 * Version 1: sin cadenas, sin '>' y sin prefijos de memoria (formato de la parte I).
 * Version 2: con cadenas, '>' en el entry point, pseudónimos de registros y b/w/l. */
void disassemble_proceso(const uint8_t *memoria, uint8_t version,
                         uint32_t baseConst, uint32_t tamConst,
                         uint32_t baseCode,  uint32_t tamCode,
                         uint32_t entryPoint);

/* Compatibilidad con la parte I: 'code' apunta al comienzo del código
 * (que en la parte I siempre está en la dirección física 0). */
void disassemble(const uint8_t *code, uint16_t codeSize);

#endif