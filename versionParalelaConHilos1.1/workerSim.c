#include "workerSim.h"
#define INFINITO 999999999.     /* or close enough */


void codigoWorker(int my_id){
    MPI_Status status;
    /*El worker se encarga de realizar la simulación por cada combinación de parámetros recibida
     Una vez terminada la recepción de un mensaje, se dedica a procesar una simulación por cada combinación
     recibida en el mensaje. Además una vez efectuada la simulación debe generar dos matrices resultados,
     una correspondiente al mapa de incendios, el cual es un mapa binario que contiene un uno en cada celda
     donde se sabe que va a haber incendio, es decir, que el tiempo que tardará el fuego en llegar a dicha zona
     es distinta de infinito, y la otra correspondiente a la altura de las llamas en cada celda*/
    /*Para realizar todos su trabajo, el nodo worker tiene:
     * un hilo receptor de mensajes
     * un hilo que se encarga de realizar las simulaciones
     * un hilo que se encarga de enviar todos los resultados al master
     */
    pthread_t * receiver;//hilo receptor de mensajes
    pthread_t * simulator;//hilo simulador
    pthread_t * sender;//hilo enviador de mensajes
    /*se define y crea el struct con información a la que accederan todos los hilos en el worker*/
    ArgToThreadW * argToThread;
    argToThread = (ArgToThreadW *)malloc(sizeof(ArgToThreadW));
    argToThread->my_id = my_id;
    int tam_in ;
    char * buf_in;
    void * return_value;
    int estado, position = 0;//estado devuelto durante la creacion de un hilo
        /*primero se recibe la cantidad de combinaciones con que tiene que tratar este worker*/
    MPI_Recv(&tam_in,1,MPI_INT,NODO_MASTER,TAM_INFO_FOR_WORKER_TAG,MPI_COMM_WORLD,&status);
    if(tam_in > 0){//si es cero se termina inmediatamente la ejecución de este worker
        /*se recibe información previa enviada por el master*/
        buf_in = (char*)malloc(tam_in);
        MPI_Recv(buf_in,tam_in,MPI_PACKED,NODO_MASTER,INFO_FOR_WORKER_TAG,MPI_COMM_WORLD,&status);
        MPI_Unpack(buf_in,tam_in,&position,&(argToThread->nroDeEnviosARecibir),1,MPI_INT,MPI_COMM_WORLD);
        MPI_Unpack(buf_in,tam_in,&position,&(argToThread->my_nroCombinaciones),1,MPI_INT,MPI_COMM_WORLD);
        argToThread->nroCombinaciones = (int *)malloc(argToThread->nroDeEnviosARecibir*sizeof(int));
        MPI_Unpack(buf_in,tam_in,&position,argToThread->nroCombinaciones,argToThread->nroDeEnviosARecibir,MPI_INT,MPI_COMM_WORLD);
        argToThread->infoMaps = (InfoMaps*)malloc(sizeof(InfoMaps));
        MPI_Unpack(buf_in,tam_in,&position,&(argToThread->infoMaps->Cols),1,MPI_INT,MPI_COMM_WORLD);
        MPI_Unpack(buf_in,tam_in,&position,&(argToThread->infoMaps->Rows),1,MPI_INT,MPI_COMM_WORLD);
        MPI_Unpack(buf_in,tam_in,&position,&(argToThread->infoMaps->CellHt),1,MPI_DOUBLE,MPI_COMM_WORLD);
        MPI_Unpack(buf_in,tam_in,&position,&(argToThread->infoMaps->CellWd),1,MPI_DOUBLE,MPI_COMM_WORLD);
        MPI_Unpack(buf_in,tam_in,&position,&(argToThread->infoMaps->cells),1,MPI_INT,MPI_COMM_WORLD);
        
        sender = (pthread_t *)malloc(sizeof(pthread_t));
        receiver = (pthread_t *)malloc(sizeof(pthread_t));
        simulator = (pthread_t *)malloc(sizeof(pthread_t));
        /*se asigna espacio a los bufferes usados por los hilos*/
        argToThread->bufferDeEntrada = (ParamValues*)malloc(argToThread->my_nroCombinaciones*sizeof(ParamValues));//buffer de entrada para simulator
        /*bufferes de salida para simulator*/
        argToThread->buffer_flMap = (double **)malloc(argToThread->my_nroCombinaciones*sizeof(double*));
        argToThread->buffer_ignMap = (double **)malloc(argToThread->my_nroCombinaciones*sizeof(double*));
        argToThread->varsMC = (VarsMC*)malloc(sizeof(VarsMC));
        /*se inicializan variables de condición y los mutex*/
        pthread_mutex_init(&(argToThread->varsMC->mutex_in),NULL);
        pthread_mutex_init(&(argToThread->varsMC->mutex_out_ignMap),NULL);
        pthread_mutex_init(&(argToThread->varsMC->mutex_out_flMap),NULL);
        pthread_cond_init(&(argToThread->varsMC->condc_in),0);
        pthread_cond_init(&(argToThread->varsMC->condc_out_ignMap),0);
        pthread_cond_init(&(argToThread->varsMC->condc_out_flMap),0);
        argToThread->varsMC->condicionEntrada = 0;
        argToThread->varsMC->condicion_buffer_flMap = 0 ;
        argToThread->varsMC->condicion_buffer_ignMap = 0 ;
        estado = pthread_create(receiver,NULL,recibirTrabajos,(void *)argToThread);
        if(estado){
                printf("error al crear hilo receiver\n");
                exit(-1);
        }//fin if

        estado = pthread_create(simulator,NULL,simular,(void *)argToThread);
        if(estado){
                printf("error al crear hilo simulator\n");
                exit(-1);
        }//fin if

        estado = pthread_create(sender,NULL,enviarResultados,(void *)argToThread);
        if(estado){
                printf("error al crear hilo sender\n");
                exit(-1);
        }//fin if
        
        /*se espera a que terminen de ejecutarse los tres hilos*/
        if(pthread_join(*receiver, &return_value)) printf("error al terminar el hilo receiver\n");
        if(pthread_join(*simulator, &return_value)) printf("error al terminar el hilo simulator\n");
        if(pthread_join(*sender, &return_value)) printf("error al terminar el hilo sender\n");
        //se destruyen las variables de condicion
        pthread_cond_destroy(&(argToThread->varsMC->condc_in));
        pthread_cond_destroy(&(argToThread->varsMC->condc_out_ignMap));
        pthread_cond_destroy(&(argToThread->varsMC->condc_out_flMap));
        pthread_mutex_destroy(&(argToThread->varsMC->mutex_in));
        pthread_mutex_destroy(&(argToThread->varsMC->mutex_out_ignMap));
        pthread_mutex_destroy(&(argToThread->varsMC->mutex_out_flMap));
        free(receiver);
        free(simulator);
        free(sender);
        /*se libera el espacio asignado a argToThread*/
        free(argToThread->infoMaps);
        free(argToThread->varsMC);
        free(argToThread->bufferDeEntrada);
        free(argToThread->buffer_ignMap);
        free(argToThread->buffer_flMap);
    }//fin if
    MPI_Finalize();
}//fin codigoWorker
/**********************************************************************************************************/

