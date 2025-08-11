#!/bin/bash

#SBATCH -N 1
#SBATCH --ntasks-per-node=8
#SBATCH --error=./logs/job.%J.err
#SBATCH --output=./logs/job.%J.out
#SBATCH --time=00:10:00         ## wall-clock time limit        
#SBATCH --partition=standard    ## can be "standard" or "cpu"

echo `date`

mpirun -np 64 ./a kk.bin 4 4 4 128 128 128 24 output_128_128_128_24_64_UNOPT.txt
mpirun -np 64 ./a kk.bin 4 4 4 128 128 64 24 output_128_128_64_24_64_UNOPT.txt
mpirun -np 64 ./a kk.bin 4 4 4 128 64 64 24 output_128_64_64_24_64_UNOPT.txt
mpirun -np 64 ./a kk.bin 4 4 4 64 64 64 24 output_64_64_64_24_64_UNOPT.txt

echo `date`
