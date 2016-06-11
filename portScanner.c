// Scanning open ports of a remote host

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 201112L

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>

#include "portScanner.h"
#include "global.h"

static int resolveService(char *name);
static void scanInit();
static void scanPort(struct addrinfo *ai, in_port_t port);
static int waitAndProcessResults();

void portScanner(char *addr, char *firstPort, char *lastPort) {
	printLog(0, "Scanning ports of %s in range %s to %s...\n", addr, firstPort, lastPort);
	int port1, port2;
	port1=resolveService(firstPort);
	port2=resolveService(lastPort);
	if (port2<port1) exitErr(2, "Wrong range, first port higher than the other.");

	struct addrinfo *ais, hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_STREAM;

	if (getaddrinfo(addr, NULL, &hints, &ais) != 0) {
		exitErr(2, "Wrong address or network error.");
	}

	scanInit();
	for (struct addrinfo *ai = ais; ai; ai=ai->ai_next) {
		for (int port=port1; port<=port2; port++) {
			scanPort(ai, port);
		}
	}
	while(waitAndProcessResults()>0);

	freeaddrinfo(ais);
}

int resolveService(char *name) {
	char *end;
	int port;
	port=strtol(name, &end, 0);
	if ((*end=='\0') && (port>0)) {
		return port;
	}

	struct servent *service;
	service=getservbyname(name, "tcp");
	if (service != NULL) {
		return ntohs(service->s_port);
	}

	exitErr(2, "Wrong port");
	return 0; // unreachable
}



// --- PORT SCANNING itself ---

struct pollfd fds[PARALLEL_CONN_CNT];
struct {
	struct addrinfo *ai;
	int port;
	struct timespec time;
} fdsInfo[PARALLEL_CONN_CNT];

void scanInit() {
	for (int i = 0; i<PARALLEL_CONN_CNT; i++) {
		fds[i].fd=-1; // free slot, ignored by poll
		fds[i].events=POLLOUT;
	}
}

void printResult(int level, const char *result, struct addrinfo *ai, int port) {
	char addr[128];
	if (getnameinfo(ai->ai_addr, ai->ai_addrlen, addr, 128, NULL, 0, NI_NUMERICHOST) == 0) {
		printOut(level, "%6d  %-10s (%s)\n", port, result, addr);
	} else {
		printOut(level, "%6d  %-10s\n", port, result);
	}
}

void scanPort(struct addrinfo *ai, in_port_t port) {
	static int fdsIndex=0;

	// find free fds slot (wait if no one available)
	while (fds[fdsIndex].fd >= 0) {
		printLog(2, "Seeking free slot...\n");
		for (int i = (fdsIndex+1)%PARALLEL_CONN_CNT; i != fdsIndex; i = (i+1)%PARALLEL_CONN_CNT) {
			if (fds[i].fd < 0) {
				fdsIndex=i;
				break;
			}
		}
		if (fds[fdsIndex].fd >= 0) {
			waitAndProcessResults();
		}
	}

	// adjust port in sockaddr
	if (ai->ai_family == AF_INET6) {
		struct sockaddr_in6 *sa = (struct sockaddr_in6*)ai->ai_addr;
		sa->sin6_port = htons(port);
	} else { // this branch should probably work even with IPv6..?
		struct sockaddr_in *sa = (struct sockaddr_in*)ai->ai_addr;
		sa->sin_port = htons(port);
	}

	// begin connecting
	int sock=socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (sock==-1) return;
	fcntl(sock, F_SETFL, O_NONBLOCK);
	errno=0;
	printLog(2, "Connecting to port %d...\n", port);
	if (connect(sock, ai->ai_addr, ai->ai_addrlen) == 0) {
		printResult(INT_MIN, "Connected", ai, port);
		close(sock);
		return;
	} else if (errno != EINPROGRESS) {
		printResult(1, "Error", ai, port);
		return;
	}

	// save information about socket
	printLog(2, "  adding to table of busy sockets...\n", port);
	fds[fdsIndex].fd=sock;
	fds[fdsIndex].revents=0;
	fdsInfo[fdsIndex].ai=ai;
	fdsInfo[fdsIndex].port=port;
	clock_gettime(CLOCK_MONOTONIC, &fdsInfo[fdsIndex].time);
}

int waitAndProcessResults() {
	int busySockets=0;

	// Is there any socket to wait for?
	for (int i=0; i<PARALLEL_CONN_CNT; i++) {
		if (fds[i].fd >= 0) {
			busySockets++;
		}
	}
	if (busySockets==0) return 0;

	// Wait
	printLog(2, "Waiting...\n");
	poll(fds, PARALLEL_CONN_CNT, TIMEOUT_MS);

	// Process results
	printLog(2, "Processing results...\n");
	struct timespec time; clock_gettime(CLOCK_MONOTONIC, &time);
	busySockets=0;
	for (int i=0; i<PARALLEL_CONN_CNT; i++) {
		if (fds[i].fd < 0) continue;
		if (fds[i].revents) {
			if ((fds[i].revents & POLLOUT) && !(fds[i].revents & POLLERR)) {
				printResult(INT_MIN, "Connected", fdsInfo[i].ai, fdsInfo[i].port);
				close(fds[i].fd);
			} else {
				printResult(1, "Error", fdsInfo[i].ai, fdsInfo[i].port);
			}
			fds[i].fd=-1;
		} else if (
				(time.tv_sec-fdsInfo[i].time.tv_sec)*1000
				+ (time.tv_nsec-fdsInfo[i].time.tv_nsec)/1000000
				> TIMEOUT_MS) {
			printResult(1, "Timeout", fdsInfo[i].ai, fdsInfo[i].port);
			fds[i].fd=-1;
		} else {
			busySockets++;
		}
	}

	return busySockets;
}


