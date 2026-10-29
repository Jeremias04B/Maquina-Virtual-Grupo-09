/* ============================================================
 * Disassembler - Máquina Virtual (TP MV2 2026)
 * ============================================================
 * Muestra lo que hay en memoria de un programa ya cargado:
 *
 *   1) Las cadenas del Const Segment:
 *        [0000] XX XX XX XX | "abc..."
 *   2) Las instrucciones del Code Segment:
 *        [0000] XX XX XX XX | MNEM OP_A, OP_B
 *
 * Todas las direcciones son FÍSICAS (posición real en la RAM). El
 * entry point se marca con un '>' pegado delante del '['.
 *
 * ------------------------------------------------------------
 * LO QUE CAMBIÓ RESPECTO DE LA PARTE I
 *
 *  - Instrucciones nuevas: PUSH(0B) POP(0C) CALL(0D) con un operando,
 *    y RET(0E) sin operandos. Antes 0B-0E eran "instrucción inválida".
 *
 *  - Registros nuevos: 7=SP 8=BP 28=ES 29=SS 30=KS 31=PS.
 *
 *  - Operando de REGISTRO (1 byte):  X SS CCCCC
 *        CCCCC = código de registro
 *        SS    = sector: 00 EAX (4 bytes) | 01 AL | 10 AH | 11 AX
 *    El pseudónimo (AL, AH, AX) solo existe para EAX..EFX (códigos 10-15).
 *
 *  - Operando de MEMORIA (3 bytes):  OFFSET(16) MMM CCCCC
 *        MMM = tamaño del acceso: 001 byte (b) | 010 word (w) | 100 long (l)
 *    Se muestra como  b[REG+offset],  w[REG+offset]  o  l[REG+offset].
 *
 *  - Header v2 y Const Segment: ver disassemble_proceso().
 *
 * LO QUE SIGUE IGUAL: el primer byte decide todo.
 *   bit4 == 1 -> DOS operandos: bits7-6 tipo B, bits5-4 tipo A,
 *                código real = 0x10 | bits3-0
 *   bit4 == 0 -> UNO o CERO operandos: bits4-0 = código completo,
 *                bits7-6 = tipo del operando (si tiene uno)
 * El tamaño en bytes de cada operando es su código de tipo
 * (0 ninguno, 1 registro, 2 inmediato, 3 memoria) y en el binario
 * van primero los bytes del operando B y después los del A.
 *
 * NOTA sobre el traductor: si el operando A de una instrucción de dos
 * operandos es un INMEDIATO (ej. "MOV 5, EAX", que no tiene sentido),
 * el traductor lo codifica de forma ambigua y ningún disassembler lo
 * puede reconstruir. Con programas "normales" no pasa.
 * ============================================================ */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "disassembler.h"

/* ---------- Nombres de registros (32 registros, código 0-31) ---------- */
static const char *regNames[32] = {
    "IP", "OPC", "OP1", "OP2", "LAR", "MAR", "MBR", "SP",
    "BP", "R09", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
    "AC", "CC",  "R18", "R19", "R20", "R21", "R22", "R23",
    "R24", "R25", "CS", "DS",  "ES",  "SS",  "KS",  "PS"
};

/* ---------- Tablas de mnemónicos ---------- */
typedef struct {
    const char *mnemonic;
    int nOperandos;
} InstrDef;

/* instrucciones con bit4 = 1: índice = bits3-0 (= código de operación - 0x10) */
static const InstrDef tablaDosOperandos[16] = {
    {"MOV", 2}, {"ADD", 2}, {"SUB", 2}, {"MUL", 2},
    {"DIV", 2}, {"CMP", 2}, {"AND", 2}, {"OR",  2},
    {"XOR", 2}, {"SWAP",2}, {"SHL", 2}, {"SHR", 2},
    {"SAR", 2}, {"LDL", 2}, {"LDH", 2}, {"RND", 2}
};

/* instrucciones con bit4 = 0: índice = código de operación (0x00..0x0F) */
static const InstrDef tablaUnoCero[16] = {
    {"SYS", 1}, {"JMP", 1}, {"JP",  1}, {"JN",   1},
    {"JZ",  1}, {"JC",  1}, {"JV",  1}, {"JNP",  1},
    {"JNN", 1}, {"JNZ", 1}, {"NOT", 1}, {"PUSH", 1},
    {"POP", 1}, {"CALL",1}, {"RET", 0}, {"STOP", 0}
};

/* ---------- Operandos ---------- */
typedef enum { OP_NINGUNO = 0, OP_REGISTRO = 1, OP_INMEDIATO = 2, OP_MEMORIA = 3 } TipoOperando;

