#!/bin/bash

#SBATCH -N 1
#SBATCH --ntasks-per-node=8
#SBATCH --error=./logs/job.%J.err
#SBATCH --output=./logs/job.%J.out
#SBATCH --time=00:10:00         ## wall-clock time limit        
#SBATCH --partition=standard    ## can be "standard" or "cpu"

echo `date`

mpirun -np 2 ./a kk.bin 2 1 1 512 256 256 24 output_512_256_256_24_2.txt
mpirun -np 4 ./a kk.bin 2 2 1 512 256 256 24 output_512_256_256_24_4.txt
mpirun -np 8 ./a kk.bin 2 2 2 512 256 256 24 output_512_256_256_24_8.txt
mpirun -np 16 ./a kk.bin 4 2 2 512 256 256 24 output_512_256_256_24_16.txt
mpirun -np 32 ./a kk.bin 4 4 2 512 256 256 24 output_512_256_256_24_32.txt
mpirun -np 64 ./a kk.bin 4 4 4 512 256 256 24 output_512_256_256_24_64.txt


echo `date`
