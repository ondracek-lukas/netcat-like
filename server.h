
// Listens on a given addr and port,
// accepts the first incomming connection
// and connects it to stdin/stdout.
// The address can be NULL for all interfaces.
extern void server(char *addr, char *port);
