#include "fireSimParallel.h"



/*estructura que contiene los mutexes, variables de condición y otras
 * variables necesarias para controlar condiciones, necesarios para lograr la exclusión mutua y el acceso
 condicional a la regiones criticas de los hilos del proceso worker*/
typedef struct{
    /*mutex usados por los hilos receiver y sumador*/
    pthread_mutex_t mutex_in;
    pthread_mutex_t mutex_out_ignMap;
    pthread_mutex_t mutex_out_flMap;
    /*variables de condición usadas por los hilos receiver y sumador*/
    pthread_cond_t condc_in;
    pthread_cond_t condc_out_ignMap;
    pthread_cond_t condc_out_flMap;
    /*contadores usados para controlar si un determinado buffer está vacio o lleno*/
    int condicionEntrada;
    int condicion_buffer_ignMap;
    int condicion_buffer_flMap;
}VarsMC;

typedef struct{
    int my_id;
    int * nroCombinaciones;
    int nroDeEnviosARecibir;
    int my_nroCombinaciones;
    VarsMC * varsMC;
    InfoMaps * infoMaps;
    /*buffer usado por cada worker, cada uno tiene su propia copia, a la cual tiene que asignarle espacio previamente,
 de modo que dos de sus hilos (de recepción y el de simulación)
 lo usen como buffer de trabajo, es decir, el hilo de recepción es el productor y el de simulación es el consumidor*/
    ParamValues * bufferDeEntrada;
/*los siguiente dos bufferes también corresponden a un problema de productor consumidor*/
    double **buffer_ignMap;             /* buffer que contiene los mapas de tiempo de encendido  (minutes) */
    double **buffer_flMap;              /* buffer que contiene los mapas de altura de las llamas */
}ArgToThreadW;


//HILOS DEL WORKER
void * recibirTrabajos(void *);
void * simular(void *);
void * enviarResultados(void *);