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

#include "fireLib.h"

#define INFINITY 999999999.     /* or close enough */

/* NOTE 1: Change these to modify map size & resolution. */
static int    Rows   = 101;     /* Number of rows in each map. */
static int    Cols   = 101;     /* Number of columns in each map. */
static double CellWd = 100.;    /* Cell width (E-W) in feet. */
static double CellHt = 100.;    /* Cell height (N-S) in feet. */

/* NOTE 2: parámetros a tener en cuenta en la simulación
Por cada worker hay que variar estos parámetros, probando todas las combinaciones posibles
*/
static size_t Model   = 1;      /* NFFL 1 */
static double WindSpd = 4.;     /* mph */
static double WindDir = 0.;     /* degrees clockwise from north */
static double Slope   = 0.0;    /* fraction rise / reach */
static double Aspect  = 0.0;    /* degrees clockwise from north */
static double M1      = .10;    /* 1-hr dead fuel moisture */
static double M10     = .10;    /* 10-hr dead fuel moisture */
static double M100    = .10;    /* 100-hr dead fuel moisture */
static double Mherb   = 1.00;   /* Live herbaceous fuel moisture */
static double Mwood   = 1.50;   /* Live woody fuel moisture */

static int PrintMap _ANSI_ARGS_((double *map, char *fileName ));

int main ( int argc, char **argv )
{
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
        ignMap[cell]   = INFINITY; //inicialmente, cada celda nunca se quemará
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
    for ( timeNext=INFINITY, cell=0; cell<cells; cell++ )
    {
        if ( ignMap[cell] < timeNext )
            timeNext = ignMap[cell];
    }

    /* NOTE 8: loop until no more cells can ignite or fire reaches an edge. */
    atEdge = 0;
    while ( timeNext < INFINITY && ! atEdge )
    {
        timeNow  = timeNext;
        timeNext = INFINITY;
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

    printf("There were %d time steps ending at %3.2f minutes (%3.2f hours).\n",
        nTimes, timeNow, timeNow/60.);

    /* NOTE 13: if requested, save the ignition & flame length maps. */
    if ( argc > 1 )
        PrintMap(ignMap, argv[1]);
    if ( argc > 2 )
        PrintMap(flMap, argv[2]);

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
            fprintf(fPtr, " %1.2f", (map[cell]==INFINITY) ? 0.0 : map[cell]);
        }
        fprintf(fPtr, "\n");
    }
    fclose(fPtr);
    return (FIRE_STATUS_OK);
}
