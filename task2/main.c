#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>


/*
#define X_START (-1)
#define X_END (1)
#define CUTS 100
*/
#define _(var) do {errno = 0; \
			if ((var)!=0) \
				{ perror(#var); return -1;} }while(0)

#define PL fprintf(stderr, "%d\n", __LINE__);

typedef struct CalcArgs
{
	unsigned long long cutNumber;
	double xStart;
	double xEnd;
	double (*f)(double x);
	double sum;
	pthread_t tid;
	pthread_attr_t attr;
	cpu_set_t cpuSet;
}CalcArgs;

double X_START = -100;
double X_END = 100;
unsigned long long CUTS = (unsigned long long)360360000*7;

double f(double x)
{
	return x*x*x +2*x*x-8*x-1;
}

void* calculate(void* args)
{
	CalcArgs* arg = (CalcArgs*) args;
	arg->sum = 0;
	double dx = ( arg->xEnd - arg->xStart ) / arg->cutNumber;
	for (double i = arg->xStart; i <= arg->xEnd; i += dx)
	{
		// fprintf(stderr, "%lg | %lg | %lg\n", i, (arg->f)(i), dx);
		arg->sum += (arg->f)(i) * dx;
	}
	// PL;
	// fprintf(stderr, "%lg \n", arg->xStart);

	return NULL;
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
	CalcArgs* args = (CalcArgs*) calloc(nThreads, sizeof(CalcArgs));
	if (!args)
		return EXIT_FAILURE;

	unsigned long long eachCutNum = CUTS / nThreads;
	int maxThreads = sysconf(_SC_NPROCESSORS_CONF);
	double cutLong = (X_END - X_START)/ nThreads;

	for (int i = 0; i < nThreads; i++)
	{
		args[i].f=f;
		args[i].xStart = X_START + cutLong*i;
		args[i].xEnd = args[i].xStart + cutLong;
		args[i].cutNumber = eachCutNum;
		_(pthread_attr_getaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
		if (i < maxThreads)
		{
			CPU_ZERO(&args[i].cpuSet);
			CPU_SET(i % maxThreads, &args[i].cpuSet);
		}
		_(pthread_attr_setinheritsched(&args[i].attr, PTHREAD_EXPLICIT_SCHED));
		_(pthread_attr_setaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));

	}

	for (int i = 0; i < nThreads; i++)
		_(pthread_create(&args[i].tid, &args[i].attr, calculate, args + i));

	// PL;
	double sum = 0;
	for (int i = 0; i < nThreads; i++)
	{
		//fprintf(stderr, "%d |", i);
		_(pthread_join(args[i].tid, NULL));
		sum += args[i].sum;
	}
	// PL;
	printf("%lg\n", sum);
	return 0;
}
