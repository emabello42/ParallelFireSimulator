#include "fireSimParallel.h"



/*estructura que contiene infomación referida a cada uno de los parámetros usados en la simulación:
 cada campo es un ptr a un vector de todos los valores posibles para el parámetro correspondiente a ese
 parámetros*/
typedef struct{
        int cantModel;
        size_t  * Model;      /* NFFL 1 */
        int cantWindSpd;
        double * WindSpd;     /* mph */
        int cantWindDir;
        double * WindDir;     /* degrees clockwise from north */
        int cantSlope;
        double * Slope;    /* fraction rise / reach */
        int cantAspect;
        double * Aspect;    /* degrees clockwise from north */
        int cantM1;
        double * M1;    /* 1-hr dead fuel moisture */
        int cantM10;
        double * M10;    /* 10-hr dead fuel moisture */
        int cantM100;
        double * M100;    /* 100-hr dead fuel moisture */
        int cantMherb;
        double * Mherb;   /* Live herbaceous fuel moisture */
        int cantMwood;
        double * Mwood;   /* Live woody fuel moisture */
}InfoParam;

/*estructura que contiene los mutexes, variables de condición y otras
 * variables necesarias para controlar condiciones, necesarios para lograr la exclusión mutua y el acceso
 condicional a la regiones criticas de los hilos del proceso master*/
typedef struct{
    /*mutex usados por los hilos receiver y sumador*/
    pthread_mutex_t mutexBufferIgnMap;
    pthread_mutex_t mutexBufferFlMap;
    /*variables de condición usadas por los hilos receiver y sumador*/
    pthread_cond_t condcBufferIgnMap;
    pthread_cond_t condpBufferIgnMap;
    pthread_cond_t condcBufferFlMap;
    pthread_cond_t condpBufferFlMap;
    /*contadores usados para controlar si un determinado buffer está vacio o lleno*/
    int condicion_buffer_ignMap;
    int condicion_buffer_flMap;
}VarsMC;
/*estructura usada para enviar parámetros a los hilos del proceso master
una sola instancia de la misma es creada por el proceso master, y compartida por los hilos del mismo*/
typedef struct{
    /*nombres de archivos de salida*/
    char * incendiosFile;
    char * llamasFile;
    /*nro total de combinaciones a tratar*/
    double nroCombinaciones;
    int nroCombinacionesPorWorker;
    int nroCombinacionesWorkerRestante;
    /*punteros a los buffers que contienen mapas enviados por los workers*/
    double **buffer_ignMap;             /* buffer que contiene los mapas de tiempo de encendido  (minutes) */
    double **buffer_flMap;              /* buffer que contiene los mapas de altura de las llamas */
    /*tamaño de los bufferes ignMap y flMap*/
    int tamBuffers;
    /*información referente a MPI*/
    int nproc;//número de procesos
    int nroDeWorkers;//número de procesos workers
    /*ptr a struct con datos de los parámetros: límite inferior, límite superior y salto*/
    InfoParam * infoParam;
    /*ptr a struct con info sobre los mapas a usar*/
    InfoMaps * infoMaps;
    /*ptr a struct con variables de condición y mutexes*/
    VarsMC * varsMC;
    int tamPaquete;//tamaño del paquete enviado a cada worker, en numero de combinaciones por paquete
} ArgToThread;
VarsMC * inicializarVarsMC();
int setearNroNodos(int nroProc, const double * const nroDeCombinaciones);
InfoParam * obtenerInfoParam(char * fileName_parameters);
double calcularNroCombinaciones(InfoParam * infoParam);/*calcula el nro de combinaciones total*/
void calcularNroCombinacionesPorWorker(int * nroCombWorker, int * nroCombWorkerFinal, int nroDeWorkers,const double * const nroDeCombinacines);/*calcula el nro de combinaciones que le corresponden a cada worker*/
InfoMaps * obtenerDimMaps(char * fileName_maps);
void PrintMap(double *map, char *fileName , InfoMaps * infoMaps);/*imprime un mapa en pantalla*/

//HILOS DEL MASTER
void * enviarTrabajos(void *);//método principal del hilo sender
void * recibirResultados(void *);//método principal del hilo receiver
void * sumarResultados(void *);//método principal del hilo sumador


