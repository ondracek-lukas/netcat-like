#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 201112L

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "global.h"
#include "client.h"
#include "server.h"
#include "portScanner.h"



// can be called from any module
void exitErr(int exitcode, char *msg) {
	if (msg) fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "Run without parameters for help.\n");
	exit(exitcode);
}

int verbosity = 0;
void printLog(int level, const char *fmt, ...) {
	if (verbosity < level) return;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
void printOut(int level, const char *fmt, ...) {
	if (verbosity < level) return;
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}


int main(int argc, char **argv) {
	bool serverMode   = false;
	bool scanMode     = false;
	char *localAddr   = NULL;
	char *localPort   = NULL;
	char *remotePort  = NULL; // or possibly range "p1:p2" if scanMode
	char *remoteAddr  = NULL;
	

	// --- HELP MESSAGE ---

	if (argc==1) {
		printf(
			"\n"
			"Usage:\n"
			"  %s [-q] ADDRESS PORT\n"
			"  %s [-q] -lp LOCAL_PORT [-s LOCAL_ADDRESS]\n"
			"  %s [ -q | -v[v] ] -x ADDRESS PORT1[:PORT2]\n"
			"\n"
			"The first variant works as a client connecting to the given address and port.\n"
			"The second as a server listening on the given port.\n"
			"The third tries to connect to the address and port (or port range)\n"
			"and prints opened ports.\n"
			"\n"
			"Argument -v increases verbosity, -q decreases.\n"
			"\n",
			argv[0], argv[0], argv[0]);
		return 0;
	}


	// --- PARSING PARAMETERS ---

	char c;
	while ((c=getopt(argc, argv, "lp:s:xvq"))!=-1) {
		switch (c) {
			case 'l':
				serverMode=true;
				break;
			case 'p':
				localPort=optarg;
				break;
			case 's':
				localAddr=optarg;
				break;
			case 'x':
				scanMode=true;
				break;
			case 'v':
				verbosity++;
				break;
			case 'q':
				verbosity--;
				break;
			default:
				exitErr(1, NULL);
				break;
		}
	}

	if (serverMode && scanMode) {
		exitErr(1, "Cannot listen and scan simultaneously.");
	}

	if (serverMode && !localPort) {
		exitErr(1, "Missing local port for listening on.");
	}

	if (!serverMode) {
		if (argc <= optind+1) {
			exitErr(1, "Missing parameters.");
		}
		remoteAddr = argv[optind++];
		remotePort = argv[optind++];
	}

	if (argc > optind) {
		exitErr(1, "Too much parameters.");
	}
	
	if (!serverMode && localPort) {
		printLog(0, "Warning: Local port parameter is ignored.\n");
	}
	if (!serverMode && localAddr) {
		printLog(0, "Warning: Local address parameter is ignored.\n");
	}
	

	// --- SERVER/CLIENT ---
	
	if (serverMode) {
		server(localAddr, localPort);
	} else if (scanMode) {
		char *port1=remotePort;
		char *port2=strchr(remotePort, ':');
		if (port2) *port2++='\0';
		else        port2=port1;
		portScanner(remoteAddr, port1, port2);
	} else {
		client(remoteAddr, remotePort);
	}

	return 0;
}