typedef struct {
    TipoOperando tipo;
    int16_t valor;   /* inmediato, u offset si es memoria */
    uint8_t reg;     /* registro, o registro base si es memoria */
    uint8_t sector;  /* solo registro: bits 6-5 del byte (0 E_X, 1 _L, 2 _H, 3 _X) */
    uint8_t modif;   /* solo memoria: bits 7-5 del último byte (1 b, 2 w, 4 l) */
} Operando;

/* Decodifica un operando desde 'buf' según su tipo.
 * Devuelve la cantidad de bytes consumidos (0,1,2,3). */
static int decodificar_operando(const uint8_t *buf, TipoOperando tipo, Operando *op) {
    op->tipo = tipo;
    switch (tipo) {
        case OP_NINGUNO:
            return 0;
        case OP_REGISTRO:
            op->reg    = buf[0] & 0x1F;
            op->sector = (buf[0] >> 5) & 0x3;
            return 1;
        case OP_INMEDIATO:
            op->valor = (int16_t)((buf[0] << 8) | buf[1]);
            return 2;
        case OP_MEMORIA:
            op->valor = (int16_t)((buf[0] << 8) | buf[1]);
            op->reg   = buf[2] & 0x1F;
            op->modif = (buf[2] >> 5) & 0x7;
            return 3;
    }
    return 0;
}

/* Nombre de un registro teniendo en cuenta el sector (solo EAX..EFX).
 * EAX(10) -> 'A', EBX(11) -> 'B', ECX -> 'C', EDX -> 'D', EEX -> 'E', EFX -> 'F' */
static void formatear_registro(uint8_t reg, uint8_t sector, char *out, size_t outsz) {
    if (reg >= 10 && reg <= 15 && sector != 0) {
        char letra = (char)('A' + (reg - 10));
        if (sector == 1)      snprintf(out, outsz, "%cL", letra);
        else if (sector == 2) snprintf(out, outsz, "%cH", letra);
        else                  snprintf(out, outsz, "%cX", letra);
    } else {
        snprintf(out, outsz, "%s", regNames[reg]);
    }
}

/* Formatea un operando ya decodificado como texto Assembler.
 * 'v2' = 0 imita la parte I (sin sectores de registro ni prefijos b/w/l). */
static void formatear_operando(const Operando *op, int v2, char *out, size_t outsz) {
    switch (op->tipo) {
        case OP_NINGUNO:
            out[0] = '\0';
            break;
        case OP_REGISTRO:
            formatear_registro(op->reg, v2 ? op->sector : 0, out, outsz);
            break;
        case OP_INMEDIATO:
            snprintf(out, outsz, "%d", op->valor);
            break;
        case OP_MEMORIA: {
            const char *prefijo = "";
            if (v2) {
                if (op->modif == 1)      prefijo = "b";
                else if (op->modif == 2) prefijo = "w";
                else if (op->modif == 4) prefijo = "l";
            }
            /* %+d imprime siempre el signo: +0, +5, -3 */
            snprintf(out, outsz, "%s[%s%+d]", prefijo, regNames[op->reg], op->valor);
            break;
        }
    }
}

/* ---------- Instrucción decodificada ---------- */
typedef struct {
    uint8_t  bytes[8];
    uint32_t len;          /* bytes totales de la instrucción */
    const char *mnemonic;
    int      nOperandos;   /* 0, 1 o 2 */
    Operando opA, opB;
} InstrDecodificada;

/* Decodifica UNA instrucción ubicada en code[off].
 * Devuelve 1 si es válida, 0 si es inválida o se sale del segmento. */
