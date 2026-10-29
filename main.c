#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "maquinaV.h"

// Prototipos de funciones auxiliares
int cargar_imagen_vmi(MaquinaVirtual *vm, const char *filename);
uint32_t obtener_tamano_memoria(int argc, char *argv[]);
int validar_y_leer_cabecera(FILE *archivo, uint8_t *version, uint16_t *t_cs, uint16_t *t_ds, uint16_t *t_ks, uint16_t *t_es, uint16_t *t_ss, uint16_t *entry_point);
uint32_t calcular_tamano_ps(int argc, char *argv[], int index_filename, int index_parametros, int *cantidad_params);
void configurar_descriptores_y_registros(MaquinaVirtual *vm, uint16_t t_cs, uint16_t t_ds, uint16_t t_ks, uint16_t t_es, uint16_t t_ss, uint32_t t_ps);
void cargar_parametros_ps(MaquinaVirtual *vm, int argc, char *argv[], int index_filename, int index_parametros, int cantidad_params);
void inicializar_pila(MaquinaVirtual *vm, uint16_t tamano_ss, int cantidad_params);
void disassemble_proceso(const uint8_t *memoria, uint8_t version, uint32_t baseConst, uint32_t tamConst, uint32_t baseCode, uint32_t tamCode, uint32_t entryPoint);
void ciclo_ejecucion(MaquinaVirtual *vm);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s filename.vmx|filename.vmi [-d] [-p param1 ...] [m=M]\n", argv);
        return 1;
    }

    int modo_disassembler = 0;
    int index_filename = -1;
    int index_parametros = -1;

    // 1. Recorrer los argumentos de la consola para identificar banderas y archivos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            modo_disassembler = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            index_parametros = i + 1; // Los parámetros del programa arrancan después de -p
            break; // Todo lo que sigue a -p son parámetros del programa guest
        } else if ((strstr(argv[i], ".vmx") != NULL || strstr(argv[i], ".vmi") != NULL) && index_filename == -1) {
            index_filename = i; // Detecta tanto .vmx como .vmi
        }
    }

    // Si no se especificó con extensión explícita, tomar el primer argumento que no sea una bandera
    if (index_filename == -1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-d") != 0 && strcmp(argv[i], "-p") != 0 && strncmp(argv[i], "m=", 2) != 0) {
                index_filename = i;
                break;
            }
        }
    }

    if (index_filename == -1 || index_filename >= argc) {
        fprintf(stderr, "Error: No se especifico el archivo ejecutable (.vmx o .vmi)\n");
        return 1;
    }

    // =====================================================================
    // ATROPELLO/DESVÍO PARA ARCHIVOS DE IMAGEN (.VMI)
    // =====================================================================
    if (strstr(argv[index_filename], ".vmi") != NULL) {
        MaquinaVirtual vm;
        memset(&vm, 0, sizeof(MaquinaVirtual));

        if (cargar_imagen_vmi(&vm, argv[index_filename]) == 0) {
            ciclo_ejecucion(&vm); // Reanuda la ejecución directamente en el IP guardado
            free(vm.memoria);
            return 0; // Finaliza la ejecución exitosamente
        }
        return 1; // Error al cargar la imagen
    }

    // =====================================================================
    // FLUJO NORMAL PARA ARCHIVOS EJECUTABLES (.VMX)
    // =====================================================================
    FILE *archivo = fopen(argv[index_filename], "rb");
    if (!archivo) {
        perror("Error al abrir el archivo .vmx");
        return 1;
    }

    // Si no se pasó -p, usás la posición por defecto según si hay -d o no
    if (index_parametros == -1) {
        index_parametros = modo_disassembler ? 3 : 2;
    }


    // 1. Leer y validar cabecera
    uint8_t version = 0;
    uint16_t t_cs = 0, t_ds = 0, t_ks = 0, t_es = 0, t_ss = 0, entry_point = 0;
    if (validar_y_leer_cabecera(archivo, &version, &t_cs, &t_ds, &t_ks, &t_es, &t_ss, &entry_point) != 0) {
        fclose(archivo);
        return 1;
    }

    // 2. Calcular memoria de parámetros (PS)
    int cantidad_params = 0;
    uint32_t t_ps = calcular_tamano_ps(argc, argv, index_filename, index_parametros, &cantidad_params);

    // 3. Inicializar Máquina Virtual y RAM
    MaquinaVirtual vm;
    memset(&vm, 0, sizeof(MaquinaVirtual)); 
 
    //guardo el nombre del archivo
    for (int i = 1; i < argc; i++) {
        if (strstr(argv[i], ".vmi") != NULL) {
            strncpy(vm.vmi_filename, argv[i], sizeof(vm.vmi_filename) - 1);
            vm.vmi_filename[sizeof(vm.vmi_filename) - 1] = '\0';
            break;
        }
    }


    vm.tam_memoria = obtener_tamano_memoria(argc, argv);
    vm.memoria = (uint8_t *)malloc(vm.tam_memoria);
    if (!vm.memoria) {
        fprintf(stderr, "Error: Memoria insuficiente al instanciar la RAM (%u bytes)\n", vm.tam_memoria);
        fclose(archivo);
        return 1;
    }
    memset(vm.memoria, 0, vm.tam_memoria);


    // 4. Configurar descriptores, registros, parámetros y pila
    configurar_descriptores_y_registros(&vm, t_cs, t_ds, t_ks, t_es, t_ss, t_ps);
    cargar_parametros_ps(&vm, argc, argv, index_filename, index_parametros, cantidad_params);
    inicializar_pila(&vm, t_ss, cantidad_params);

    // 5. Cargar los segmentos desde el archivo a la memoria RAM
    if (fread(&vm.memoria[vm.TablaSegmentos[SEG_CS].base], sizeof(uint8_t), t_cs, archivo) != t_cs) {
        fprintf(stderr, "Error al leer el Code Segment\n");
        fclose(archivo);
        return 1;
    }

    if (version == 2 && t_ks > 0) {
        fread(&vm.memoria[vm.TablaSegmentos[SEG_KS].base], sizeof(uint8_t), t_ks, archivo);
    }
    fclose(archivo);

    // 6. Configurar el inicio de ejecución
    vm.registros[REG_IP] = vm.registros[REG_CS] | entry_point;

    // 7. Derivar ejecución
    if (modo_disassembler) {
        disassemble_proceso(
            vm.memoria,
            version,
            vm.TablaSegmentos[SEG_KS].base,
            t_ks,
            vm.TablaSegmentos[SEG_CS].base,
            t_cs,
            entry_point
        );
    } else {
        ciclo_ejecucion(&vm);
    }
    free(vm.memoria);
    return 0;
}

