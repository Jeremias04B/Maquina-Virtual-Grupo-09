#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "maquinaV.h" 

void disassemble(const uint8_t *code, uint16_t codeSize);
void ciclo_ejecucion(MaquinaVirtual *vm);

// --- Prototipos de funciones de inicializacion
int validar_y_leer_cabecera(FILE *archivo, uint16_t *t_cs, uint16_t *t_ds, uint16_t *t_ks, uint16_t *t_es, uint16_t *t_ss);
uint32_t calcular_tamano_ps(int argc, char *argv[], int index_parametros, int *cantidad_params);
void configurar_descriptores_y_registros(MaquinaVirtual *vm, uint16_t t_cs, uint16_t t_ds, uint16_t t_ks, uint16_t t_es, uint16_t t_ss, uint32_t t_ps);
void cargar_parametros_ps(MaquinaVirtual *vm, int argc, char *argv[], int index_parametros, int cantidad_params);
void inicializar_pila(MaquinaVirtual *vm, uint16_t tamano_ss, int cantidad_params);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s filename.vmx [-d] [param1 param2 ...]\n", argv[0]);
        return 1;
    }

    int modo_disassembler = 0;
    int index_parametros = 2; 

    if (argc > 2 && strcmp(argv[2], "-d") == 0) {
        modo_disassembler = 1;
        index_parametros = 3;
    }

    FILE *archivo = fopen(argv[1], "rb");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // 1. Leer y validar cabecera
    uint16_t t_cs, t_ds, t_ks, t_es, t_ss;
    if (validar_y_leer_cabecera(archivo, &t_cs, &t_ds, &t_ks, &t_es, &t_ss) != 0) {
        fclose(archivo);
        return 1;
    }

    // 2. Calcular memoria de parámetros (PS)
    int cantidad_params;
    uint32_t t_ps = calcular_tamano_ps(argc, argv, index_parametros, &cantidad_params);

    // 3. Inicializar Máquina Virtual
    MaquinaVirtual vm;
    memset(&vm, 0, sizeof(MaquinaVirtual)); 

    // Cargar las instrucciones del archivo en el Code Segment
    if (fread(vm.memoria, sizeof(uint8_t), t_cs, archivo) != t_cs) {
        fprintf(stderr, "Error al leer las instrucciones del archivo\n");
        fclose(archivo);
        return 1;
    }
    fclose(archivo);

    // 4. Configurar segmentos, registros, parámetros y pila
    configurar_descriptores_y_registros(&vm, t_cs, t_ds, t_ks, t_es, t_ss, t_ps);
    cargar_parametros_ps(&vm, argc, argv, index_parametros, cantidad_params);
    inicializar_pila(&vm, t_ss, cantidad_params);

    // 5. Configurar el inicio de ejecución
    vm.registros[REG_IP] = vm.registros[REG_CS]; // TODO: Lógica para buscar 'main' en VMX2

    // 6. Derivar ejecución
    if (modo_disassembler) {
       disassemble(vm.memoria, t_cs);
    } else {
        ciclo_ejecucion(&vm);
    }

    return 0;
}


// =====================================================================
// DEFINICIÓN DE FUNCIONES AUXILIARES
// =====================================================================

int validar_y_leer_cabecera(FILE *archivo, uint16_t *t_cs, uint16_t *t_ds, uint16_t *t_ks, uint16_t *t_es, uint16_t *t_ss) {
    char identificador[6] = {0};
    if (fread(identificador, sizeof(char), 5, archivo) != 5 || strncmp(identificador, "VMX26", 5) != 0) {
        fprintf(stderr, "Error: Identificador invalido. Debe ser 'VMX26'\n");
        return 1;
    }

    uint8_t version = 0;
    if (fread(&version, sizeof(uint8_t), 1, archivo) != 1 || (version != 1 && version != 2)) {
        fprintf(stderr, "Error: Version no compatible (se requiere version 1 o 2)\n");
        return 1;
    }

    // Valores por defecto para versión 1[cite: 1]
    *t_ds = 1024;
    *t_ks = 1024;
    *t_es = 1024;
    *t_ss = 1024;

    if (version == 2) {
        uint8_t buffer_header[10];
        if (fread(buffer_header, sizeof(uint8_t), 10, archivo) != 10) {
            fprintf(stderr, "Error al leer los tamanos de segmentos en VMX2\n");
            return 1;
        }
        *t_cs = ((uint16_t)buffer_header[0] << 8) | buffer_header[1];
        *t_ds = ((uint16_t)buffer_header[2] << 8) | buffer_header[3];
        *t_ks = ((uint16_t)buffer_header[4] << 8) | buffer_header[5];
        *t_es = ((uint16_t)buffer_header[6] << 8) | buffer_header[7];
        *t_ss = ((uint16_t)buffer_header[8] << 8) | buffer_header[9];
    } else {
        uint8_t tam_bytes[2];
        if (fread(tam_bytes, sizeof(uint8_t), 2, archivo) != 2) {
            fprintf(stderr, "Error al leer el tamano del codigo\n");
            return 1;
        }
        *t_cs = ((uint16_t)tam_bytes[0] << 8) | tam_bytes[1];
    }
    return 0;
}

