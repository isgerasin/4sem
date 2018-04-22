
#include <pthread.h>

#define PORT 4001

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


typedef struct InetCut
{
	double sum;
	unsigned long long cutNumber;
	double xStart;
	double xEnd;
	long nThreads;
} InetCut;

//http://rsdn.org/?article/unix/sockets.xml