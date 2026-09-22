#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "maquinaV.h" 

void disassemble(const uint8_t *code, uint16_t codeSize);
void ciclo_ejecucion(MaquinaVirtual *vm);
int main(int argc, char *argv[]) {
    //validar argumentos de invocacion: vmx filename.vmx [-d]
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s filename.vmx [-d]\n", argv[0]);
        return 1;
    }

    int modo_disassembler = 0;
    if (argc == 3) {
        if (strcmp(argv[2], "-d") == 0) {
            modo_disassembler = 1;
        } else {
            fprintf(stderr, "Error: argumento desconocido '%s'. Se esperaba [-d]\n", argv[2]);
            return 1;
        }
    }

    // abrir el archivo binario
    FILE *archivo = fopen(argv[1], "rb");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // leer y validar la cabecera (8 bytes)
    char identificador[6] = {0};
    if (fread(identificador, sizeof(char), 5, archivo) != 5 || strncmp(identificador, "VMX26", 5) != 0) {
        fprintf(stderr, "Error: Identificador invalido. Debe ser 'VMX26'\n");
        fclose(archivo);
        return 1;
    }

    uint8_t version = 0;
    if (fread(&version, sizeof(uint8_t), 1, archivo) != 1 || version != 1) {
        fprintf(stderr, "Error: Version no compatible (se requiere version 1)\n");
        fclose(archivo);
        return 1;
    }

    // Leer los 2 bytes del tamaño del codigo
    uint16_t tamano_codigo = 0;
    if (fread(&tamano_codigo, sizeof(uint16_t), 1, archivo) != 1) {
        fprintf(stderr, "Error al leer el tamano del codigo\n");
        fclose(archivo);
        return 1;
    }

    if (tamano_codigo > 16384) { // Verifica que no exceda los 16 KiB
        fprintf(stderr, "Error: El size del codigo excede la memoria disponible\n");
        fclose(archivo);
        return 1;
    }

    // instanciar e inicializar la maquina virtual a cero
    MaquinaVirtual vm;
    memset(&vm, 0, sizeof(MaquinaVirtual)); 

    // cargar el codigo binario en la memoria RAM (a partir de la posicion 0)
    if (fread(vm.memoria, sizeof(uint8_t), tamano_codigo, archivo) != tamano_codigo) {
        fprintf(stderr, "Error al leer las instrucciones del archivo\n");
        fclose(archivo);
        return 1;
    }
    fclose(archivo);

    // configurar la tabla de descriptores de segmentos (8 entradas)
    // entrada 0 (CS): base = 0, tamaño = tamaño del codigo
    vm.TablaSegmentos[0].base = 0x0000;
    vm.TablaSegmentos[0].size = tamano_codigo;

    // entrada 1 (DS): base = tamaño del codigo, Tamaño = resto de la memoria
    vm.TablaSegmentos[1].base = tamano_codigo;
    vm.TablaSegmentos[1].size = 16384 - tamano_codigo;

    // entradas 2 a 7: Sin uso (-1 o 0xFFFFFFFF)
    for (int i = 2; i < 8; i++) {
        vm.TablaSegmentos[i].base = 0xFFFF;   
        vm.TablaSegmentos[i].size = 0xFFFF; 
    }

    //inicializar los registros obligatorios de arranque
    // los codigos de registro estan en la tabla: IP es 0, CS es 26, DS es 27
    vm.registros[REG_CS] = 0x00000000; // CS: segmento 0, offset 0
    vm.registros[REG_DS] = 0x00010000; // DS: segmento 1, offset 0
    vm.registros[REG_IP]  = vm.registros[REG_CS]; // IP apunta a la primera instruccion del codigo

    //segun el modo de ejecucion
    if (modo_disassembler) {
       disassemble(vm.memoria, tamano_codigo);
    } else {
        ciclo_ejecucion(&vm);
    }

    return 0;
}
