/* ============================================================
 * Disassembler - Máquina Virtual (TP MV1 2026)
 * ============================================================
 * Lee un archivo .vmx (cabecera VMX26 + código máquina) y muestra
 * el código Assembler correspondiente, formato:
 *
 *   [0000] XX XX XX XX | MNEM OP_A, OP_B
 *
 * ------------------------------------------------------------
 * CÓMO SE DECODIFICA CADA INSTRUCCIÓN (deducido leyendo el PDF
 * y verificando con el traductor 'vmt' provisto):
 *
 * El primer byte de toda instrucción es:
 *
 *   bit4 == 1  -> instrucción de DOS operandos
 *       bits7-6 = tipo operando B (2 bits)
 *       bits5-4 = tipo operando A (2 bits)
 *       bits3-0 = (código de operación - 0x10)
 *       código real = 0x10 | bits3-0        (rango 0x10-0x1F)
 *
 *   bit4 == 0  -> instrucción de UNO o CERO operandos
 *       bits4-0 = código de operación completo
 *       - si código == 0x0F               -> STOP (0 operandos)
 *       - si código está entre 0x00-0x0A  -> 1 operando
 *             bits7-6 = tipo del operando
 *             bit5    = relleno (0)
 *       - cualquier otro valor            -> instrucción inválida
 *
 * El tamaño en bytes de cada operando coincide con su código de
 * tipo (0=ninguno, 1=registro, 2=inmediato, 3=memoria), tal como
 * indica el enunciado. Los operandos se guardan en el ORDEN
 * INVERSO al de la instrucción Assembler, es decir: primero los
 * bytes del operando B, luego los del operando A. Al mostrar la
 * instrucción en Assembler, sin embargo, se listan como "A, B"
 * (el orden natural).
 *
 * NOTA sobre el traductor 'vmt' provisto: al probarlo detecté que
 * si el operando A (el que normalmente sería destino, ej. en MOV
 * o ADD) es un INMEDIATO en lugar de registro/memoria, el
 * traductor lo codifica con el mismo código de tipo que usa para
 * "memoria" pero grabando solo 2 bytes en vez de 3 -> genera un
 * .vmx ambiguo/mal formado que ningún disassembler puede
 * reconstruir de manera confiable (es un caso sin sentido de todos
 * modos: el destino de un MOV/ADD no puede ser una constante). Del
 * mismo modo, "RND" escrito con un solo operando también produce
 * una codificación irregular (marca dos operandos tipo registro
 * pero graba un solo byte). Mientras el código fuente respete el
 * uso normal de cada instrucción (tal como en los ejemplos del
 * enunciado) esto no afecta al disassembler.
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <maquinaV.h>   
/* ---------- Nombres de registros (32 registros, código 0-31) ---------- */
static const char *regNames[32] = {
    "IP", "OPC", "OP1", "OP2", "LAR", "MAR", "MBR", "R07",
    "R08", "R09", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
    "AC",  "CC",  "R18", "R19", "R20", "R21", "R22", "R23",
    "R24", "R25", "CS",  "DS",  "R28", "R29", "R30", "R31"
};

/* ---------- Tablas de mnemónicos ---------- */
typedef struct {
    const char *mnemonic;
    int nOperandos;
} InstrDef;

/* índice = código de operación - 0x10 (0x10..0x1F) */
static const InstrDef tablaDosOperandos[16] = {
    {"MOV", 2}, {"ADD", 2}, {"SUB", 2}, {"MUL", 2},
    {"DIV", 2}, {"CMP", 2}, {"AND", 2}, {"OR",  2},
    {"XOR", 2}, {"SWAP",2}, {"SHL", 2}, {"SHR", 2},
    {"SAR", 2}, {"LDL", 2}, {"LDH", 2}, {"RND", 2}
};

/* índice = código de operación (0x00..0x0A) */
static const InstrDef tablaUnOperando[11] = {
    {"SYS", 1}, {"JMP", 1}, {"JP", 1}, {"JN", 1},
    {"JZ",  1}, {"JC",  1}, {"JV", 1}, {"JNP",1},
    {"JNN", 1}, {"JNZ", 1}, {"NOT",1}
};

/* ---------- Tipos y valores de operando ---------- */
typedef enum { OP_NINGUNO = 0, OP_REGISTRO = 1, OP_INMEDIATO = 2, OP_MEMORIA = 3 } TipoOperando;

typedef struct {
    TipoOperando tipo;
    int16_t valor;   /* inmediato, u offset si es memoria */
    uint8_t reg;     /* registro, o registro base si es memoria */
} Operando;

/* Decodifica un operando desde 'buf' según su tipo.
 * Devuelve la cantidad de bytes consumidos (0,1,2,3). */
static int decodificar_operando(const uint8_t *buf, TipoOperando tipo, Operando *op) {
    op->tipo = tipo;
    switch (tipo) {
        case OP_NINGUNO:
            return 0;
        case OP_REGISTRO:
            op->reg = buf[0] & 0x1F;
            return 1;
        case OP_INMEDIATO:
            op->valor = (int16_t)((buf[0] << 8) | buf[1]);
            return 2;
        case OP_MEMORIA:
            op->valor = (int16_t)((buf[0] << 8) | buf[1]);
            op->reg   = buf[2] & 0x1F;
            return 3;
    }
    return 0;
}

