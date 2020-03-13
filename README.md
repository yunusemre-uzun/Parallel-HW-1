# Parallel-HW-1

This code finds the maximum value in array of floats in parallel. The code can be with 1,2,4,8 and 16 processes. 

In order to run:
  1. $make all
  2.  mpiexec -n <number_of_processes> ./hw1.o <input_file> <array_length>
  
Example run with 4 processes:
  mpiexec -n 4 ./hw1.o input.txt 1000000
