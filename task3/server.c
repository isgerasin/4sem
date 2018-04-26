#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "debug.h"
#include "CalcArgs.h"


typedef struct ClientId
{
	struct sockaddr_in addr;
	int nThreads;
	unsigned int addrLen;
	int sk;
	int fd;
} ClientId;

int connectClients(ClientId* clients, long nCalc)
{
	int fdUdp = 0;
	_(fdUdp = socket(AF_INET, SOCK_DGRAM, 0));

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(PORTUDP),
		.sin_addr = htonl(INADDR_BROADCAST)
	};

	int tm = 1;
	_(setsockopt(fdUdp, SOL_SOCKET, SO_BROADCAST, &tm, sizeof(tm)));

	char msgC = 's';
	
	int nClients = 0;
	int residue = 0;
	ssize_t ret = 0;
	alarm(10);

	for (residue = nCalc; residue > 0; )
	{
		_(sendto(fdUdp, &msgC, sizeof(msgC), 0, (struct sockaddr*)&addr, sizeof(addr)));

		Msg msg = {};
		errno = 0;
		clients[nClients].addrLen = sizeof(clients[nClients].addr);
		ret = recvfrom(fdUdp, &msg,
			sizeof(msg), MSG_DONTWAIT, (struct sockaddr*)&clients[nClients].addr, &clients[nClients].addrLen);

		if ( ret == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
			return -1;
		else if (ret != -1)
		{
			clients[nClients].nThreads = msg.nThreads;
			clients[nClients].addr.sin_port =  msg.port;
			residue -= clients[nClients].nThreads;
			nClients++;
		}
	}
	// PL;
	alarm(0);
	clients[nClients-1].nThreads += residue;

	close(fdUdp);
	return nClients;
}

int main(int argc, char const *argv[])
{
	double X_START = -2;
	double X_END = 2;
	unsigned long long CUTS = (unsigned long long)360360000ull*10;//*2;

	if (argc != 2)
	{
		printf("Usage: %s <number of calculators(>=1)>\n", argv[0]);
		return EXIT_FAILURE;
	}
	long nCalc = strtol(argv[1], NULL, 10);
	if (nCalc < 1)
	{
		printf("Usage: %s <number of calculators(>=1)>\n", argv[0]);
		return EXIT_FAILURE;
	}

	ClientId* clients = calloc(nCalc, sizeof(*clients));
	int nClients = 0;

	_(nClients = connectClients(clients, nCalc));
	// printf("nClients = %d\n", nClients);

	InetCut* cuts = calloc(nClients, sizeof(*cuts));

	unsigned long long eachCutNum = CUTS / nCalc;
	double cutLong = (X_END - X_START)/ nCalc;

	double curXStart = X_START;
	for (int i = 0; i < nClients; i++)
	{
		cuts[i].nThreads = clients[i].nThreads;
		cuts[i].xStart = curXStart;
		cuts[i].xEnd = curXStart + cuts[i].nThreads*cutLong;
		curXStart = cuts[i].xEnd;
		cuts[i].cutNumber = cuts[i].nThreads*eachCutNum;

		// printf("i %d Threads %li Cuts %llu \n", i, cuts[i].nThreads, cuts[i].cutNumber );

		_(clients[i].fd = socket(PF_INET, SOCK_STREAM, 0));
		_(connect(clients[i].fd, (struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr)));
	}

	for (int i = 0; i < nClients; i++)
	{
		size_t len = 0;
		_(len = write(clients[i].fd, cuts + i, sizeof(*cuts)));
	}
	
	double sum = 0;
	alarm(90);
	for (int i = 0; i < nClients; i++)
	{
		size_t len = 0;
		_(len = read(clients[i].fd, &cuts[i].sum, sizeof(cuts[i].sum)));
		sum += cuts[i].sum;
		close(clients[i].fd);
	}
	alarm(0);
	printf("%lg\n", sum);

	return 0;
}