void * recibirTrabajos(void * argToThread){
    ArgToThreadW * args;
    args = (ArgToThreadW *)argToThread;
    /*este es el código que ejecuta el hilo receptor de los mensajes que contienen las combinaciones de parámetros*/
    /*este hilo debe desenpaquetar cada uno de los paquetes recibidos, y por cada combinación encontrada crear un
     hilo que se encargue de ejecutar la simulación con dichos parámetros*/
    MPI_Status status;
    int tam_buf;
    int position,i;
    char * inBuf;//buffer de entrada
    int indexBuffer = 0;
    /*se prepara para recibir cada paquete*/
    i = 0;
    while(args->nroDeEnviosARecibir > 0){
            tam_buf = args->nroCombinaciones[i]*NRO_PARAMETROS*sizeof(double);
            inBuf = (char*)malloc(tam_buf);//asigna el espacio suficiente para recibir todos los ParamValues
            MPI_Recv(inBuf,tam_buf,MPI_PACKED,NODO_MASTER,VALORES_TAG,MPI_COMM_WORLD,&status);
            position = 0;
            while(args->nroCombinaciones[i] > 0 ){
                pthread_mutex_lock(&(args->varsMC->mutex_in));/*obtiene acceso exclusivo al buffer de entrada*/
                MPI_Unpack(inBuf,tam_buf,&position,&(args->bufferDeEntrada[indexBuffer]),NRO_PARAMETROS,MPI_DOUBLE,MPI_COMM_WORLD);
                args->varsMC->condicionEntrada++;
                pthread_cond_signal(&(args->varsMC->condc_in));/*desoierta al consumidor*/
                pthread_mutex_unlock(&(args->varsMC->mutex_in));/*libera el acceso al buffer*/
                args->nroCombinaciones[i]--;
                indexBuffer++;
            }//fin while
            args->nroDeEnviosARecibir--;
            i++;
            free(inBuf);
    }//fin while
    pthread_exit(0);
}//fin de receptorDeCombinaciones
/******************************************************************************/
void * simular(void * argToThread){
    ArgToThreadW * args;
    args = (ArgToThreadW *)argToThread;
/* NOTE 2: parámetros a tener en cuenta en la simulación
Por cada worker hay que variar estos parámetros, probando todas las combinaciones posibles
*/
int nroCombinaciones = args->my_nroCombinaciones;//se hace una copia para evitar inconsistencias
    int Rows = args->infoMaps->Rows;
    int Cols = args->infoMaps->Cols;
    double CellWd = args->infoMaps->CellWd;
    double CellHt = args->infoMaps->CellHt;
    /* neighbor's address*/     /* N  NE   E  SE   S  SW   W  NW */
    static int nCol[8] =        {  0,  1,  1,  1,  0, -1, -1, -1};
    static int nRow[8] =        {  1,  1,  0, -1, -1, -1,  0,  1};
    static int nTimes = 0;      /* counter for number of time steps */
    FuelCatalogPtr catalog;     /* fuel catalog handle */
    double nDist[8];            /* distance to each neighbor */
    double nAzm[8];             /* compass azimuth to each neighbor (0=N) */
    double timeNow;             /* current time (minutes) */
    double timeNext;            /* time of next cell ignition (minutes) */
    int    row, col, cell;      /* row, col, and index of current cell */
    int    nrow, ncol, ncell;   /* row, col, and index of neighbor cell */
    int    n, cells;            /* neighbor index, total number of map cells */
    size_t modelNumber;         /* fuel model number at current cell */
    double moisture[6];         /* fuel moisture content at current cell */
    double fpm;                 /* spread rate in direction of neighbor */
    double minutes;             /* time to spread from cell to neighbor */
    double ignTime;             /* time neighbor is ignited by current cell */
    int    atEdge;              /* flag indicating fire has reached edge */
    //ESTOS SON LOS MAPAS
    size_t *fuelMap;            /* ptr al mapa del modelo de combustible */
    double *ignMap;             /* ptr al mapa de tiempo de encendido  (minutes) */
    double *flMap;              /* ptr al mapa de altura de las llamas (en pies) */
    double *slpMap;             /* ptr al mapa de pendientes (subida/alcance) */
    double *aspMap;             /* ptr al mapa de aspecto (grados desde el norte) */
    double *wspdMap;            /* ptr al mapa de velocidad del viento (ft/min) */
    double *wdirMap;            /* ptr al mapa de dirección del viento (grados desde el norte) */
    double *m1Map;              /* ptr de 1-hr mapa muertos humedad del combustible */
    double *m10Map;             /* ptr a 10 h. mapa muertos humedad del combustible*/
    double *m100Map;            /* ptr 100-hr mapa muertos humedad del combustible */
    double *mherbMap;           /* ptr to live herbaceous fuel moisture map */
    double *mwoodMap;           /* ptr to live stem fuel moisture map */
    int indexBuffer = 0; //indice usado para recorrer el buffer de entrada y los de salida
    /* NOTE 3: se asigna espacio para almacenar todos los mapas
    Cada worker debería tener su propia copia por cada mapa
    */
    cells = Rows * Cols;
    while(nroCombinaciones){//mientras haya combinaciones de parámetros con los cuales simular
        if ( (ignMap   = (double *) calloc(cells, sizeof(double))) == NULL
      || (flMap    = (double *) calloc(cells, sizeof(double))) == NULL
      || (slpMap   = (double *) calloc(cells, sizeof(double))) == NULL
      || (aspMap   = (double *) calloc(cells, sizeof(double))) == NULL
      || (wspdMap  = (double *) calloc(cells, sizeof(double))) == NULL
      || (wdirMap  = (double *) calloc(cells, sizeof(double))) == NULL
      || (m1Map    = (double *) calloc(cells, sizeof(double))) == NULL
      || (m10Map   = (double *) calloc(cells, sizeof(double))) == NULL
      || (m100Map  = (double *) calloc(cells, sizeof(double))) == NULL
      || (mherbMap = (double *) calloc(cells, sizeof(double))) == NULL
      || (mwoodMap = (double *) calloc(cells, sizeof(double))) == NULL
      || (fuelMap  = (size_t *) calloc(cells, sizeof(size_t))) == NULL )
    {
        fprintf(stderr, "Unable to allocate maps with %d cols and %d rows.\n",
            Cols, Rows);
        exit(-1);
    }
        pthread_mutex_lock(&(args->varsMC->mutex_in));/*obtiene acceso excluiso al buffer*/
        while(args->varsMC->condicionEntrada == 0){//mientras la condicion de entrada no se cumpla
            pthread_cond_wait(&(args->varsMC->condc_in),&(args->varsMC->mutex_in));
        }//fin while
        args->varsMC->condicionEntrada--;//disminuye el numero de elementos en el buffer disponibles
        pthread_mutex_unlock(&(args->varsMC->mutex_in));/*libera el acceso al buffer*/
    /* NOTE 4: initialize all the maps -- modify them as you please. */
    /*se cargan los valores de la combinacion dada*/

    for ( cell=0; cell<cells; cell++ )
    {
        fuelMap[cell]  = (size_t)args->bufferDeEntrada[indexBuffer].Model;
        slpMap[cell]   = args->bufferDeEntrada[indexBuffer].Slope;
        aspMap[cell]   = args->bufferDeEntrada[indexBuffer].Aspect;
        wspdMap[cell]  = 88. * args->bufferDeEntrada[indexBuffer].WindSpd;     /* convert mph into ft/min */
        wdirMap[cell]  = args->bufferDeEntrada[indexBuffer].WindDir;
        m1Map[cell]    = args->bufferDeEntrada[indexBuffer].M1;
        m10Map[cell]   = args->bufferDeEntrada[indexBuffer].M10;
        m100Map[cell]  = args->bufferDeEntrada[indexBuffer].M100;
        mherbMap[cell] = args->bufferDeEntrada[indexBuffer].Mherb;
        mwoodMap[cell] = args->bufferDeEntrada[indexBuffer].Mwood;
        ignMap[cell]   = INFINITO; //inicialmente, cada celda nunca se quemará
        flMap[cell]    = 0.;
    }

    /* NOTE 5: set an ignition time & pattern (this ignites the middle cell). */
    cell = Cols/2 + Cols*(Rows/4);
    ignMap[cell] = 1.0;

    /* NOTE 6: create a standard fuel model catalog and a flame length table. */
    catalog = Fire_FuelCatalogCreateStandard("Standard", 13);
    Fire_FlameLengthTable(catalog, 500, 0.1);

    /* Calculate distance across cell to each neighbor and its azimuth. */
    for ( n=0; n<8; n++ )
    {
        nDist[n] = sqrt( nCol[n] * CellWd * nCol[n] * CellWd
                        + nRow[n] * CellHt * nRow[n] * CellHt );
        nAzm[n] = n * 45.;
    }

    /* NOTE 7: find the earliest (starting) ignition time. */
    for ( timeNext=INFINITO, cell=0; cell<cells; cell++ )
    {
        if ( ignMap[cell] < timeNext )
            timeNext = ignMap[cell];
    }

    /* NOTE 8: loop until no more cells can ignite or fire reaches an edge. */
    atEdge = 0;
    while ( timeNext < INFINITO && ! atEdge )
    {
        timeNow  = timeNext;
        timeNext = INFINITO;
        nTimes++;

        /* NOTE 9: examine each ignited cell in the fuel array. */
        for ( cell=0, row=0; row<Rows; row++ )
        {
            for ( col=0; col<Cols; col++, cell++ )
            {
                /* Skip this cell if it has not ignited. */
                if ( ignMap[cell] > timeNow )
                {
                    /* NOTE 12: first check if it is the next cell to ignite. */
                    if ( ignMap[cell] < timeNext )
                        timeNext = ignMap[cell];
                    continue;
                }

                /* NOTE 10: flag if the fire has reached the array edge. */
                if ( row==0 || row==Rows-1 || col==0 || col==Cols-1 )
                    atEdge = 1;

                /* NOTE 11: determine basic fire behavior within this cell. */
                modelNumber = fuelMap[cell];
                moisture[0] = m1Map[cell];
                moisture[1] = m10Map[cell];
                moisture[2] = m100Map[cell];
                moisture[3] = m100Map[cell];
                moisture[4] = mherbMap[cell];
                moisture[5] = mwoodMap[cell];
                Fire_SpreadNoWindNoSlope(catalog, modelNumber, moisture);
                Fire_SpreadWindSlopeMax(catalog, modelNumber, wspdMap[cell],
                    wdirMap[cell], slpMap[cell], aspMap[cell]);

                /* NOTE 12: examine each unignited neighbor. */
                for ( n=0; n<8; n++ )
                {
                    /* First find the neighbor's location. */
                    nrow = row + nRow[n];
                    ncol = col + nCol[n];
                    if ( nrow<0 || nrow>=Rows || ncol<0 || ncol>=Cols )
                        continue;
                    ncell = ncol + nrow*Cols;

                    /* Skip this neighbor if it is already ignited. */
                    if ( ignMap[ncell] <= timeNow )
                        continue;

                    /* Determine time to spread to this neighbor. */
                    Fire_SpreadAtAzimuth(catalog, modelNumber, nAzm[n], FIRE_NONE);
                    if ( (fpm = Fuel_SpreadAny(catalog, modelNumber)) > Smidgen)
                    {
                        minutes = nDist[n] / fpm;

                        /* Assign neighbor the earliest ignition time. */
                        if ( (ignTime = timeNow + minutes) < ignMap[ncell] )
                        {
                            ignMap[ncell] = ignTime;
                            Fire_FlameScorch(catalog, modelNumber, FIRE_FLAME);
                            flMap[ncell] = Fuel_FlameLength(catalog,modelNumber);
                        }

                        /* Keep track of next cell ignition time. */
                        if ( ignTime < timeNext )
                            timeNext = ignTime;
                    }
                }   /* next neighbor n */
            }   /* next source col */
        }   /* next source row */
    } /* next time */

   // printf("There were %d time steps ending at %3.2f minutes (%3.2f hours).\n",
     //   nTimes, timeNow, timeNow/60.);
    
    /* NOTE 13: if requested, save the ignition & flame length maps. */
/*se libera el espacio usado*/
      free(slpMap);
      free(aspMap);
      free(wspdMap);
      free(wdirMap);
      free(m1Map);
      free(m10Map);
      free(m100Map);
      free(mherbMap);
      free(mwoodMap);
      free(fuelMap);
    /*se producen los resultados*/
    //mapa de ignición
    pthread_mutex_lock(&(args->varsMC->mutex_out_ignMap));
    args->buffer_ignMap[indexBuffer] = ignMap;
    args->varsMC->condicion_buffer_ignMap++;//se invierte la condicion
    pthread_cond_signal(&(args->varsMC->condc_out_ignMap));
    pthread_mutex_unlock(&(args->varsMC->mutex_out_ignMap));

    //mapa de altura de llamas
    pthread_mutex_lock(&(args->varsMC->mutex_out_flMap));
    args->buffer_flMap[indexBuffer] = flMap;
    args->varsMC->condicion_buffer_flMap++;//se invierte la condicion
    pthread_cond_signal(&(args->varsMC->condc_out_flMap));
    pthread_mutex_unlock(&(args->varsMC->mutex_out_flMap));
    nroCombinaciones--;
    indexBuffer++;//se pasas al siguiente elemento en el buffer, si es que esta disponible
    }//fin while
    pthread_exit(0);
}//fin de simular
/*****************************************************************************/
void * enviarResultados(void * argToThread){
    ArgToThreadW * args;
    args = (ArgToThreadW *)argToThread;
    int nroCombinaciones  = args->my_nroCombinaciones;
    int indexBuffer = 0;//indice para los bufferes de salida
    int cell, col, row;
    char * out_buff;
    int tam_out, position;
    tam_out = 2*args->infoMaps->cells*sizeof(double);/*el buffer de salida tiene que tener el espacio
                                           * suficiente para almacenar dos mapas*/
    out_buff = (char*)malloc(tam_out);//se asigna espacio al buffer de salida
    /*se crean dos mapas que contendrán los resultados parciales (NO CALCULA PROMEDIO, SOLO SUMA),
     * respecto al worker actual*/
    double * ignMapParcial;
    double * flMapParcial;
    ignMapParcial = (double *)calloc(args->infoMaps->cells,sizeof(double));
    flMapParcial = (double *)calloc(args->infoMaps->cells,sizeof(double));
    while(nroCombinaciones){
        //por el mapa de ignicion
        pthread_mutex_lock(&(args->varsMC->mutex_out_ignMap));
        while(args->varsMC->condicion_buffer_ignMap == 0){//mientras no haya nuevos elementos en el buffer
            pthread_cond_wait(&(args->varsMC->condc_out_ignMap),&(args->varsMC->mutex_out_ignMap));
        }//fin while
        args->varsMC->condicion_buffer_ignMap--;//invierte la condicion
        pthread_mutex_unlock(&(args->varsMC->mutex_out_ignMap));

        //por el mapa de llamas
        pthread_mutex_lock(&(args->varsMC->mutex_out_flMap));
        while(args->varsMC->condicion_buffer_flMap == 0){
            pthread_cond_wait(&(args->varsMC->condc_out_flMap),&(args->varsMC->mutex_out_flMap));
        }//fin while
        args->varsMC->condicion_buffer_flMap--;//invierte la condicion
        pthread_mutex_unlock(&(args->varsMC->mutex_out_flMap));
       
        /*una vez que se sabe que están listos los valores a enviar se preparan los resultados a enviar*/
    /*se armar el mapa binario de ignición*/
    for ( row=0; row < args->infoMaps->Rows; row++ )
    {
        for ( cell=row*args->infoMaps->Cols, col=0; col <  args->infoMaps->Cols; col++, cell++ )
        {
            ignMapParcial[cell] += ((args->buffer_ignMap[indexBuffer][cell]==INFINITO) ? 0.0 : 1.0);
        }//fin for
    }//fin for

    /*se arma el mapa de alturas de llamas*/
    for ( row=0; row < args->infoMaps->Rows; row++ )
    {
        for ( cell=row*args->infoMaps->Cols, col=0; col <  args->infoMaps->Cols; col++, cell++ )
        {
            flMapParcial[cell] += ((args->buffer_flMap[indexBuffer][cell]==INFINITO) ? 0.0 : args->buffer_flMap[indexBuffer][cell]);
        }//fin for
    }//fin for

    /*se empaquetan ambas matrices en el paquete*/
    
    free(args->buffer_ignMap[indexBuffer]);
    free(args->buffer_flMap[indexBuffer]);
    indexBuffer++;
    nroCombinaciones--;
    }//fin while
    position = 0 ;
    /*se envia un solo paquete al master con los dos mapas con resultados parciales, correspondientes
     a todas las simulaciones hechas por el worker*/
    MPI_Pack(ignMapParcial,args->infoMaps->cells,MPI_DOUBLE,out_buff,tam_out,&position,MPI_COMM_WORLD);
    MPI_Pack(flMapParcial,args->infoMaps->cells,MPI_DOUBLE,out_buff,tam_out,&position,MPI_COMM_WORLD);
    MPI_Send(out_buff,position, MPI_PACKED,NODO_MASTER,RESULT_TAG,MPI_COMM_WORLD);
    free(out_buff);
    free(ignMapParcial);
    free(flMapParcial);
    pthread_exit(0);
}//fin de enviarResultados
