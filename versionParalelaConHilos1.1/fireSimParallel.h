#include "fireLib.h"
#include <pthread.h> //hay que enlazar con la opcion -lpthread
#include <mpi.h>
#define NRO_PARAMETROS      10
#define NODO_MASTER         0   /*id del nodo master*/
/*etiquetas usadas en el envio de mensajes con MPI*/
#define VALORES_TAG         4 //se refiere a la transmisión de los valores de parámetros a probar
#define RESULT_TAG          5 //se refiere al envio de resultados al master desde los workers
#define INFO_FOR_WORKER_TAG     6//se refiere a la informacion previa enviada a cada uno de los workers
#define TAM_INFO_FOR_WORKER_TAG 7
/*estructura que contiene información sobre los mapas*/
typedef struct{
    int Rows;//número de filas
    int Cols;//número de columnas
    double CellWd;//ancho de cada una de las celdas
    double CellHt;//altura de cada una de las celdas
    int cells; //indica la cantidad de celdas de cada mapa
}InfoMaps;

/*struct que contiene toda la información requerida por los workers, a excepción de los conjuntos de parámetros*/
typedef  struct{
    int nroDeEnvios;//nro de envios a recibir del master
    int nroDeCombinacionesWorker;//nro de combinaciones a recibir
    int * nroCombinaciones;//arreglo con las combinaciones a enviar en cada envio
    InfoMaps * infoMaps;//información de los mapas a usar
}InfoForWorkers;

void codigoMaster(int,char*,char*,char*,char*,int);//codigo ejecutado por el procesos master
void codigoWorker(int);//código ejecutado por el proceso worker
/*declaración de funciones*/

typedef struct{
        double Model;      /* NFFL 1 */
        double WindSpd;     /* mph */
        double WindDir;     /* degrees clockwise from north */
        double Slope;    /* fraction rise / reach */
        double Aspect;    /* degrees clockwise from north */
        double M1;    /* 1-hr dead fuel moisture */
        double M10;    /* 10-hr dead fuel moisture */
        double M100;    /* 100-hr dead fuel moisture */
        double Mherb;   /* Live herbaceous fuel moisture */
        double Mwood;   /* Live woody fuel moisture */
}ParamValues;//estructura que contiene una combinaciones de valores de los parámetros en cada instante

