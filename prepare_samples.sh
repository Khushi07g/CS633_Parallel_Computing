#!/bin/bash

mkdir Sample_Test_Cases
cd Sample_Test_Cases

mkdir Run1 Run2 Run3 Run4

mpicc ../src.c -o a

cp ../data_64_64_64_3.bin ../data_64_64_96_7.bin ../sample1.sh ../sample2.sh ./a Run1
cp ../data_64_64_64_3.bin ../data_64_64_96_7.bin ../sample1.sh ../sample2.sh ./a Run2
cp ../data_64_64_64_3.bin ../data_64_64_96_7.bin ../sample1.sh ../sample2.sh ./a Run3
cp ../data_64_64_64_3.bin ../data_64_64_96_7.bin ../sample1.sh ../sample2.sh ./a Run4
