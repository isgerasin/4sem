// #define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <cuda.h>
#include <cuda_runtime.h>



#define ALIGN 64
#define MAXCPUS 20
#define COREID "processor	:"
#define PHYSID "core id		:"

#define f(x) (x*x*x +2*x*x-8*x-1)
/*
#define X_START (-1)
#define X_END (1)
#define CUTS 100
*/
#define _(var) do {errno = 0; \
			if ((var)<0) \
				{ perror(#var); return -1;} }while(0)

#define PL fprintf(stderr, "%d\n", __LINE__);

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
typedef struct CalcArgs
{
	double sum;
	unsigned long long cutNumber;
	double xStart;
	double xEnd;
}CalcArgs;



typedef struct CpuIds
{
	int phys;
	int core;
}CpuIds;

double X_START = -2;
double X_END = 2;
unsigned long long CUTS = (unsigned long long)360360000ull;//*10;
int cpuTable[MAXCPUS][MAXCPUS] = {};
int cpuPhysCoreMax[MAXCPUS] = {};
int cpuPhysCoreMin[MAXCPUS] = {};

int cpuCorePhys[MAXCPUS] = {};
CpuIds Idtable[MAXCPUS] = {};
int maxPhys = 0;

int CpuTableDump()
{
	for (int i = 0; i < MAXCPUS; i++)
	{
		printf("%3d:\t", i);

		for (int j = 0; j < MAXCPUS; j++)
			printf("%2d ", cpuTable[i][j]);
		printf("\n");
	}
	return 0;
}

__global__
void calculate(CalcArgs* args)
{
	CalcArgs* arg = args /*+ blockIdx.x * blockDim.x*/ + threadIdx.x;
	// fprintf(stderr, "%p\n", &arg->tid);
	double sum = 0;
	double xEnd =  arg->xEnd;
	double xStart = arg->xStart;
	double dx = ( xEnd - xStart ) / arg->cutNumber;
	for (double i = xStart; i <= xEnd; i += dx)
		sum += f(i) * dx;
	arg->sum = sum;

	return;
}

void* bisy(void* args)
{
	while(1);
}


int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <number of threads(>=1)>\n", argv[0]);
		return EXIT_FAILURE;
	}
	long nThreads = strtol(argv[1], NULL, 10);
	if (nThreads < 1)
	{
		printf("Usage: %s <number of threads(>=1)>\n", argv[0]);
		return EXIT_FAILURE;
	}


	CalcArgs* args = (CalcArgs*) malloc(nThreads * sizeof(CalcArgs));

	//fprintf(stderr, "%p %d %lu, %ld\n", args, err, sizeof(CalcArgs), nThreads);

	int maxThreads = sysconf(_SC_NPROCESSORS_ONLN);

	unsigned long long eachCutNum = CUTS / min(nThreads, maxThreads);
	double cutLong = (X_END - X_START)/ nThreads;
	for (int i = 0; i < nThreads; i++)
	{

		args[i].xStart = X_START + cutLong*i;
		args[i].xEnd = args[i].xStart + cutLong;
		args[i].cutNumber = eachCutNum ;// (nThreads/maxThreads + (i%maxThreads<nThreads%maxThreads? 1 : 0) );
		args[i].sum = 0;
		

	}
	double sum = 0;
	cudaSetDevice(0);

	CalcArgs* argsH = NULL;
	cudaMalloc(&argsH, nThreads * sizeof(CalcArgs));
	cudaMemcpy(argsH, args, nThreads * sizeof(CalcArgs), cudaMemcpyHostToDevice);
	calculate<<<1, nThreads>>> (argsH);
	cudaMemcpy(args, argsH, nThreads * sizeof(CalcArgs), cudaMemcpyDeviceToHost);
	
	cudaDeviceSynchronize();

	for (int i = 0; i < nThreads; i++)
	{
		//_(pthread_join(args[i].tid, NULL));
		sum += args[i].sum;
	}

	printf("%lg\n", sum);

	return 0;
}
