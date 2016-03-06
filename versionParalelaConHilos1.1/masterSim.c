#include "masterSim.h"

/*******************************CODIGO EJECUTADO EN EL MASTER******************************/
/**********************************************************************************************************/
void codigoMaster(int nproc, char * fileName_maps, char * fileName_param, char * fileName_outIncencio,
        char * fileName_outLlamas,int tam_paquete){
/*El master se encarga de generar los valores y decidir las combinaciones de parámetros que tiene que usar cada
 worker para ejecutar la simulación. Además tiene que recibir los resultados retornados por los workers.
 * Por último tiene que sumar y promediar los resultados retornados por los workers
 Para ello, se usan dos hilos: uno de envio tareas y otro de recepción de resultados*/
 
        pthread_t * sender;
        pthread_t * receiver;
        pthread_t * sumador;
        void * return_value;
        int estado,i;
        /*estructura de datos que contiene los datos a enviar a los hilos en el proceso master
         Se mantiene una única copia de esta estructura en todo el proceso master, y se pasa a cada uno de los
         hilos en el mismo la dirección de la misma*/
        ArgToThread * argToThread;
        argToThread = (ArgToThread *)malloc(sizeof(ArgToThread));
        argToThread->tamPaquete = tam_paquete;
        /*se obtiene info sobre los parámetros, del archivo*/
        argToThread->infoParam = obtenerInfoParam(fileName_param);
        /*se calculan la cantidad de combinaciones en base al conjunto de parámetros*/
        argToThread->nroCombinaciones = calcularNroCombinaciones(argToThread->infoParam);
        /*se setea el nro de nodos y el nro de workers*/
        argToThread->nproc = setearNroNodos(nproc, &(argToThread->nroCombinaciones));
        argToThread->nroDeWorkers = argToThread->nproc - 1;
        /*se calcula el nro de combinaciones por workers*/
        calcularNroCombinacionesPorWorker(&(argToThread->nroCombinacionesPorWorker),
          &(argToThread->nroCombinacionesWorkerRestante), argToThread->nroDeWorkers,&(argToThread->nroCombinaciones));
        argToThread->infoMaps = obtenerDimMaps(fileName_maps);
        argToThread->varsMC =  inicializarVarsMC();/*se inicializan variables de condición y los mutex*/
        argToThread->incendiosFile = fileName_outIncencio;
        argToThread->llamasFile = fileName_outLlamas;
        /*se asigna espacio a los bufferes usados por los hilos
         receiver y sumador*/
        argToThread->buffer_ignMap =
                (double **)malloc(argToThread->nroDeWorkers *sizeof(double*));
        argToThread->buffer_flMap =
                (double **)malloc(argToThread->nroDeWorkers*sizeof(double*));
        for(i = 0; i < argToThread->nroDeWorkers ; i++){
            argToThread->buffer_ignMap[i] =
                    (double *)malloc(argToThread->infoMaps->cells *sizeof(double));
            argToThread->buffer_flMap[i] =
                    (double *)malloc(argToThread->infoMaps->cells *sizeof(double));
        }//fin for
        /*se crean e inician los hilos del master*/
        sender = (pthread_t *)malloc(sizeof(pthread_t));
        receiver = (pthread_t *)malloc(sizeof(pthread_t));
        sumador = (pthread_t *)malloc(sizeof(pthread_t));
        estado = pthread_create(sender,NULL,enviarTrabajos,(void *)argToThread);
        if(estado){
                printf("error al crear hilo sender\n");
                exit(-1);
        }//fin if
      
        estado = pthread_create(receiver,NULL,recibirResultados,(void *)argToThread);
        if(estado){
                printf("error al crear hilo receiver\n");
                exit(-1);
        }//fin if
        estado = pthread_create(sumador,NULL,sumarResultados,(void *)argToThread);
        if(estado){
                printf("error al crear hilo sumador\n");
                exit(-1);
        }//fin if
        /*se espera a que terminen de ejecutarse ambos hilos*/
        if(pthread_join(*sender, &return_value)) printf("error al terminar el hilo sender\n");
        if(pthread_join(*receiver, &return_value)) printf("error al terminar el hilo receiver\n");
        if(pthread_join(*sumador, &return_value)) printf("error al terminar el hilo sumador\n");
        //se destruyen las variables de condicion
        pthread_cond_destroy(&(argToThread->varsMC->condcBufferIgnMap));
        pthread_cond_destroy(&(argToThread->varsMC->condcBufferFlMap));
        pthread_mutex_destroy(&(argToThread->varsMC->mutexBufferIgnMap));
        pthread_mutex_destroy(&(argToThread->varsMC->mutexBufferFlMap));
        free(sender);
        free(receiver);
        free(sumador);
        /*
        //se libera el espacio asignado a argToThread
        free(argToThread->infoMaps);
        free(argToThread->infoParam);
        free(argToThread->varsMC);
        //se libera el espacio usados por los mapas
        for(i = 0 ; i < argToThread->nroCombinacionesPorWorker;i++){
            free(argToThread->buffer_ignMap[i]);
            free(argToThread->buffer_flMap[i]);
        }//fin for
        
        free(argToThread->buffer_ignMap);
        free(argToThread->buffer_flMap);
        */
        MPI_Finalize();
}//fin codigoMaster
/**********************************************************************************************************/
InfoParam * obtenerInfoParam(char * fileName_parameters){
    InfoParam * infoParam;
    int j,dataInt;
    double dataDouble;
    FILE *fparam;
    infoParam = (InfoParam *)malloc(sizeof(InfoParam));
     /*se leen los parámetros*/
        fparam = fopen(fileName_parameters,"r");
        /*se lee Model*/
        fscanf(fparam,"(Model,%d):\t",&(infoParam->cantModel));
        infoParam->Model = (size_t *)malloc((infoParam->cantModel)*sizeof(size_t));
        for(j = 0 ; j < (infoParam->cantModel)-1 ; j++){
            fscanf(fparam,"%d\t",&dataInt);
            infoParam->Model[j] = dataInt;
        }
        fscanf(fparam,"%d\n",&dataInt);
        infoParam->Model[j] = dataInt;

        /*se lee WindSpd*/
        fscanf(fparam,"(WindSpd,%d):\t",&(infoParam->cantWindSpd));
        infoParam->WindSpd = (double *)malloc((infoParam->cantWindSpd)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantWindSpd)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->WindSpd[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->WindSpd[j] = dataDouble;

        /*se lee WindDir*/
        fscanf(fparam,"(WindDir,%d):\t",&(infoParam->cantWindDir));
        infoParam->WindDir = (double *)malloc((infoParam->cantWindDir)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantWindDir)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->WindDir[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->WindDir[j] = dataDouble;

        /*se lee Slope*/
        fscanf(fparam,"(Slope,%d):\t",&(infoParam->cantSlope));
        infoParam->Slope = (double *)malloc((infoParam->cantSlope)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantSlope)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->Slope[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->Slope[j] = dataDouble;

        /*se lee Aspect*/
        fscanf(fparam,"(Aspect,%d):\t",&(infoParam->cantAspect));
        infoParam->Aspect = (double *)malloc((infoParam->cantAspect)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantAspect)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->Aspect[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->Aspect[j] = dataDouble;

        /*se lee M1*/
        fscanf(fparam,"(M1,%d):\t",&(infoParam->cantM1));
        infoParam->M1 = (double *)malloc((infoParam->cantM1)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantM1)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->M1[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->M1[j] = dataDouble;

        /*se lee M10*/
        fscanf(fparam,"(M10,%d):\t",&(infoParam->cantM10));
        infoParam->M10 = (double *)malloc((infoParam->cantM10)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantM10)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->M10[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->M10[j] = dataDouble;

        /*se lee M100*/
        fscanf(fparam,"(M100,%d):\t",&(infoParam->cantM100));
        infoParam->M100 = (double *)malloc((infoParam->cantM100)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantM100)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->M100[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->M100[j] = dataDouble;

        /*se lee Mherb*/
        fscanf(fparam,"(Mherb,%d):\t",&(infoParam->cantMherb));
        infoParam->Mherb = (double *)malloc((infoParam->cantMherb)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantMherb)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->Mherb[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->Mherb[j] = dataDouble;

        /*se lee Mwood*/
        fscanf(fparam,"(Mwood,%d):\t",&(infoParam->cantMwood));
        infoParam->Mwood = (double *)malloc((infoParam->cantMwood)*sizeof(double));
        for(j = 0 ; j < (infoParam->cantMwood)-1 ; j++){
            fscanf(fparam,"%lf\t",&dataDouble);
            infoParam->Mwood[j] = dataDouble;
        }
        fscanf(fparam,"%lf\n",&dataDouble);
        infoParam->Mwood[j] = dataDouble;

        fclose(fparam);
        return infoParam;
}//fin de InfoParam * obtenerInfoParam(char * fileName_parameters);
/*****************************************************************************************/
double calcularNroCombinaciones(InfoParam * infoParam){
    return (double)(
                infoParam->cantModel*
                infoParam->cantAspect*
                infoParam->cantWindSpd*
                infoParam->cantMherb*
                infoParam->cantWindDir*
                infoParam->cantSlope*
                infoParam->cantMwood*
                infoParam->cantM100*
                infoParam->cantM10*
                infoParam->cantM1
            )
            ;
}//fin de double calcularNroCombinaciones(InfoParam * infoParam)
/************************************************************/
int setearNroNodos(int nroProc, const double * const nroDeCombinaciones){
    int  j;
    if((nroProc - 1) > (*nroDeCombinaciones)){
            
            /*se les avisa a los workers sobrantes que el número de combinaciones que le corresponden es cero,
             de modo que terminan*/
            int tamOut;
            tamOut = 0;
            for(j = (*nroDeCombinaciones) +1; j<= (nroProc - 1);j++ ){
                MPI_Send(&tamOut,1, MPI_INT,j,TAM_INFO_FOR_WORKER_TAG,MPI_COMM_WORLD);
            }//fin for
            return (*nroDeCombinaciones) +1;
    }//fin if
    else{    return nroProc;}
}
/***********************************************************/
void calcularNroCombinacionesPorWorker(int * nroCombWorker, int * nroCombWorkerFinal,
        int nroDeWorkers,const double * const nroDeCombinacines){
     *nroCombWorker = (int)((*nroDeCombinacines)/nroDeWorkers);
    /*se asigna el nro de combinaciones a tratar al último worker*/
        if((int)(*nroDeCombinacines) % (nroDeWorkers) == 0){
            *nroCombWorkerFinal = *nroCombWorker;
        }//fin if
        else{
            *nroCombWorkerFinal = *nroCombWorker + (int)(*nroDeCombinacines) % (nroDeWorkers) ;
        }//fin else
}//fin de calcularNroCombinacionesPorWorker(int * nroCombWorker, int * nroCombWorkerFinal, double * nroDeCombinacines)
/*************************************************************/
InfoMaps * obtenerDimMaps(char * fileName_maps){
        FILE *fmaps;
        InfoMaps * infoMaps;
        fmaps = fopen(fileName_maps,"r");
        infoMaps = (InfoMaps *)malloc(sizeof(InfoMaps));
        fscanf(fmaps,"Rows\t=\t%d\n",&(infoMaps->Rows));//se lee la cantidad de filas
        fscanf(fmaps,"Cols\t=\t%d\n",&(infoMaps->Cols));// se lee la cantidad de columnas
        fscanf(fmaps,"CellWd\t=\t%lf\n",&(infoMaps->CellWd)); //se lee el ancho de celda
        fscanf(fmaps,"CellHt\t=\t%lf\n",&(infoMaps->CellHt)); //se lee el alto de celda
        infoMaps->cells = infoMaps->Cols * infoMaps->Rows;
        fclose(fmaps);
        return infoMaps;
}
/************************************************************/
VarsMC * inicializarVarsMC(){
    VarsMC * varsMC;
    varsMC = (VarsMC*)malloc(sizeof(VarsMC));
    varsMC->condicion_buffer_flMap = 0;
    varsMC->condicion_buffer_ignMap =  0;
    pthread_mutex_init(&(varsMC->mutexBufferIgnMap),NULL);
    pthread_mutex_init(&(varsMC->mutexBufferFlMap),NULL);
    pthread_cond_init(&(varsMC->condcBufferIgnMap),0);
    pthread_cond_init(&(varsMC->condcBufferFlMap),0);
    return varsMC;
}//fin de VarsMC * inicializarVarsMC()
/*********************************************************/

/*******************CÓDIGOS EJECUTADOS POR LOS HILOS**************************/
/*funciones ejecutas por el hilo sender*/
/****************************************************************************/
void * enviarTrabajos(void * argToThread){
        ParamValues * paramValues;//estructura que contiene una combinación de valores de los parámetros en cada instante
        ArgToThread * args;
        args = (ArgToThread *)argToThread;
        int ** nroCombinaciones;//arreglo con el numero de combinaciones que le corresponden a cada worker por envio
        int i,j;
        int position;
        char * outBuf;
        InfoForWorkers * infoForWorkers;//información que se debe enviar previamente a todos los workers excepto al último
        InfoForWorkers * infoForWorkerRestante;//información que se le debe enviar previamente al último worker
        int tam_out;//tamaño del buffer de salida
        /*se asigna espacio para los inforForWorkers*/
        infoForWorkers = (InfoForWorkers *)malloc(sizeof(InfoForWorkers));
        infoForWorkerRestante = (InfoForWorkers *)malloc(sizeof(InfoForWorkers));
        nroCombinaciones =
        determinarInfoForWorkers(args,infoForWorkers,infoForWorkerRestante);/*se determina la informacion a enviar
                                                                          a cada worker*/
        sendInfoToWorkers(args,infoForWorkers,infoForWorkerRestante);/*se empaqueta y envia dicha información a los workers*/
        
        paramValues = (ParamValues*)malloc(sizeof(ParamValues));
        /*El criterio que se sigue para distribuir todas las combinaciones posibles de parámetros entre los workers es:
        por cada parámetro dividir el conjunto de valores para el mismo en dos, enviar la primera mitad, en el próximo ciclo enviar
        la otra mitad de la misma manera, es decir, de nuevo dividiendola por la mitad, enviando la primera parte, y así sucesivamente*/
        /*En cada combinación posible, se empaqueta una estructura con los valores de dicha combinación
         de parámetros. Si ya se alcanzó el número de combinaciones a enviar a un worker determinado, es decir, la mitad
         de el número total de combinaciones, se envia el paquete al worker respectivo, y se pasa a empaquetar
         el siguiente conjunto de combinaciones para enviarselas al siguiente worker*/
        i = 0;//índice usado en nroCombinaciones
        j=0;//indice usado en el envio de paquetes a los workers
        position = 0;
        int a1,a2,a3,a4,a5,a6,a7,a8,a9,a10;
        tam_out =nroCombinaciones[i][j]*NRO_PARAMETROS*sizeof(double);//se asigna espacio para almacenar el siguiente paquete a enviar
        outBuf = (char*)malloc(tam_out);
        
        for(a1 = 0; a1 < args->infoParam->cantModel ;a1++){
         paramValues->Model = (double)args->infoParam->Model[a1];
         for(a2=0; a2 < args->infoParam->cantAspect ;a2++){
          paramValues->Aspect = args->infoParam->Aspect[a2];
          for(a3=0; a3 <args->infoParam->cantWindSpd;a3++){
           paramValues->WindSpd = args->infoParam->WindSpd[a3];
           for(a4=0;a4< args->infoParam->cantMherb;a4++){
           paramValues->Mherb = args->infoParam->Mherb[a4];
            for(a5=0;a5 < args->infoParam->cantWindDir;a5++){
            paramValues->WindDir = args->infoParam->WindDir[a5];
             for(a6=0;a6< args->infoParam->cantSlope ;a6++){
             paramValues->Slope = args->infoParam->Slope[a6];
              for(a7=0;a7 < args->infoParam->cantMwood;a7++){
              paramValues->Mwood = args->infoParam->Mwood[a7];
               for(a8=0;a8< args->infoParam->cantM100 ;a8++){
               paramValues->M100 = args->infoParam->M100[a8];
                for(a9=0;a9< args->infoParam->cantM10 ;a9++){
                paramValues->M10 = args->infoParam->M10[a9];
                 for(a10=0; a10< args->infoParam->cantM1 ;a10++){
                 paramValues->M1 = args->infoParam->M1[a10];
                 MPI_Pack(paramValues,NRO_PARAMETROS,MPI_DOUBLE,outBuf,tam_out,&position,MPI_COMM_WORLD);
                     nroCombinaciones[i][j]--;
                     if(nroCombinaciones[i][j] == 0){
                        MPI_Send(outBuf,position, MPI_PACKED,i+1,VALORES_TAG,MPI_COMM_WORLD);
                        position = 0;
                        i = (i+1) % (args->nroDeWorkers);//se pasa al siguiente worker
                        if(i == 0) j++;//se pasa al siguiente grupo de combinaciones
                        free(outBuf);
                        tam_out =nroCombinaciones[i][j]*NRO_PARAMETROS*sizeof(double);//se asigna espacio para almacenar el siguiente paquete a enviar
                        outBuf = (char*)malloc(tam_out);
                     }//FIN IF   
                 }//fin for
                }//fin for
               }//fin for
              }//fin for
             }//fin for
            }//fin for
           }//fin for
          }//fin for
         }//fin for
        }//fin for
        free(args->infoParam->Aspect);
        free(args->infoParam->M1);
        free(args->infoParam->M10);
        free(args->infoParam->M100);
        free(args->infoParam->Mherb);
        free(args->infoParam->Model);
        free(args->infoParam->Mwood);
        free(args->infoParam->Slope);
        free(args->infoParam->WindSpd);
        free(args->infoParam->WindDir);
        free(args->infoParam);
        free(outBuf);
        free(paramValues);
        
        pthread_exit(0);
}//fin de enviarTrabajos

/*esta función determina las información previa a enviar a cada uno de los workers y además retorna una
 matriz con los envios a enviar a cada uno de los workers*/
int ** determinarInfoForWorkers(ArgToThread * args,InfoForWorkers * infoForWorkers, InfoForWorkers * infoForWorkerRestante){
    int ** nroCombinaciones;
    int i,j;
    int nroDeEnvios, nroDeEnviosWR;
    /*se asigna espacio para almacenar el vector con el nro de combinaciones que le corresponde a cada worker*/
        nroCombinaciones = (int **)malloc((args->nroDeWorkers)*sizeof(int*));

        /*Se calcula y setean los datos a enviar al último worker*/

        /*se calcula el número de combinaciones a enviar al último worker*/
        if((args->nroCombinacionesWorkerRestante % args->tamPaquete) == 0){
                    nroDeEnviosWR = (int)(args->nroCombinacionesWorkerRestante / args->tamPaquete);
                    nroCombinaciones[args->nroDeWorkers-1] = (int *)malloc(nroDeEnviosWR*sizeof(int));
                    for(j = 0 ; j < nroDeEnviosWR ; j++){
                        nroCombinaciones[args->nroDeWorkers-1][j] = args->tamPaquete;
                    }//fin for
        }
        else{
            nroDeEnviosWR = ((int)(args->nroCombinacionesWorkerRestante / args->tamPaquete)) + 1;
                    nroCombinaciones[args->nroDeWorkers-1] = (int *)malloc(nroDeEnviosWR*sizeof(int));
                    for(j = 0 ; j < nroDeEnviosWR-1 ; j++){
                        nroCombinaciones[args->nroDeWorkers-1][j] = args->tamPaquete;
                    }//fin for
                    nroCombinaciones[args->nroDeWorkers-1][j] = (args->nroCombinacionesWorkerRestante % args->tamPaquete);
        }
        infoForWorkerRestante->nroCombinaciones = nroCombinaciones[args->nroDeWorkers-1];
        infoForWorkerRestante->nroDeEnvios = nroDeEnviosWR;
        infoForWorkerRestante->nroDeCombinacionesWorker = args->nroCombinacionesWorkerRestante;
        infoForWorkerRestante->infoMaps = args->infoMaps;

        /*Se calculan y setean los datos a enviar al resto de los workers*/

        /*se asigna el nro de combinaciones a tratar al resto de los workers*/
        if((args->nroCombinacionesPorWorker % args->tamPaquete) == 0){
            nroDeEnvios = (int)args->nroCombinacionesPorWorker / args->tamPaquete;
                for(i = 0;i<(args->nroDeWorkers-1);i++){
                    nroCombinaciones[i] = (int *)malloc(nroDeEnvios*sizeof(int));
                    for(j = 0 ; j < nroDeEnvios ; j++){
                        nroCombinaciones[i][j] = args->tamPaquete;
                    }//fin for
                }//fin for
        }
        else{
            nroDeEnvios = ((int)(args->nroCombinacionesPorWorker / args->tamPaquete)) + 1;
            for(i = 0;i<(args->nroDeWorkers-1);i++){
                    nroCombinaciones[i] = (int *)malloc(nroDeEnvios*sizeof(int));
                    for(j = 0 ; j < (nroDeEnvios-1) ; j++){
                        nroCombinaciones[i][j] = args->tamPaquete;
                    }//fin for
                    nroCombinaciones[i][j] = (args->nroCombinacionesPorWorker % args->tamPaquete);
            }//fin for
        }
        infoForWorkers->nroCombinaciones = nroCombinaciones[0];
        infoForWorkers->nroDeEnvios = nroDeEnvios;
        infoForWorkers->nroDeCombinacionesWorker = args->nroCombinacionesPorWorker;
        infoForWorkers->infoMaps = args->infoMaps;

        return nroCombinaciones;
}

void sendInfoToWorkers(ArgToThread * args, InfoForWorkers * infoForWorkers, InfoForWorkers * infoForWorkerRestante){
        /*se empaquete la información a enviar a cada uno de los workers*/
        /*va a haber nroDeEnvios datos de tipo int más 3 int del infoMaps más 1 int de nroDeEnvios
         * más 1 int de nroDeCombinacionesWorker, más 2 double del infoMaps */
    int i,position,tam_out;
    char * outBuf;
        /*se empaquete y envian datos a todos los workers excepto al último*/
        tam_out = (infoForWorkers->nroDeEnvios + 5)*sizeof(int) + 2*sizeof(double) ;
        outBuf = (char*)malloc(tam_out);
        position = 0;
        MPI_Pack(&(infoForWorkers->nroDeEnvios),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkers->nroDeCombinacionesWorker),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(infoForWorkers->nroCombinaciones,infoForWorkers->nroDeEnvios,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkers->infoMaps->Cols),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkers->infoMaps->Rows),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkers->infoMaps->CellHt),1,MPI_DOUBLE,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkers->infoMaps->CellWd),1,MPI_DOUBLE,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkers->infoMaps->cells),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        for(i = 1 ; i < args->nroDeWorkers; i++){
            MPI_Send(&tam_out,1, MPI_INT,i,TAM_INFO_FOR_WORKER_TAG,MPI_COMM_WORLD);
            MPI_Send(outBuf,position, MPI_PACKED,i,INFO_FOR_WORKER_TAG,MPI_COMM_WORLD);
        }//fin for
        free(outBuf);

        /*se empaquetan y envian datos al último worker*/
        tam_out = (infoForWorkerRestante->nroDeEnvios + 5)*sizeof(int) + 2*sizeof(double) ;
        outBuf = (char*)malloc(tam_out);
        position = 0;
        MPI_Send(&tam_out,1, MPI_INT,i,TAM_INFO_FOR_WORKER_TAG,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkerRestante->nroDeEnvios),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkerRestante->nroDeCombinacionesWorker),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(infoForWorkerRestante->nroCombinaciones,infoForWorkerRestante->nroDeEnvios,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkerRestante->infoMaps->Cols),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkerRestante->infoMaps->Rows),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkerRestante->infoMaps->CellHt),1,MPI_DOUBLE,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkerRestante->infoMaps->CellWd),1,MPI_DOUBLE,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Pack(&(infoForWorkerRestante->infoMaps->cells),1,MPI_INT,outBuf,tam_out,&position,MPI_COMM_WORLD);
        MPI_Send(outBuf,position, MPI_PACKED,i,INFO_FOR_WORKER_TAG,MPI_COMM_WORLD);
        free(outBuf);
}
void * recibirResultados(void * argToThread){
    ArgToThread * args;
    args = (ArgToThread *)argToThread;
    MPI_Status status;
    int indexBuffer=0,np;
    int flag,position;
    char * inBuf;//buffer de entrada
    int tam_buf,tamMap;//tamaño del buffer de entrada
    tam_buf =2*args->infoMaps->cells*sizeof(double);
    tamMap = args->infoMaps->cells;
    inBuf = (char *)malloc(tam_buf);
    /*se realiza un bucle hasta que se hayan obtenidos los resultados de todas las combinaciones*/
    while(indexBuffer < args->nroDeWorkers){
        /*por cada worker se verifica si ya tiene una respuesta para el master*/
        for(np=1;np < args->nproc;np++){
            MPI_Iprobe(np,RESULT_TAG, MPI_COMM_WORLD,&flag, &status);//se verifica si hay nuevos datos enviados por el worker
            if(flag){
                position = 0;
                MPI_Recv(inBuf,tam_buf,MPI_PACKED,np,RESULT_TAG,MPI_COMM_WORLD,&status);
                //mapa de ignición
                pthread_mutex_lock(&(args->varsMC->mutexBufferIgnMap));
                MPI_Unpack(inBuf,tam_buf,&position,args->buffer_ignMap[indexBuffer],tamMap,MPI_DOUBLE,MPI_COMM_WORLD);
                args->varsMC->condicion_buffer_ignMap++;//se indica que se produjo un elemento más en el buffer
                pthread_cond_signal(&(args->varsMC->condcBufferIgnMap));
                pthread_mutex_unlock(&(args->varsMC->mutexBufferIgnMap));

                //mapa de altura de llamas
                pthread_mutex_lock(&(args->varsMC->mutexBufferFlMap));
                MPI_Unpack(inBuf,tam_buf,&position,args->buffer_flMap[indexBuffer],tamMap,MPI_DOUBLE,MPI_COMM_WORLD);
                args->varsMC->condicion_buffer_flMap++;//se invierte la condicion
                pthread_cond_signal(&(args->varsMC->condcBufferFlMap));
                pthread_mutex_unlock(&(args->varsMC->mutexBufferFlMap));
                indexBuffer++;
            }//fin if
        }//fin for
    }//fin while
    free(inBuf);
pthread_exit(0);
}//fin de recibirResultados
/*****************************************************************/
void * sumarResultados(void * argToThread){
    ArgToThread * args;
    args = (ArgToThread *)argToThread;
    double * ignMapFinal;//mapa de ignición final
    double * flMapFinal;//mapa de altura de llamas final
    int indexBuffer = 0;//cantidad de celdas en cada mapa
    int i;
    //los mapas se inicializan todos en cero
    ignMapFinal = (double*)calloc(args->infoMaps->cells,sizeof(double));
    flMapFinal = (double*)calloc(args->infoMaps->cells,sizeof(double));
    while(indexBuffer < args->nroDeWorkers){
        //por el mapa de ignicion
        pthread_mutex_lock(&(args->varsMC->mutexBufferIgnMap));/*obtiene acceso excluiso al buffer*/
        while(args->varsMC->condicion_buffer_ignMap == 0){//mientras no haya nuevos elementos en el buffer
            pthread_cond_wait(&(args->varsMC->condcBufferIgnMap),&(args->varsMC->mutexBufferIgnMap));
        }//fin while
        /*se suman todos los valores en el mapa final ya la ubicación actual
         en el buffer va a ser luego reutilizada*/
        for(i = 0; i < args->infoMaps->cells; i++){
            ignMapFinal[i] += args->buffer_ignMap[indexBuffer][i];
        }//fin for
        args->varsMC->condicion_buffer_ignMap--;//se indica que se ha consumido un valor
        pthread_mutex_unlock(&(args->varsMC->mutexBufferIgnMap));/*libera el acceso al buffer*/

        //por el mapa de altura de llamas
        pthread_mutex_lock(&(args->varsMC->mutexBufferFlMap));/*obtiene acceso excluiso al buffer*/
        while(args->varsMC->condicion_buffer_flMap == 0){//mientras no haya nuevos elementos en el buffer
            pthread_cond_wait(&(args->varsMC->condcBufferFlMap),&(args->varsMC->mutexBufferFlMap));
        }//fin while
        for(i = 0; i < args->infoMaps->cells; i++){
            flMapFinal[i] += args->buffer_flMap[indexBuffer][i];
        }//fin for
        args->varsMC->condicion_buffer_flMap--;//se indica que se ha consumido un valor
        pthread_mutex_unlock(&(args->varsMC->mutexBufferFlMap));/*libera el acceso al buffer*/
        indexBuffer++;
    }//fin while
    /*por cada mapa se calcula un promedio por cada celda*/
    for(i = 0 ; i < args->infoMaps->cells;i++){
        flMapFinal[i] /= args->nroCombinaciones;
        ignMapFinal[i] /= args->nroCombinaciones;
    }//fin for
    
    /*se imprimen los respectivos mapas en los archivos*/
    PrintMap(ignMapFinal,args->incendiosFile,args->infoMaps);
    PrintMap(flMapFinal,args->llamasFile,args->infoMaps);
    free(ignMapFinal);
    free(flMapFinal);
    pthread_exit(0);
}//fin de sumarResultados
/********************************************************/


void PrintMap ( double *map, char * fileName, InfoMaps * infoMaps)
{
    FILE *fPtr;
    int cell, col, row;
    int Cols;
    int Rows;
    double CellWd;
    double CellHt;
    Cols = infoMaps->Cols;
    Rows = infoMaps->Rows;
    CellWd = infoMaps->CellWd;
    CellHt = infoMaps->CellHt;
    if ( (fPtr = fopen(fileName, "w")) == NULL )
    {
        printf("Unable to open output map \"%s\".\n", fileName);
    }

    fprintf(fPtr, "#Dimensiones %d %d %1.3f %1.3F\n\n", Cols, Rows,CellWd, CellHt);
    for ( row=0; row< Rows; row++ )
    {
        for ( cell=row* Cols, col=0; col< Cols; col++, cell++ )
        {
            fprintf(fPtr, " %1.2f", map[cell]);
        }
        fprintf(fPtr, "\n");
    }
    fclose(fPtr);
}
