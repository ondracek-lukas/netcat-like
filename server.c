// Server part, listens on a given port

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 201112L

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "server.h"
#include "global.h"
#include "dataExchange.h"


void server(char *addr, char *port) {
	if (addr) {
		printLog(0, "Listenning on port %s (%s)...\n", port, addr);
	} else {
		printLog(0, "Listenning on port %s...\n", port);
	}

	struct addrinfo *ais, hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags|=AI_PASSIVE;

	if (getaddrinfo(addr, port, &hints, &ais) != 0) {
		exitErr(2, "Wrong port or no suitable interface exists.");
	}

	int sock=-1, sock2;

	for (int ipv6=1; ipv6>=0; ipv6--) { // two passes: ipv6=true and ipv6=false
		for (struct addrinfo *ai = ais; ai; ai=ai->ai_next) {
			if ((bool)ipv6 ^ (ai->ai_family==AF_INET6)) continue;
			// FOR each ai IN ais ORDERED such that ipv6 appears before all other (ipv4):

			sock=socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
			{ // allow repeated usage of the same address
				int value=1;
				setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
			}
			if (sock==-1) continue;
			if (bind(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
				sock=-1;
				continue;
			}
			if (listen(sock, 1) == -1) {
				sock=-1;
				continue;
			}
			break;

			// END FOR
		}
		if (sock != -1) break;
	}
	freeaddrinfo(ais);

	if (sock == -1) exitErr(2, "Cannot listen on the specified address and port.");

	sock2 = accept(sock, NULL, NULL);
	close(sock);

	if (sock2<0) exitErr(2, "Cannot accept incomming connection.");

	dataExchange(sock2);

	close(sock2);
}