uint32_t calcular_tamano_ps(int argc, char *argv[], int index_parametros, int *cantidad_params) {
    uint32_t tamano_ps = 0;
    *cantidad_params = argc - index_parametros;
    
    if (*cantidad_params > 0) {
        tamano_ps += *cantidad_params * 4; // Arreglo de punteros
        for (int i = index_parametros; i < argc; i++) {
            tamano_ps += strlen(argv[i]) + 1; // Longitud del string + '\0'
        }
    } else {
        tamano_ps = 1024; // Valor por defecto si no hay parámetros
    }
    return tamano_ps;
}

void configurar_descriptores_y_registros(MaquinaVirtual *vm, uint16_t t_cs, uint16_t t_ds, uint16_t t_ks, uint16_t t_es, uint16_t t_ss, uint32_t t_ps) {
    uint32_t base_actual = 0;

    // Tabla de Segmentos[cite: 1]
    vm->TablaSegmentos[SEG_CS].base = base_actual;
    vm->TablaSegmentos[SEG_CS].size = t_cs;
    base_actual += t_cs;

    vm->TablaSegmentos[SEG_DS].base = base_actual;
    vm->TablaSegmentos[SEG_DS].size = t_ds;
    base_actual += t_ds;

    vm->TablaSegmentos[SEG_KS].base = base_actual;
    vm->TablaSegmentos[SEG_KS].size = t_ks;
    base_actual += t_ks;

    vm->TablaSegmentos[SEG_ES].base = base_actual;
    vm->TablaSegmentos[SEG_ES].size = t_es;
    base_actual += t_es;

    vm->TablaSegmentos[SEG_SS].base = base_actual;
    vm->TablaSegmentos[SEG_SS].size = t_ss;
    base_actual += t_ss;

    vm->TablaSegmentos[SEG_PS].base = base_actual;
    vm->TablaSegmentos[SEG_PS].size = t_ps;

    // Entradas 6 y 7 inactivas[cite: 1]
    for (int i = 6; i < 8; i++) {
        vm->TablaSegmentos[i].base = 0xFFFF;   
        vm->TablaSegmentos[i].size = 0xFFFF; 
    }

    // Registros
    vm->registros[REG_CS] = SEG_CS << 16;
    vm->registros[REG_DS] = SEG_DS << 16;
    vm->registros[REG_KS] = SEG_KS << 16;
    vm->registros[REG_ES] = SEG_ES << 16;
    vm->registros[REG_SS] = SEG_SS << 16;
    vm->registros[REG_PS] = SEG_PS << 16;
}

void cargar_parametros_ps(MaquinaVirtual *vm, int argc, char *argv[], int index_parametros, int cantidad_params) {
    uint32_t base_ps = vm->TablaSegmentos[SEG_PS].base;
    uint32_t offset_arreglo = 0; 
    uint32_t offset_strings = cantidad_params * 4; 

    if (cantidad_params > 0) {
        for (int i = index_parametros; i < argc; i++) {
            uint32_t dir_logica_str = (SEG_PS << 16) | offset_strings;
            
            // Puntero en Little-Endian[cite: 1]
            vm->memoria[base_ps + offset_arreglo + 0] = dir_logica_str & 0xFF;
            vm->memoria[base_ps + offset_arreglo + 1] = (dir_logica_str >> 8) & 0xFF;
            vm->memoria[base_ps + offset_arreglo + 2] = (dir_logica_str >> 16) & 0xFF;
            vm->memoria[base_ps + offset_arreglo + 3] = (dir_logica_str >> 24) & 0xFF;
            offset_arreglo += 4;
            
            // Copiar cadena con memcpy[cite: 1]
            size_t longitud = strlen(argv[i]) + 1;
            memcpy(&vm->memoria[base_ps + offset_strings], argv[i], longitud);
            offset_strings += longitud;
        }
    }
}

void inicializar_pila(MaquinaVirtual *vm, uint16_t tamano_ss, int cantidad_params) {
    uint32_t puntero_sp = tamano_ss; 

    // Macro local solo para no repetir código dentro de esta función
    #define PUSH_INICIAL(val) \
        puntero_sp -= 4; \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 0] = (val) & 0xFF; \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 1] = ((val) >> 8) & 0xFF; \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 2] = ((val) >> 16) & 0xFF; \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 3] = ((val) >> 24) & 0xFF;

    uint32_t puntero_argv = (cantidad_params > 0) ? (SEG_PS << 16) : 0xFFFFFFFF;
    
    // Dejar la pila precargada como exige el procesador[cite: 1]
    PUSH_INICIAL(puntero_argv);       
    PUSH_INICIAL(cantidad_params);    
    PUSH_INICIAL(0xFFFFFFFF);         

    #undef PUSH_INICIAL

    vm->registros[REG_SP] = puntero_sp;
    vm->registros[REG_BP] = puntero_sp; 
}