// =====================================================================
// FUNCIONES AUXILIARES
// =====================================================================

int validar_y_leer_cabecera(FILE *archivo, uint8_t *version, 
                            uint16_t *t_cs, uint16_t *t_ds, uint16_t *t_ks, 
                            uint16_t *t_es, uint16_t *t_ss, uint16_t *entry_point) {
    
    // 1. Vector de 6 posiciones para "VMX26" + '\0'
    char identificador[6] = {0};
    if (fread(identificador, sizeof(char), 5, archivo) != 5 || strncmp(identificador, "VMX26", 5) != 0) {
        fprintf(stderr, "Error: Identificador invalido. Debe ser 'VMX26'\n");
        return 1;
    }

    // 2. Versión (1 byte)
    if (fread(version, sizeof(uint8_t), 1, archivo) != 1 || (*version != 1 && *version != 2)) {
        fprintf(stderr, "Error: Version no compatible (se requiere version 1 o 2)\n");
        return 1;
    }

    // Valores por defecto para versión 1
    *t_ds = 1024;
    *t_ks = 1024;
    *t_es = 1024;
    *t_ss = 1024;
    *entry_point = 0;

    if (*version == 2) {
        // 3. Vector de 12 bytes para los descriptores de VMX2
        uint8_t buffer_header[12];
        if (fread(buffer_header, sizeof(uint8_t), 12, archivo) != 12) {
            fprintf(stderr, "Error al leer los tamanos de segmentos y entry point en VMX2\n");
            return 1;
        }

        // Armar los enteros de 16 bits en Big-Endian usando los índices  a
        *t_cs        = ((uint16_t)buffer_header[0]  << 8) | buffer_header[1];
        *t_ds        = ((uint16_t)buffer_header[2]  << 8) | buffer_header[3];
        *t_es        = ((uint16_t)buffer_header[4]  << 8) | buffer_header[5];
        *t_ss        = ((uint16_t)buffer_header[6]  << 8) | buffer_header[7];
        *t_ks        = ((uint16_t)buffer_header[8]  << 8) | buffer_header[9];
        *entry_point = ((uint16_t)buffer_header[10] << 8) | buffer_header[11];
    } else {
        // 4. Vector de 2 bytes para VMX1
        uint8_t tam_bytes[3];
        if (fread(tam_bytes, sizeof(uint8_t), 2, archivo) != 2) {
            fprintf(stderr, "Error al leer el tamano del codigo\n");
            return 1;
        }
        *t_cs = ((uint16_t)tam_bytes[0] << 8) | tam_bytes[1];
    }

    return 0;
}
 //debe ignorar las flags 'm' o 'vmi'
