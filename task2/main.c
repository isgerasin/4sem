#define _GNU_SOURCE             /* See feature_test_macros(7) */

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
	double (*f)(double x);
	pthread_t tid;
	pthread_attr_t attr;
	cpu_set_t cpuSet;
}CalcArgs;




typedef struct CpuIds
{
	int phys;
	int core;
}CpuIds;

double X_START = -2;
double X_END = 2;
unsigned long long CUTS = (unsigned long long)360360000ull*10;//*2;
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

void* calculate(void* args)
{
	CalcArgs* arg = (CalcArgs*) args;
	// fprintf(stderr, "%p\n", &arg->tid);
	double sum = 0;
	double xEnd =  arg->xEnd;
	double xStart = arg->xStart;
	double dx = ( xEnd - xStart ) / arg->cutNumber;
	for (double i = xStart; i <= xEnd; i += dx)
		sum += f(i) * dx;
	arg->sum = sum;

	return NULL;
}

void* bisy(void* args)
{
	while(1);
}



int parseCpuinfo()
{
	size_t nread = 0;
	size_t size = 1024 * 20;
	FILE* fl = fopen("/proc/cpuinfo", "r");
	char* file = calloc(size+1, sizeof(char));
	if (file == NULL || fl == NULL)
		exit(EXIT_FAILURE);

	_(nread = fread(file, size+1, 1, fl));
	char* line = file;


	memset(cpuTable, 0xFF, MAXCPUS * MAXCPUS * sizeof(cpuTable[0][0]));

	for (int i = 0; i <= 20; i++)
		cpuPhysCoreMin[i] = 20;

	for (int i = 0; line; i++)
	{
		// PL;
		int core = -1;
		int phys = -1;

		line = strstr(line, COREID);
		if (line == NULL)
			break;
		// puts(line);
		// PL;
		sscanf(line + sizeof(COREID), "%i", &core);
		Idtable[i].core = core;
		line = strstr(line, PHYSID);
		sscanf(line + sizeof(PHYSID), "%i", &phys);
		Idtable[i].phys = phys;
		
		int j = 0;
		for (j = 0; cpuTable[phys][j] >=0; j++);
		cpuTable[phys][j] = core;

		// fprintf(stderr, "%d %d\n", Idtable[i].phys, Idtable[i].core);
		cpuCorePhys[Idtable[i].core] = Idtable[i].phys;

		cpuPhysCoreMax[Idtable[i].phys] = max(cpuPhysCoreMax[Idtable[i].phys], Idtable[i].core);
		cpuPhysCoreMin[Idtable[i].phys] = min(cpuPhysCoreMin[Idtable[i].phys], Idtable[i].core);
		maxPhys = max(maxPhys, Idtable[i].phys);
	}
	maxPhys++;
	// CpuTableDump();
	// for (int i = 0; i <= maxPhys; i++)
		// fprintf(stderr, "max %d min %d\n", cpuPhysCoreMax[i], cpuPhysCoreMin[i]);

	return 0;
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

	parseCpuinfo();

	CalcArgs* args = (CalcArgs*) calloc(max(nThreads, maxPhys), sizeof(CalcArgs));
	int maxThreads = sysconf(_SC_NPROCESSORS_ONLN);
	
	unsigned long long eachCutNum = CUTS / min(nThreads, maxThreads);
	double cutLong = (X_END - X_START)/ nThreads;

	int j = 0;
	for (int i = 0; i < nThreads; i++)
	{
		args[i].f=NULL;
		args[i].xStart = X_START + cutLong*i;
		args[i].xEnd = args[i].xStart + cutLong;
		args[i].cutNumber = eachCutNum / (nThreads/maxThreads + (i%maxThreads<nThreads%maxThreads? 1 : 0) );
		args[i].sum = 0;

/*
		struct sched_param sch = {.sched_priority = PTHREAD_EXPLICIT_SCHED };
		_(pthread_attr_setschedparam(&args[i].attr, &sch));

		_(pthread_attr_setscope(&args[i].attr, PTHREAD_SCOPE_PROCESS));

		_(pthread_attr_setschedpolicy(&args[i].attr, SCHED_RR));
		*/
		

		_(pthread_attr_getaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
		/*
		if (i < maxPhys)
		{
			CPU_ZERO(&args[i].cpuSet);
			CPU_SET(cpuPhysCoreMax[cpuCorePhys[i]] , &args[i].cpuSet);
			//fprintf(stderr, "%d\n", cpuPhysCoreMax[cpuCorePhys[i]]);
		}
		else if (i < maxThreads)
		{
			CPU_ZERO(&args[i].cpuSet);
			CPU_SET(cpuPhysCoreMin[cpuCorePhys[i]], &args[i].cpuSet);
		}*/

		// if (i < maxThreads)
		{
			CPU_ZERO(&args[i].cpuSet);
			CPU_SET(cpuTable[(i%maxThreads)%maxPhys][(i%maxThreads)/maxPhys], &args[i].cpuSet);
		}
		// fprintf(stderr, "i = %d  start = %4lg cuts = %llu cpu = %d\n",i, args[i].xStart, args[i].cutNumber, cpuTable[(i%maxThreads)%maxPhys][(i%maxThreads)/maxPhys]);


		_(pthread_attr_setinheritsched(&args[i].attr, PTHREAD_EXPLICIT_SCHED));
		_(pthread_attr_setaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
		j = i;

	}

	for (int i = nThreads; i < maxPhys; i++)
	{
		_(pthread_attr_getaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
		{
			CPU_ZERO(&args[i].cpuSet);
			CPU_SET(cpuTable[(i%maxThreads)%maxPhys][(i%maxThreads)/maxPhys], &args[i].cpuSet);
		}
		// fprintf(stderr, "i = %d  start = %4lg cuts = %llu cpu = %d\n",i, args[i].xStart, args[i].cutNumber, cpuTable[(i%maxThreads)%maxPhys][(i%maxThreads)/maxPhys]);

		_(pthread_attr_setinheritsched(&args[i].attr, PTHREAD_EXPLICIT_SCHED));
		_(pthread_attr_setaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
	}

	double sum = 0;
	for (int i = 0; i < nThreads; i++)
	{
		// fprintf(stderr, "thread %d\n", i);
		_(pthread_create(&args[i].tid, &args[i].attr, calculate, &args[i]));
		// usleep(100);
		j = i;
	}

	for (int i = nThreads; i < maxPhys; i++)
	{
		// fprintf(stderr, "thread %d\n", i);
		_(pthread_create(&args[i].tid, &args[i].attr, bisy, &args[i]));
		// usleep(100);
	}


	
	for (int i = 0; i < nThreads; i++)
	{
		_(pthread_join(args[i].tid, NULL));
		sum += args[i].sum;
	}

	printf("%lg\n", sum);
	return 0;
}
