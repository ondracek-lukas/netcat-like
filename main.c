#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 201112L

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "errorHandling.h"
#include "client.h"
#include "server.h"



// can be called from any module
void exitErr(int exitcode, char *msg) {
	if (msg) fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "Run without parameters for help.\n");
	exit(exitcode);
}



int main(int argc, char **argv) {
	bool serverMode  = false;
	char *localAddr  = NULL;
	char *localPort  = NULL;
	char *remotePort = NULL;
	char *remoteAddr = NULL;
	

	// --- HELP MESSAGE ---

	if (argc==1) {
		printf(
			"Usage:\n"
			"  %s ADDRESS PORT\n"
			"  %s -lp LOCAL_PORT [-s LOCAL_ADDRESS]\n"
			"The former works as a client connecting to the given address and port,\n"
			"the latter as a server listening on the given port.\n",
			argv[0], argv[0]);
		return 0;
	}


	// --- PARSING PARAMETERS ---

	char c;
	while ((c=getopt(argc, argv, "lp:s:"))!=-1) {
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
			default:
				exitErr(1, NULL);
				break;
		}
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
		printf("Warning: Local port parameter is ignored.\n");
	}
	if (!serverMode && localAddr) {
		printf("Warning: Local address parameter is ignored.\n");
	}
	

	// --- SERVER/CLIENT ---
	
	if (serverMode) {
		server(localAddr, localPort);
	} else {
		client(remoteAddr, remotePort);
	}

	return 0;
}
