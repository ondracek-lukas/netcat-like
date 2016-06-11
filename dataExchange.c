// Data exchange between a given opened socked and stdin/stdout

#include <unistd.h>
#include <pthread.h>

#include "dataExchange.h"
#include "errorHandling.h"

static void *incommingPrinter(void *sockPtr);
static void *inputSender(void *sockPtr);

void dataExchange(int sock) {
	// maybe a little better throughput than using poll or select
	pthread_t inSenderThr;
	pthread_create(&inSenderThr, NULL, inputSender, &sock);

	incommingPrinter(&sock);

	// pthread_join(inSenderThr, NULL); // exit immediately instead
	pthread_cancel(inSenderThr);
}


static void *incommingPrinter(void *sockPtr) {
	int sock = *(int *)sockPtr;
	char buf[BUFFER_SIZE];
	int size;
	while ((size=read(sock, buf, BUFFER_SIZE)) > 0) {
		if (write(1, buf, size) != size) {
			size=-1;
			break;
		};
	}
	if (size<0) exitErr(3, "Error occurred while receiving data.");

	return NULL;
}

static void *inputSender(void *sockPtr) {
	int sock = *(int *)sockPtr;
	char buf[BUFFER_SIZE];
	int size;
	while ((size=read(0, buf, BUFFER_SIZE)) > 0) {
		if (write(sock, buf, size) != size) {
			size=-1;
			break;
		};
	}
	if (size<0) exitErr(3, "Error occurred while sending data.");

	return NULL;
}