static int decodificar_instruccion(const uint8_t *code, uint32_t codeSize,
                                   uint32_t off, int v2, InstrDecodificada *out) {
    memset(out, 0, sizeof(*out));
    if (off >= codeSize) return 0;

    uint8_t b0 = code[off];
    uint32_t total;

    if ((b0 >> 4) & 1) {
        /* ---- dos operandos ---- */
        TipoOperando tipoB = (TipoOperando)((b0 >> 6) & 0x3);
        TipoOperando tipoA = (TipoOperando)((b0 >> 4) & 0x3);
        total = 1u + (uint32_t)tipoB + (uint32_t)tipoA;
        if (off + total > codeSize) return 0;

        uint32_t pos = 1;
        pos += (uint32_t)decodificar_operando(&code[off + pos], tipoB, &out->opB);
        decodificar_operando(&code[off + pos], tipoA, &out->opA);

        out->mnemonic   = tablaDosOperandos[b0 & 0x0F].mnemonic;
        out->nOperandos = 2;
    } else {
        /* ---- uno o cero operandos: bits4-0 son el código completo (0x00..0x0F) ---- */
        int op5 = b0 & 0x1F;
        if (!v2 && op5 >= 0x0B && op5 <= 0x0E) return 0;   /* en la parte I eran inválidos */

        const InstrDef *def = &tablaUnoCero[op5];
        TipoOperando tipoA = (def->nOperandos == 1) ? (TipoOperando)((b0 >> 6) & 0x3)
                                                    : OP_NINGUNO;
        total = 1u + (uint32_t)tipoA;
        if (off + total > codeSize) return 0;

        decodificar_operando(&code[off + 1], tipoA, &out->opA);
        out->mnemonic   = def->mnemonic;
        out->nOperandos = def->nOperandos;
    }

    for (uint32_t i = 0; i < total; i++) out->bytes[i] = code[off + i];
    out->len = total;
    return 1;
}

/* Bytes en hexadecimal separados por espacio. Si son más de 7 muestra
 * los primeros 6 y ".." para indicar que sigue. */
static void bytes_a_hex(const uint8_t *b, uint32_t n, char *out, size_t outsz) {
    size_t p = 0;
    uint32_t mostrar = (n > 7) ? 6 : n;
    out[0] = '\0';
    for (uint32_t i = 0; i < mostrar && p < outsz; i++)
        p += (size_t)snprintf(out + p, outsz - p, i ? " %02X" : "%02X", b[i]);
    if (n > 7 && p < outsz)
        snprintf(out + p, outsz - p, " ..");
}

/* Recorre el Const Segment: cada cadena llega hasta su '\0' (inclusive). */
static void mostrar_constantes(const uint8_t *seg, uint32_t tam, uint32_t baseFisica) {
    uint32_t i = 0;
    while (i < tam) {
        uint32_t ini = i;
        while (i < tam && seg[i] != 0) i++;
        int terminada = (i < tam);        /* encontró el '\0' */
        if (terminada) i++;               /* el '\0' se muestra en el hexa */
        uint32_t len = i - ini;

        char hexbuf[48];
        bytes_a_hex(&seg[ini], len, hexbuf, sizeof(hexbuf));
        printf("[%04X] %s | \"", (unsigned)(baseFisica + ini), hexbuf);

        uint32_t nchars = terminada ? len - 1 : len;   /* el '\0' no se imprime como texto */
        for (uint32_t j = 0; j < nchars; j++) {
            uint8_t c = seg[ini + j];
            putchar((c >= 32 && c <= 126) ? c : '.');
        }
        printf("\"\n");
    }
}

void disassemble_proceso(const uint8_t *memoria, uint8_t version,
                         uint32_t baseConst, uint32_t tamConst,
                         uint32_t baseCode,  uint32_t tamCode,
                         uint32_t entryPoint) {
    int v2 = (version >= 2);

    if (v2 && tamConst > 0)
        mostrar_constantes(memoria + baseConst, tamConst, baseConst);

    const uint8_t *code = memoria + baseCode;
    uint32_t off = 0;

    while (off < tamCode) {
        InstrDecodificada ins;
        const char *marca = (v2 && off == entryPoint) ? ">" : "";
        unsigned dir = (unsigned)(baseCode + off);

        if (!decodificar_instruccion(code, tamCode, off, v2, &ins)) {
            printf("%s[%04X] %02X | ERROR: instruccion invalida\n", marca, dir, code[off]);
            break;
        }

        char hexbuf[48];
        bytes_a_hex(ins.bytes, ins.len, hexbuf, sizeof(hexbuf));

        char opAstr[48] = {0}, opBstr[48] = {0};
        if (ins.nOperandos >= 1) formatear_operando(&ins.opA, v2, opAstr, sizeof(opAstr));
        if (ins.nOperandos >= 2) formatear_operando(&ins.opB, v2, opBstr, sizeof(opBstr));

        printf("%s[%04X] %s | ", marca, dir, hexbuf);
        if (ins.nOperandos == 0)
            printf("%s\n", ins.mnemonic);
        else if (ins.nOperandos == 1)
            printf("%s %s\n", ins.mnemonic, opAstr);
        else
            printf("%s %s, %s\n", ins.mnemonic, opAstr, opBstr);

        off += ins.len;
    }
}

/* Parte I: el código está en la dirección física 0 y no hay constantes. */
void disassemble(const uint8_t *code, uint16_t codeSize) {
    disassemble_proceso(code, 1, 0, 0, 0, codeSize, 0);
}