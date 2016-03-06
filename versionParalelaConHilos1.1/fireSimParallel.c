/*
 *******************************************************************************
 *
 *  fireSim.c
 *
 *  Legalities
 *      Copyright (c) 1996 Collin D. Bevins.
 *      See the file "license.txt" for information on usage and redistribution
 *      of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *VERSIÓN PARALELA
 
 version 1.1
 fecha: 5 de diciembre del 2010
 autor: Emmanuel Bello (eware42)
 *******************************************************************************
 Para poder ejecutar este programa es necesario recibir como parámetros los nombres de los siguientes archivos, en el siguiente orden:
<tamaño de mapas> 
 * <archivo con parámetros, a variar por cada worker, para la simulación>
 * <archivo con las probabilidades de incendio por cada celda>
 <archivo con las alturas de las llamas por cada celda>
 <tamaño de los mensajes que tiene que enviar el master a los workers (en nro de combinaciones por mensaje)>
 */

#include <time.h>
#include "fireSimParallel.h"

int main ( int argc, char **argv )
{
int my_id, nproc;
MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
clock_t a, b;
if(argc != 6){
        printf("Nro incorrecto de parametros:\n");
        exit(-1);
}
else{
if(my_id == NODO_MASTER){
        a = clock();
        codigoMaster(nproc,argv[1],argv[2],argv[3],argv[4],atoi(argv[5]));
        b = clock();
        printf("tiempo transcurrido = %g seconds\n",(double)(b-a)/(double)CLOCKS_PER_SEC);
}//fin if
else{
    codigoWorker(my_id);
}//fin else
exit(0);
}//fin else
}//fin main