uint32_t calcular_tamano_ps(int argc, char *argv[], int index_filename, int index_parametros, int *cantidad_params) {
    uint32_t tamano_ps = 0;
    
    if (index_parametros > 0 && index_parametros < argc) {
        int count_extras = 0;
        for (int i = index_parametros; i < argc; i++) {
            if (strncmp(argv[i], "m=", 2) == 0 || strstr(argv[i], ".vmi") != NULL) continue;
            count_extras++;
        }
        *cantidad_params = 1 + count_extras; // param0 (nombre ejecutable) + parámetros
    } else {
        *cantidad_params = 1; // Solo param0 (nombre del ejecutable)
    }

    // Calcular bytes requeridos en PS (arreglo de punteros + strings)
    tamano_ps += (*cantidad_params) * 4;
    tamano_ps += strlen(argv[index_filename]) + 1;

    if (index_parametros > 0 && index_parametros < argc) {
        for (int i = index_parametros; i < argc; i++) {
            if (strncmp(argv[i], "m=", 2) == 0 || strstr(argv[i], ".vmi") != NULL) continue;
            tamano_ps += strlen(argv[i]) + 1;
        }
    }

    return tamano_ps;
}

void configurar_descriptores_y_registros(MaquinaVirtual *vm, uint16_t t_cs, uint16_t t_ds, uint16_t t_ks, uint16_t t_es, uint16_t t_ss, uint32_t t_ps) {
    uint32_t base_actual = 0;

    vm->TablaSegmentos[SEG_PS].base = base_actual;
    vm->TablaSegmentos[SEG_PS].size = t_ps;
    base_actual += t_ps;

    vm->TablaSegmentos[SEG_KS].base = base_actual;
    vm->TablaSegmentos[SEG_KS].size = t_ks;
    base_actual += t_ks;

    vm->TablaSegmentos[SEG_CS].base = base_actual;
    vm->TablaSegmentos[SEG_CS].size = t_cs;
    base_actual += t_cs;

    vm->TablaSegmentos[SEG_DS].base = base_actual;
    vm->TablaSegmentos[SEG_DS].size = t_ds;
    base_actual += t_ds;

    vm->TablaSegmentos[SEG_ES].base = base_actual;
    vm->TablaSegmentos[SEG_ES].size = t_es;
    base_actual += t_es;

    vm->TablaSegmentos[SEG_SS].base = base_actual;
    vm->TablaSegmentos[SEG_SS].size = t_ss;
    base_actual += t_ss;

    for (int i = 6; i < 8; i++) {
        vm->TablaSegmentos[i].base = 0xFFFF;   
        vm->TablaSegmentos[i].size = 0xFFFF; 
    }

    vm->registros[REG_PS] = (t_ps > 0) ? (SEG_PS << 16) : 0xFFFFFFFF;
    vm->registros[REG_KS] = (t_ks > 0) ? (SEG_KS << 16) : 0xFFFFFFFF;
    vm->registros[REG_CS] = (t_cs > 0) ? (SEG_CS << 16) : 0xFFFFFFFF;
    vm->registros[REG_DS] = (t_ds > 0) ? (SEG_DS << 16) : 0xFFFFFFFF;
    vm->registros[REG_ES] = (t_es > 0) ? (SEG_ES << 16) : 0xFFFFFFFF;
    vm->registros[REG_SS] = (t_ss > 0) ? (SEG_SS << 16) : 0xFFFFFFFF;

    if (base_actual > vm->tam_memoria) {
    fprintf(stderr, "Error: Memoria insuficiente en la RAM (%u bytes requeridos, %u disponibles).\n", 
     base_actual, vm->tam_memoria);
    exit(1);
  }
}

