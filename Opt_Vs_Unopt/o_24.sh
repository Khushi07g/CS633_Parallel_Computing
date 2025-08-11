#!/bin/bash

#SBATCH -N 1
#SBATCH --ntasks-per-node=8
#SBATCH --error=./logs/job.%J.err
#SBATCH --output=./logs/job.%J.out
#SBATCH --time=00:10:00         ## wall-clock time limit        
#SBATCH --partition=standard    ## can be "standard" or "cpu"

echo `date`

mpirun -np 64 ./a binary 4 4 4 256 256 256 24 output_256_256_256_24_64_OPT.txt

echo `date`
