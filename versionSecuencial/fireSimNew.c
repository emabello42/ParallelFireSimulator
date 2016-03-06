/*
 *******************************************************************************
 *
 *  fireSim.c
 *
 *  Legalities
 *      Copyright (c) 1996 Collin D. Bevins.
 *      See the file "license.txt" for information on usage and redistribution
 *      of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 *******************************************************************************
 */
#include <time.h>
#include "fireLib.h"

#define INFINITO 999999999.     /* or close enough */
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
/* NOTE 1: Change these to modify map size & resolution. */
static int    Rows;     /* Number of rows in each map. */
static int    Cols;     /* Number of columns in each map. */
static double CellWd;    /* Cell width (E-W) in feet. */
static double CellHt;    /* Cell height (N-S) in feet. */

/* NOTE 2: parámetros a tener en cuenta en la simulación
Por cada worker hay que variar estos parámetros, probando todas las combinaciones posibles
*/
static size_t Model;      /* NFFL 1 */
static double WindSpd;     /* mph */
static double WindDir;     /* degrees clockwise from north */
static double Slope ;    /* fraction rise / reach */
static double Aspect;    /* degrees clockwise from north */
static double M1 ;    /* 1-hr dead fuel moisture */
static double M10;    /* 10-hr dead fuel moisture */
static double M100;    /* 100-hr dead fuel moisture */
static double Mherb;   /* Live herbaceous fuel moisture */
static double Mwood;   /* Live woody fuel moisture */

static int PrintMap _ANSI_ARGS_((double *map, char *fileName ));

int main ( int argc, char **argv )
{clock_t a, b;
a = clock();
    FILE *fmaps;
        fmaps = fopen(argv[1],"r");
        fscanf(fmaps,"Rows\t=\t%d\n",&(Rows));//se lee la cantidad de filas
        fscanf(fmaps,"Cols\t=\t%d\n",&(Cols));// se lee la cantidad de columnas
        fscanf(fmaps,"CellWd\t=\t%lf\n",&(CellWd)); //se lee el ancho de celda
        fscanf(fmaps,"CellHt\t=\t%lf\n",&(CellHt)); //se lee el alto de celda
        fclose(fmaps);
        
        InfoParam * infoParam;
    int j,dataInt;
    double dataDouble;
    FILE *fparam;
    infoParam = (InfoParam *)malloc(sizeof(InfoParam));
     /*se leen los parámetros*/
        fparam = fopen(argv[2],"r");
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
    
    double * ignMapFinal;//mapa de ignición final
    double * flMapFinal;//mapa de altura de llamas final
        
        
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

    /* NOTE 3: se asigna espacio para almacenar todos los mapas 
    Cada worker debería tener su propia copia por cada mapa
    */
    cells = Rows * Cols;
    int a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,i,combinaciones;    
    ignMapFinal = (double*)calloc(cells,sizeof(double));
    flMapFinal = (double*)calloc(cells,sizeof(double));
       combinaciones = 0 ;//empieza el conteo de combinaciones
        for(a1 = 0; a1 < infoParam->cantModel ;a1++){
         Model = infoParam->Model[a1];
         for(a2=0; a2 < infoParam->cantAspect ;a2++){
          Aspect = infoParam->Aspect[a2];
          for(a3=0; a3 <infoParam->cantWindSpd;a3++){
           WindSpd = infoParam->WindSpd[a3];
           for(a4=0;a4< infoParam->cantMherb;a4++){
           Mherb = infoParam->Mherb[a4];
            for(a5=0;a5 < infoParam->cantWindDir;a5++){
            WindDir = infoParam->WindDir[a5];
             for(a6=0;a6< infoParam->cantSlope ;a6++){
             Slope = infoParam->Slope[a6];
              for(a7=0;a7 < infoParam->cantMwood;a7++){
              Mwood = infoParam->Mwood[a7];
               for(a8=0;a8< infoParam->cantM100 ;a8++){
               M100 = infoParam->M100[a8];
                for(a9=0;a9< infoParam->cantM10 ;a9++){
                M10 = infoParam->M10[a9];
                 for(a10=0; a10< infoParam->cantM1 ;a10++){
                 M1 = infoParam->M1[a10];    
 combinaciones++;
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
        return (1);
    }

    /* NOTE 4: initialize all the maps -- modify them as you please. */
    for ( cell=0; cell<cells; cell++ )
    {
        fuelMap[cell]  = Model;
        slpMap[cell]   = Slope;
        aspMap[cell]   = Aspect;
        wspdMap[cell]  = 88. * WindSpd;     /* convert mph into ft/min */
        wdirMap[cell]  = WindDir;
        m1Map[cell]    = M1;
        m10Map[cell]   = M10;
        m100Map[cell]  = M100;
        mherbMap[cell] = Mherb;
        mwoodMap[cell] = Mwood;
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

  //  printf("There were %d time steps ending at %3.2f minutes (%3.2f hours).\n",
    //    nTimes, timeNow, timeNow/60.);
     
     /*se armar el mapa binario de ignición*/
    for ( row=0; row < Rows; row++ )
    {
        for ( cell=row*Cols, col=0; col <  Cols; col++, cell++ )
        {
            ignMap[cell] = ignMap[cell]==INFINITO ? 0.0 : 1.0;
        }//fin for
    }//fin for

    /*se arma el mapa de alturas de llamas*/
    for ( row=0; row < Rows; row++ )
    {
        for ( cell=row*Cols, col=0; col <  Cols; col++, cell++ )
        {
            flMap[cell] = flMap[cell]==INFINITO ? 0.0 : flMap[cell];
        }//fin for
    }//fin for
        for(i = 0; i < cells; i++){
            ignMapFinal[i] += ignMap[i];
        }//fin for
     for(i = 0; i < cells; i++){
            flMapFinal[i] += flMap[i];
        }//fin for
      free(ignMap);
      free(flMap);
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
    for(i = 0; i < cells; i++){
            ignMapFinal[i] /= combinaciones;
        }//fin for
     for(i = 0; i < cells; i++){
            flMapFinal[i] /= combinaciones;
        }//fin for
       
    /* NOTE 13: if requested, save the ignition & flame length maps. */
        PrintMap(ignMapFinal, argv[3]);
        PrintMap(flMapFinal, argv[4]);
b = clock();
printf("tiempo transcurrido = %g seconds\n",(double)(b-a)/(double)CLOCKS_PER_SEC);
    return (0);
}

static int PrintMap ( map, fileName )
    double *map;
    char   *fileName;
{
    FILE *fPtr;
    int cell, col, row;

    if ( (fPtr = fopen(fileName, "w")) == NULL )
    {
        printf("Unable to open output map \"%s\".\n", fileName);
        return (FIRE_STATUS_ERROR);
    }

    fprintf(fPtr, "#Dimensiones %d %d %1.3f %1.3F\n\n", Cols, Rows, CellWd, CellHt);
    for ( row=0; row<Rows; row++ )
    {
        for ( cell=row*Cols, col=0; col<Cols; col++, cell++ )
        {
            fprintf(fPtr, " %1.2f", map[cell]);
        }
        fprintf(fPtr, "\n");
    }
    fclose(fPtr);
    return (FIRE_STATUS_OK);
}
