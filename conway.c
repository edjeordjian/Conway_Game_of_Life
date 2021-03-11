#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DIR_DELIM '/'
#define PREFIJO "ESTADO"

typedef unsigned int uint;
typedef unsigned char uchar;

const uchar APAGADO = '1';
const uchar PRENDIDO = '0';

char* FLAG_HELP = "-h";
char* FLAG_OUTPUT = "-o";

    
void print_help() {
   printf("Uso:\n  ./conway -h\n  ./conway i M N inputfile \nOpciones:\n  -h, --help    Imprime este mensaje.\nEjemplo:\n  ./conway 120 80 80 gosper_glider_gun \n  Representa 120 iteraciones en una matriz de 80x80,\n  con un estado inicial tomado del archivo ‘‘gosper_glider_gun’’.\n");
}

bool verificar_inclusion_en_matriz(uint n_filas, uint n_columnas, uint fila_a_verificar, uint columna_a_verificar){
    if (!(fila_a_verificar < n_filas)){
        fprintf(stderr, "La combinacion fila columna [%d,%d] es invalida (error en fila).\n", fila_a_verificar, columna_a_verificar);
        return false;
    }
    
    if (!(columna_a_verificar < n_columnas)){
        fprintf(stderr, "La combinacion fila columna [%d,%d] es invalida (error en columna).\n", fila_a_verificar, columna_a_verificar);
        return false;
    }

    return true;
}

