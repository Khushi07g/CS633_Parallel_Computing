#!/bin/bash

#SBATCH -N 1
#SBATCH --ntasks-per-node=8
#SBATCH --error=./logs/job.%J.err
#SBATCH --output=./logs/job.%J.out
#SBATCH --time=00:10:00		## wall-clock time limit	
#SBATCH --partition=standard 	## can be "standard" or "cpu"

echo `date`

mpirun -np 8 ./a data_64_64_96_7.bin 2 2 2 64 64 96 7 output_64_64_96_7_8.txt
mpirun -np 16 ./a data_64_64_96_7.bin 4 2 2 64 64 96 7 output_64_64_96_7_16.txt
mpirun -np 32 ./a data_64_64_96_7.bin 4 4 2 64 64 96 7 output_64_64_96_7_32.txt
mpirun -np 64 ./a data_64_64_96_7.bin 4 4 4 64 64 96 7 output_64_64_96_7_64.txt

echo `date`
