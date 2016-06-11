
// Maximum number of parallel connections
#define PARALLEL_CONN_CNT 128

// Connection timeout
#define TIMEOUT_MS 1000

// Tries to connect to all ports in a given range and prints which ones succeeded
extern void portScanner(char *addr, char *firstPort, char *lastPort);
