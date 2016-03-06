#include "fireLib.h"
#include <pthread.h> //hay que enlazar con la opcion -lpthread
#include <mpi.h>
#define NRO_PARAMETROS      10
#define MATRIZ_TAG          98
#define NODO_MASTER         0   /*id del nodo master*/
/*etiquetas usadas en el envio de mensajes con MPI*/
#define COMBINACIONES_TAG   3   //etiqueta referente a la transferencia del numerode combinaciones  a trata por cada worker
#define VALORES_TAG         9 //se refiere a la transmisión de los valores de parámetros a probar
#define RESULT_TAG          10 //se refiere al envio de resultados al master desde los workers
#define NRO_ENVIOS_TAG      8
#define COMBPORWORKER_TAG   1
/*estructura que contiene información sobre los mapas*/
typedef struct{
    int Rows;//número de filas
    int Cols;//número de columnas
    double CellWd;//ancho de cada una de las celdas
    double CellHt;//altura de cada una de las celdas
}InfoMaps;


void codigoMaster(int,int,char*,char*,char*,char*,int);//codigo ejecutado por el procesos master
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

