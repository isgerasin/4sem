
#include <pthread.h>

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
	long nThreads;
}CalcArgs;
