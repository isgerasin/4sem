
#include <pthread.h>

#define PORTUDP 1080
#define PORTTCP ((PORTUDP)/* + 4*/)

typedef struct Msg
{
	long nThreads;
	unsigned short int port;
} Msg;

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
} CalcArgs;


typedef struct InetCut
{
	double sum;
	unsigned long long cutNumber;
	double xStart;
	double xEnd;
	long nThreads;
} InetCut;

//http://rsdn.org/?article/unix/sockets.xml