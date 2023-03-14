build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c cFunctions.c -o cFunctions.o
	mpicxx -fopenmp -c mpiHelper.c -o mpiHelper.o
	nvcc -I./inc -c cudaFunctions.cu -o cudaFunctions.o
	mpicxx -fopenmp -o exec main.o cFunctions.o mpiHelper.o cudaFunctions.o  /usr/local/cuda-11.0/lib64/libcudart_static.a -ldl -lrt

clean:
	rm -f *.o ./exec

run:
	mpiexec -np 2 ./exec < input.txt

runOn2:
	mpiexec -np 2 -machinefile  mf  -map-by  node  ./exec