/* Formatea un operando ya decodificado como texto Assembler. */
static void formatear_operando(const Operando *op, char *out, size_t outsz) {
    switch (op->tipo) {
        case OP_NINGUNO:
            out[0] = '\0';
            break;
        case OP_REGISTRO:
            snprintf(out, outsz, "%s", regNames[op->reg]);
            break;
        case OP_INMEDIATO:
            snprintf(out, outsz, "%d", op->valor);
            break;
        case OP_MEMORIA:
            if (op->valor == 0)
                snprintf(out, outsz, "[%s]", regNames[op->reg]);
            else if (op->valor > 0)
                snprintf(out, outsz, "[%s+%d]", regNames[op->reg], op->valor);
            else
                snprintf(out, outsz, "[%s%d]", regNames[op->reg], op->valor); /* valor ya trae el '-' */
            break;
    }
}

/* ---------- Instrucción decodificada ---------- */
typedef struct {
    uint16_t addr;
    uint8_t  bytes[8];
    int      len;          /* bytes totales de la instrucción */
    const char *mnemonic;
    int      nOperandos;   /* 0, 1 o 2 */
    Operando opA, opB;
    int      valida;
} InstrDecodificada;

/* Decodifica UNA instrucción ubicada en code[addr].
 * Devuelve 1 si es válida, 0 si es inválida o excede el segmento. */
static int decodificar_instruccion(const uint8_t *code, uint16_t codeSize, uint16_t addr, InstrDecodificada *out) {
    memset(out, 0, sizeof(*out));
    out->addr = addr;

    if (addr >= codeSize) return 0;

    uint8_t b0 = code[addr];
    int bit4 = (b0 >> 4) & 1;

    if (bit4) {
        /* ---- dos operandos ---- */
        int opcode = 0x10 | (b0 & 0x0F);
        TipoOperando tipoB = (TipoOperando)((b0 >> 6) & 0x3);
        TipoOperando tipoA = (TipoOperando)((b0 >> 4) & 0x3);
        int sizeB = (int)tipoB;
        int sizeA = (int)tipoA;
        int total = 1 + sizeB + sizeA;

        if (total > 8 || addr + total > codeSize) return 0;

        int pos = 1;
        pos += decodificar_operando(&code[addr + pos], tipoB, &out->opB);
        pos += decodificar_operando(&code[addr + pos], tipoA, &out->opA);
        (void)pos;

        for (int i = 0; i < total; i++) out->bytes[i] = code[addr + i];
        out->len = total;
        out->mnemonic = tablaDosOperandos[opcode - 0x10].mnemonic;
        out->nOperandos = 2;
        out->valida = 1;
    } else {
        int op5 = b0 & 0x1F;
        if (op5 == 0x0F) {
            /* ---- STOP: cero operandos ---- */
            out->bytes[0] = b0;
            out->len = 1;
            out->mnemonic = "STOP";
            out->nOperandos = 0;
            out->valida = 1;
        } else if (op5 <= 0x0A) {
            /* ---- un operando ---- */
            TipoOperando tipoA = (TipoOperando)((b0 >> 6) & 0x3);
            int sizeA = (int)tipoA;
            int total = 1 + sizeA;

            if (total > 8 || addr + total > codeSize) return 0;

            decodificar_operando(&code[addr + 1], tipoA, &out->opA);
            for (int i = 0; i < total; i++) out->bytes[i] = code[addr + i];
            out->len = total;
            out->mnemonic = tablaUnOperando[op5].mnemonic;
            out->nOperandos = 1;
            out->valida = 1;
        } else {
            /* código de operación no existente */
            return 0;
        }
    }
    return 1;
}

/* Recorre TODO el segmento de código e imprime el disassembler. */
void disassemble(const uint8_t *code, uint16_t codeSize) {
    uint16_t addr = 0;

    while (addr < codeSize) {
        InstrDecodificada ins;

        if (!decodificar_instruccion(code, codeSize, addr, &ins)) {
            printf("[%04X] %02X                      | ERROR: instruccion invalida\n",
                   addr, code[addr]);
            break;
        }

        /* dirección + bytes en hex */
        char hexbuf[32] = {0};
        int p = 0;
        for (int i = 0; i < ins.len; i++)
            p += snprintf(hexbuf + p, sizeof(hexbuf) - p, "%02X ", ins.bytes[i]);

        printf("[%04X] %-20s| ", ins.addr, hexbuf);

        char opAstr[32] = {0}, opBstr[32] = {0};
        if (ins.nOperandos >= 1) formatear_operando(&ins.opA, opAstr, sizeof(opAstr));
        if (ins.nOperandos >= 2) formatear_operando(&ins.opB, opBstr, sizeof(opBstr));

        if (ins.nOperandos == 0)
            printf("%s\n", ins.mnemonic);
        else if (ins.nOperandos == 1)
            printf("%s %s\n", ins.mnemonic, opAstr);
        else
            printf("%s %s, %s\n", ins.mnemonic, opAstr, opBstr);

        addr += ins.len;
    }
}

