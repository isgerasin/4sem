#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "CalcArgs.h"


#define MAXCPUS 20
#define COREID "processor	:"
#define PHYSID "core id		:"

#define f(x) (x*x*x +2*x*x-8*x-1)


#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)


typedef struct CpuIds
{
	int phys;
	int core;
}CpuIds;


int maxPhys = 0;
int cpuTable[MAXCPUS][MAXCPUS] = {};

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

	double sum = 0;
	double xEnd =  arg->xEnd;
	double xStart = arg->xStart;
	double dx = ( xEnd - xStart ) / arg->cutNumber;
	for (double i = xStart; i <= xEnd; i += dx)
		sum += f(i) * dx;
	arg->sum = sum;

	return NULL;
}

void* busy(void* args)
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

	for (int i = 0; line; i++)
	{
		int core = -1;
		int phys = -1;

		line = strstr(line, COREID);
		if (line == NULL)
			break;

		sscanf(line + sizeof(COREID), "%i", &core);
		line = strstr(line, PHYSID);
		sscanf(line + sizeof(PHYSID), "%i", &phys);
		
		int j = 0;
		for (j = 0; cpuTable[phys][j] >=0; j++);
		cpuTable[phys][j] = core;

		maxPhys = max(maxPhys, phys);
	}
	maxPhys++;

	return 0;
}

double integrate(double xStart, double xEnd, unsigned long long cuts, long nThreads)
{
	parseCpuinfo();

	CalcArgs* args = (CalcArgs*) calloc(max(nThreads, maxPhys), sizeof(CalcArgs));
	int maxThreads = sysconf(_SC_NPROCESSORS_ONLN);
	
	unsigned long long eachCutNum = cuts / min(nThreads, maxThreads);
	double cutLong = (xEnd - xStart)/ nThreads;

	for (int i = 0; i < nThreads; i++)
	{
		args[i].f=NULL;
		args[i].xStart = xStart + cutLong*i;
		args[i].xEnd = args[i].xStart + cutLong;
		args[i].cutNumber = eachCutNum / (nThreads/maxThreads + (i%maxThreads<nThreads%maxThreads? 1 : 0) );
		args[i].sum = 0;

		_(pthread_attr_getaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
		CPU_ZERO(&args[i].cpuSet);
		CPU_SET(cpuTable[(i%maxThreads)%maxPhys][(i%maxThreads)/maxPhys], &args[i].cpuSet);
		_(pthread_attr_setinheritsched(&args[i].attr, PTHREAD_EXPLICIT_SCHED));
		_(pthread_attr_setaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));

	}

	for (int i = nThreads; i < maxPhys; i++)
	{
		_(pthread_attr_getaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
		CPU_ZERO(&args[i].cpuSet);
		CPU_SET(cpuTable[(i%maxThreads)%maxPhys][(i%maxThreads)/maxPhys], &args[i].cpuSet);
		_(pthread_attr_setinheritsched(&args[i].attr, PTHREAD_EXPLICIT_SCHED));
		_(pthread_attr_setaffinity_np(&args[i].attr, sizeof(args[i].cpuSet), &args[i].cpuSet));
	}

	double sum = 0;
	for (int i = 0; i < nThreads; i++)
		_(pthread_create(&args[i].tid, &args[i].attr, calculate, &args[i]));

	for (int i = nThreads; i < maxPhys; i++)
		_(pthread_create(&args[i].tid, &args[i].attr, busy, &args[i]));
	
	for (int i = 0; i < nThreads; i++)
	{
		_(pthread_join(args[i].tid, NULL));
		sum += args[i].sum;
	}
	return sum;
}

int connectServer(struct sockaddr_in* server, long nThreads)
{
	int fdUdp = 0;
	char msg = 0;
	socklen_t fromLen = sizeof(*server);
	_(fdUdp = socket(AF_INET, SOCK_DGRAM, 0));

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(PORT),
		.sin_addr = htonl(INADDR_ANY)
	};
	_(bind(fdUdp, &addr, sizeof(addr)));
	_(recvfrom(fdUdp, &msg, sizeof(msg), 0, (struct sockaddr*)server, &fromLen));

	printf("server: %c %s\n", msg, inet_ntoa(server->sin_addr));
	int fd = 0;
	// _(fd = socket(PF_INET, SOCK_STREAM, 0));
	// _(listen(fd, 256));

	// _(bind(fd, (struct sockaddr*) &server, sizeof(server)));
	// int sk = 0;
	// _(sk = accept())




	_(sendto(fdUdp, &nThreads, sizeof(nThreads), 0, (struct sockaddr*)server, sizeof(*server)));

	close(fdUdp);
	return fd;
}

int main(int argc, char const *argv[])
{
	double X_START = -2;
	double X_END = 2;
	unsigned long long CUTS = (unsigned long long)360360000ull*10;//*2;

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

	struct sockaddr_in server = {};
	int fd = 0;
	_(fd = connectServer(&server, nThreads));

	// int fd = 0;
	sleep(1);
	// int fd = 0;
	_(fd = socket(PF_INET, SOCK_STREAM, 0));
	_(connect(fd, (struct sockaddr*)&server, sizeof(server)));

	// _(bind(fd, (struct sockaddr*)&server, sizeof(server)));
	// _(listen(fd, 256));
	int sk = 0;
	sk = fd;
	socklen_t len = 0;
	InetCut cut = {};
	PL;
	// _(sk = accept(fd, (struct sockaddr*)&server, &len));
	PL;
	_(read(sk, &cut, sizeof(cut)));
	PL;
	double sum = 0;
	_(sum = integrate(cut.xStart, cut.xEnd, cut.cutNumber, cut.nThreads));
	_(write(sk, &sum, sizeof(sum)));
	PL;
	close(sk);
	close(fd);



	printf("%lg\n", sum);

	return 0;
}
