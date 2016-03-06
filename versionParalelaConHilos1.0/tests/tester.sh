#!/bin/bash
#simuladorParallel: app name of which performs parallel algorith
echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8 
mpirun -np 9 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 100

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 9 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 200

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 9 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 300

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 8 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 100

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 8 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 300

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 8 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 300

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 7 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 100

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 7 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 200

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 7 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 300

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 6 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 100

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 6 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 200

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 6 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 300

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 5 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 100

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 5 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 200

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 5 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 300

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 4 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 100

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 4 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 200

echo algorith parallel with threads, version 1.0, packet size: 100 messages, number of nodes: 9, number of workers: 8
mpirun -np 4 ./simuladorParallel tamMaps.txt parametros.txt incendios.txt altllamas.txt 300
