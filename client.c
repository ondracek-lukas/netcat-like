// Client part, connects to a given port

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 201112L

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "client.h"
#include "errorHandling.h"
#include "dataExchange.h"

void client(char *addr, char *port) {
	fprintf(stderr, "Connecting to %s on port %s...\n", addr, port);
	
	struct addrinfo *ais, hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;

	if (getaddrinfo(addr, port, &hints, &ais) != 0) {
		exitErr(2, "Wrong address/port or network error.");
	}

	int sock=-1;

	for (struct addrinfo *ai = ais; ai; ai=ai->ai_next) {
		sock=socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock==-1) continue;
		if (connect(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
			sock=-1;
			continue;
		}
		break;
	}
	freeaddrinfo(ais);

	if (sock == -1) exitErr(2, "Cannot connect to the host.");

	dataExchange(sock);

	close(sock);
}
