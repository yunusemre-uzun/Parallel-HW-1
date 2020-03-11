all: hw1.c
	mpicc hw1.c -o hw1.o
run1:
	mpiexec -n 1 ./hw1.o input.txt 1000000
run2:
	mpiexec -n 2 ./hw1.o input.txt 1000000
run4:
	mpiexec -n 4 ./hw1.o input.txt 1000000
run8:
	mpiexec -n 8 ./hw1.o input.txt 1000000
run16:
	mpiexec -n 16 ./hw1.o input.txt 1000000