bool verificar_archivo(char* nombre_archivo, char* n_filas, char* n_columnas){
    FILE* file;

    char linea[256];
    char linea_aux[256];
    if ( !( file = fopen(nombre_archivo,"r") ) ){
        fprintf(stderr, "%s", "No se encontró el archivo de input.\n");
        return false;
    }

    while (fgets(linea, sizeof(linea), file)) {
        strcpy(linea_aux, linea);

        /* Se recorre cada valor separado por espacio de la línea.
         * strok requiere que para las siguientes iteraciones los valores de 
         * referencia sean los nulos agregados al string original.*/
        char *token = strtok(linea_aux, " ");
        int cant = 0;
        uint act = 0;
        bool invalido = false;
        while (token) {
            cant += 1;

            do {
                if( !isdigit( token[act]) ){
                    invalido = true;
                    break;
                }

                act += 1;

            } while( token[act] && act != (strlen(token) - 1) );

            act = 0;

            if(invalido){
                break;
            }

            token = strtok(NULL, " ");
        }

        if(cant != 2 || invalido){
            fprintf(stderr, "%s\n", "Error de formato en el archivo.");
            fclose(file);
            return false;
        }

        /* Si solo hay dos valores, y son números, se verifica
         * si están dentro del rango de la matriz*/
        token = strtok(linea, " ");

        char* fila_actual = token;
        token = strtok(NULL, "");
        char* columna_actual = token;

        if (! verificar_inclusion_en_matriz((uint)atoi(n_filas), (uint)atoi(n_columnas), (uint)atoi(fila_actual), (uint)atoi(columna_actual)) ){
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}

bool construir_matriz(uint M, uint N, uchar** mat) {
    uchar* tmp = malloc(sizeof(uchar*) * M * N);
    if (!tmp) {
        fprintf(stderr, "Error al construir matriz: falla al asignar memoria.\n");
        return false;
    }

    *mat = tmp;

    return true;
}

void copiar_matriz(uchar* dest, uchar* src, uint M, uint N) {
    for (uint i = 0; i < M; i++) {
        for (uint j = 0; j < N; j++){
            *(dest + i * N + j) = *(src + i * N + j);
        }
    }
}

void destruir_matriz(uchar* matriz) {
    free(matriz);
}

bool inicializar_matriz(uchar* matriz, uint M, uint N, const char* archivo) {
    for (uint i = 0; i < M; i++) {
        for (uint j = 0; j < N; j++) {
            *(matriz + i * N + j) = APAGADO;
        }
    }

    FILE* file;
    char line[256];
    if( ( file = fopen(archivo,"r") ) ){
        while (fgets(line, sizeof(line), file)) {
            char* token = strtok(line, " ");
            char* fila_actual = token;
            token = strtok(NULL, "");
            char* columna_actual = token;

            uint fila = (uint)atoi(fila_actual);
            uint col = (uint)atoi(columna_actual);

            *(matriz + fila * N + col) = PRENDIDO;
        }

        fclose(file);
    } 

    else {
        fprintf(stderr, "Error al abrir archivo %s\n", archivo);
        return false;
    }

    return true;
}

uint vecinos(uchar* matriz, uint x, uint y, uint M, uint N) {
    uint count = 0;

    for (int i = -1; i < 2; i++){
        for (int j = -1; j < 2; j++){
            if (i == 0 && j == 0)
                continue;

            int next_x = ( (int) x ) + i;
            int next_y = ( (int) y ) + j;

            // Casos borde
            if (next_x >= (int)M) next_x = 0;
            if (next_x < 0) next_x = (int)M - 1;

            if (next_y >= (int)N) next_y = 0;
            if (next_y < 0) next_y = (int)N - 1;

            uchar celda_vecina = *(matriz + next_x * (int)N + next_y);

            if (celda_vecina == PRENDIDO)
                count++;
        }
    }

    return count;
}

uchar siguiente_estado_celda(uchar* matriz, uint x, uint y, uint M, uint N) {
    uint v = vecinos(matriz, x, y, M, N);
    uchar estadoActual = *(matriz + x * N + y);

    if (estadoActual == PRENDIDO)
        return (v == 2 || v == 3) ? PRENDIDO : APAGADO;

    return (v == 3) ? PRENDIDO : APAGADO;
}

bool siguiente_estado(uchar* matriz, uint M, uint N) {
    uchar* matriz_temporal;

    if (!construir_matriz(M, N, &matriz_temporal)) {
        return false;
    }

    for (uint i = 0; i < M; i++) {
        for (uint j = 0; j < N; j++) {
            *(matriz_temporal + i * N + j) = siguiente_estado_celda(matriz, i, j, M, N);
        }
    }

    copiar_matriz(matriz, matriz_temporal, M, N);
    destruir_matriz(matriz_temporal);

    return true;
}

bool guardar_pbm(uchar* matriz, const char* output_prefix, uint nro_iteracion, uint M, uint N) {
    mkdir(output_prefix, 0700);

    // prefix / prefix + "_XXXX.pbm" + null terminator
    size_t size = strlen(output_prefix) * 2 + 1 + 9 + 1;
    char* filename = calloc(size, sizeof(char));
    if( !filename ){
        return false;
    }
    
    snprintf(filename, size, "%s/%s_%04d.pbm", output_prefix, output_prefix, nro_iteracion);

    FILE* file = fopen(filename, "w+");

    fprintf(file, "%s\n", "P1");
    fprintf(file, "%u %u\n", N, M);

    for (size_t i = 0; i < M; i++) {
        for (size_t j = 0; j < N; j++) {
            if (j != 0)
                fprintf(file, " ");

            uchar celda = *(matriz + i * N + j);
            fprintf(file, "%c", celda);
        }

        fprintf(file, "\n");
    }

    fclose(file);
    free(filename);
    return true;
}

bool crear_video(uint cant_imgs, const char* prefijo){
    ///// VIDEO /////
    ///// -loglevel quiet --> No pone output
    ///// -t n            --> Cantidad de segundos que dura
    ///// -r x            --> Cada imagen dura 1/x segundos en el video
    /////-loop 0          --> No se repite
    /////-r y             --> SEGUNDO -r, es los fps a los que va el video producido
    /////
    char* comando_1 = "ffmpeg -loglevel quiet -t ";
    char comando_2[12];
    sprintf(comando_2, "%d", cant_imgs/4);
    char* comando_3 = " -f image2 -r 4 -loop 0 -i '";
    // prefijo
    char* sufijo = "_%04d.pbm'";
    char* comando_4 =  " -r 30 '";

    // Tamaño de la fecha + null terminator
    char* timestamp = (char*) malloc(sizeof(char) * 20);
    if( !timestamp ){
        return false;
    }

    time_t ltime = time(NULL);
    struct tm* ts_aux = localtime(&ltime);

    sprintf(timestamp,"%04d_%02d_%02d_%02d_%02d_%02d", ts_aux->tm_year + 1900, ts_aux->tm_mon, ts_aux->tm_mday, ts_aux->tm_hour, ts_aux->tm_min, ts_aux->tm_sec);

    char* comando_5 =  ".mov'";

    char* comando = malloc(sizeof(char) * strlen(comando_1) + strlen(comando_2) + strlen(comando_3) + strlen(prefijo) * 2 + 1 + strlen(sufijo) + strlen(comando_4) + strlen(timestamp) + strlen(comando_5) +  1);
    if( !comando ){
        free(timestamp);
        return false;
    }

    strcpy(comando, comando_1);
    strcat(comando, comando_2);
    strcat(comando, comando_3);
    strcat(comando, prefijo);
    strcat(comando, "/");
    strcat(comando, prefijo);
    strcat(comando, sufijo);
    strcat(comando, comando_4);
    strcat(comando, timestamp);
    strcat(comando, comando_5);
    system(comando);
    free(timestamp);
    free(comando);

    //rm prefix/prefix_*
    char* nombre_archivos = malloc( sizeof(char) * strlen(prefijo) * 2 + 7);
    if( !nombre_archivos ){
        return false;
    }

    strcpy(nombre_archivos, "rm ");
    strcat(nombre_archivos, prefijo);
    strcat(nombre_archivos, "/");
    strcat(nombre_archivos, prefijo);
    strcat(nombre_archivos, "_*");
    system(nombre_archivos); 
    free(nombre_archivos);

    //rmdir prefix 
    size_t tam = sizeof(prefijo) + 7; 
    char* quitar_directorio = malloc( sizeof(char) * tam );

    if( !quitar_directorio ){
        return false;
    }
    
    snprintf(quitar_directorio, tam, "rmdir %s", prefijo);
    system(quitar_directorio);
    free(quitar_directorio);

    return true;
}

bool conway(uint iteraciones, uchar* matriz, uint M, uint N, const char* output_prefix) {
    // Guardo el estado inicial
    if( ! guardar_pbm(matriz, output_prefix, 0, M, N) ){
        return false;
    }

    for (uint i = 0; i < iteraciones; i++) {
        if (!siguiente_estado(matriz, M, N)) {
            fprintf(stderr, "Error al intentar computar el siguiente estado.\n");
            return false;
        }

        if( ! guardar_pbm(matriz, output_prefix, i + 1, M, N) ){
            return false;
        }
        
    }

    return true;
}

int main(int argc, char *argv[]) {
    if(argc == 2) {
        if(strcmp(argv[1],FLAG_HELP) == 0){
            print_help();
            return 0;
        }

        else{
            fprintf(stderr, "%s", "Opción incorrecta.\n");
            return 1;
        }
    }

    else if ( !(argc >= 5) ){
        fprintf(stderr, "%s", "No se encontró el archivo de input.\n");
        return 2;
    }

    // conway <Iteraciones> <TAM_horizontal> <Tam vertical> <Input file> [-o <output_file_name>]
    char* valor_i = argv[1];
    char* valor_M = argv[2];
    char* valor_N = argv[3];
    char* nombre_archivo = argv[4];
    char* nombre_output = PREFIJO;

    if(! verificar_archivo(nombre_archivo, valor_M, valor_N) ) {
        return 3;
    }

    uint its = (uint) atoi(valor_i);
    uint M   = (uint) atoi(valor_M);
    uint N   = (uint) atoi(valor_N);

    if (its > 9999){
        fprintf(stderr, "Valor inadecuado de iteraciones.\n");
        return 4;
    }

    uchar* estado_inicial;

    if (!construir_matriz(M, N, &estado_inicial)) {
        return 5;
    }

    if (!inicializar_matriz(estado_inicial, M, N, nombre_archivo)) {
        fprintf(stderr, "Error al leer estado inicial\n");
        destruir_matriz(estado_inicial);
        return 6;
    }

    if (!conway(its, estado_inicial, M, N, nombre_output)) {
        fprintf(stderr, "Error al iterar los diferentes estados.\n");
        destruir_matriz(estado_inicial);
        return 7;
    }

    if( !crear_video(its + 1, nombre_output) ){
        fprintf(stderr, "Error al crear el video.\n");
        destruir_matriz(estado_inicial);
        return 8;
    }

    destruir_matriz(estado_inicial);
    return 0;
}