void cargar_parametros_ps(MaquinaVirtual *vm, int argc, char *argv[], int index_filename, int index_parametros, int cantidad_params) {
    if (cantidad_params == 0 || index_filename >= argc) return;

    uint32_t base_ps = vm->TablaSegmentos[SEG_PS].base;
    uint32_t offset_arreglo = 0;
    uint32_t offset_strings = cantidad_params * 4;

    // param0: Nombre del ejecutable (.vmx)
    uint32_t dir_logica_str = (SEG_PS << 16) | offset_strings;
    vm->memoria[base_ps + offset_arreglo + 0] = (dir_logica_str >> 24) & 0xFF;
    vm->memoria[base_ps + offset_arreglo + 1] = (dir_logica_str >> 16) & 0xFF;
    vm->memoria[base_ps + offset_arreglo + 2] = (dir_logica_str >> 8)  & 0xFF;
    vm->memoria[base_ps + offset_arreglo + 3] = dir_logica_str & 0xFF;
    offset_arreglo += 4;

    size_t len_prog = strlen(argv[index_filename]) + 1;
    memcpy(&vm->memoria[base_ps + offset_strings], argv[index_filename], len_prog);
    offset_strings += len_prog;

 // Parámetros adicionales (param1, param2, ...) -> argv, argv, ... en la VM
    if (index_parametros > 0 && index_parametros < argc) {
        for (int i = index_parametros; i < argc; i++) {
            if (strncmp(argv[i], "m=", 2) == 0 || strstr(argv[i], ".vmi") != NULL) continue;

            dir_logica_str = (SEG_PS << 16) | offset_strings;

            vm->memoria[base_ps + offset_arreglo + 0] = (dir_logica_str >> 24) & 0xFF;
            vm->memoria[base_ps + offset_arreglo + 1] = (dir_logica_str >> 16) & 0xFF;
            vm->memoria[base_ps + offset_arreglo + 2] = (dir_logica_str >> 8)  & 0xFF;
            vm->memoria[base_ps + offset_arreglo + 3] = dir_logica_str & 0xFF;
            offset_arreglo += 4;

            size_t len_param = strlen(argv[i]) + 1;
            memcpy(&vm->memoria[base_ps + offset_strings], argv[i], len_param);
            offset_strings += len_param;
        }
    }
}

void inicializar_pila(MaquinaVirtual *vm, uint16_t tamano_ss, int cantidad_params) {
    uint32_t puntero_sp = tamano_ss; 

    #define PUSH_INICIAL(val) \
        puntero_sp -= 4; \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 0] = ((val) >> 24) & 0xFF; \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 1] = ((val) >> 16) & 0xFF; \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 2] = ((val) >> 8) & 0xFF;  \
        vm->memoria[vm->TablaSegmentos[SEG_SS].base + puntero_sp + 3] = (val) & 0xFF;

    uint32_t puntero_argv = (cantidad_params > 0) ? (SEG_PS << 16) : 0xFFFFFFFF;
    
    PUSH_INICIAL(puntero_argv);       
    PUSH_INICIAL(cantidad_params);    
    PUSH_INICIAL(0xFFFFFFFF);         

    #undef PUSH_INICIAL

    vm->registros[REG_SP] = (SEG_SS << 16) | puntero_sp;
    vm->registros[REG_BP] = (SEG_SS << 16) | puntero_sp; 
}

/**
 * Recorre argv buscando la opción "m=M" (ej: "m=32").
 * Retorna el tamaño total de la memoria RAM expresado en BYTES.
 */
uint32_t obtener_tamano_memoria(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        // Verificar si el argumento comienza con "m="
        if (strncmp(argv[i], "m=", 2) == 0) {
            int val = atoi(argv[i]+ 2);  //saltea m y lee el numero
            if (val > 0) {
                return (uint32_t)val * 1024; // Retorna M en KiB convertidos a bytes
            }
        }
    }
    return RAM_SIZE;
}

int cargar_imagen_vmi(MaquinaVirtual *vm, const char *filename) {
    FILE *archivo = fopen(filename, "rb");
    if (!archivo) {
        perror("Error al abrir el archivo de imagen VMI");
        return 1;
    }

    // 1. Validar Header (8 bytes)
    char identificador = {0};
    if (fread(identificador, sizeof(char), 5, archivo) != 5 || strncmp(identificador, "VMI26", 5) != 0) {
        fprintf(stderr, "Error: Identificador VMI invalido (debe ser VMI26)\n");
        fclose(archivo);
        return 1;
    }

    uint8_t version = 0;
    fread(&version, sizeof(uint8_t), 1, archivo);

    uint16_t ram_kib = 0;
    fread(&ram_kib, sizeof(uint16_t), 1, archivo);

    // 2. Asignar RAM en la VM
    vm->tam_memoria = (uint32_t)ram_kib * 1024;
    vm->memoria = (uint8_t *)malloc(vm->tam_memoria);
    if (!vm->memoria) {
        fprintf(stderr, "Error al reservar RAM para la imagen VMI\n");
        fclose(archivo);
        return 1;
    }

    // 3. Cargar Registros (128 bytes)
    fread(vm->registros, sizeof(int32_t), 32, archivo);

    // 4. Cargar Tabla de Descriptores de Segmentos (32 bytes)
    fread(vm->TablaSegmentos, sizeof(SegmentDescriptor), 8, archivo);

    // 5. Cargar el volcado completo de la Memoria RAM
    fread(vm->memoria, sizeof(uint8_t), vm->tam_memoria, archivo);

    // Guardar el nombre por si ocurren nuevos breakpoints
    strncpy(vm->vmi_filename, filename, sizeof(vm->vmi_filename) - 1);

    fclose(archivo);
    return 0; // Exito
}
