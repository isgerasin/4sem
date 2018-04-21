#include <stdio.h>
#include <stdlib.h>
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
} ClientId;

int connectClients(ClientId* clients, long nCalc)
{
	int fdUdp = 0;
	_(fdUdp = socket(AF_INET, SOCK_DGRAM, 0));

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(4000),
		.sin_addr = htonl(INADDR_BROADCAST)
	};

	int tm = 1;
	_(setsockopt(fdUdp, SOL_SOCKET, SO_BROADCAST, &tm, sizeof(tm)));

	char msg = 's';
	_(sendto(fdUdp, &msg, sizeof(msg), 0, (struct sockaddr*)&addr, sizeof(addr)));
	int nClients = 0;
	int residue = 0;
	alarm(5);
	for (residue = nCalc; residue > 0; nClients++)
	{
		_(recvfrom(fdUdp, &clients[nClients].nThreads, sizeof(clients[nClients].nThreads), 0, (struct sockaddr*)&clients[nClients].addr, &clients[nClients].addrLen));
		residue -= clients[nClients].nThreads;
		printf("%d\n", clients[nClients].nThreads);
	}
	alarm(0);
	clients[nClients].nThreads -= residue;
	close(fdUdp);
	return nClients;
}

int main(int argc, char const *argv[])
{
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

	return 0;
}