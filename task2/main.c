#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
/*
#define X_START (-1)
#define X_END (1)
#define CUTS 100
*/
#define PL fprintf(stderr, "%d\n", __LINE__);

typedef struct CalcArgs
{
	int cutNumber;
	double xStart;
	double xEnd;
	double (*f)(double x);
	double sum;
	pthread_t tid;
}CalcArgs;

double X_START = -100;
double X_END = 100;
long long int CUTS = 10000000000;

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

	int eachCutNum = CUTS / nThreads;

	double cutLong = (X_END - X_START)/ nThreads;

	for (int i = 0; i < nThreads; i++)
	{
		args[i].f=f;
		args[i].xStart = X_START + cutLong*i;
		args[i].xEnd = args[i].xStart + cutLong;
		args[i].cutNumber = eachCutNum;
		pthread_create(&args[i].tid, NULL, calculate, args + i);
	}
	// PL;
	double sum = 0;
	for (int i = 0; i < nThreads; i++)
	{
		//fprintf(stderr, "%d |", i);
		pthread_join(args[i].tid, NULL);
		sum += args[i].sum;
	}
	// PL;
	printf("%lg\n", sum);
	return 0;
}