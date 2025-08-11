#!/bin/bash

#SBATCH -N 1
#SBATCH --ntasks-per-node=8
#SBATCH --error=./logs/job.%J.err
#SBATCH --output=./logs/job.%J.out
#SBATCH --time=00:10:00		## wall-clock time limit	
#SBATCH --partition=standard 	## can be "standard" or "cpu"

echo `date`

mpirun -np 2 ./a kk.bin 2 1 1 64 64 64 3 output_64_64_64_3_2.txt
mpirun -np 4 ./a kk.bin 2 2 1 64 64 64 3 output_64_64_64_3_4.txt
mpirun -np 8 ./a kk.bin 2 2 2 64 64 64 3 output_64_64_64_3_8.txt
mpirun -np 16 ./a kk.bin 4 2 2 64 64 64 3 output_64_64_64_3_16.txt
mpirun -np 32 ./a kk.bin 4 4 2 64 64 64 3 output_64_64_64_3_32.txt
mpirun -np 64 ./a kk.bin 4 4 4 64 64 64 3 output_64_64_64_3_64.txt

mpirun -np 2 ./a kk.bin 2 1 1 64 64 64 6 output_64_64_64_6_2.txt
mpirun -np 4 ./a kk.bin 2 2 1 64 64 64 6 output_64_64_64_6_4.txt
mpirun -np 8 ./a kk.bin 2 2 2 64 64 64 6 output_64_64_64_6_8.txt
mpirun -np 16 ./a kk.bin 4 2 2 64 64 64 6 output_64_64_64_6_16.txt
mpirun -np 32 ./a kk.bin 4 4 2 64 64 64 6 output_64_64_64_6_32.txt
mpirun -np 64 ./a kk.bin 4 4 4 64 64 64 6 output_64_64_64_6_64.txt

echo `date`

