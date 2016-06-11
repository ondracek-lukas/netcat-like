
CFLAGS=-Wall -std=c99 -pthread
LDFLAGS=-pthread
.PHONY: clean clean-tmps

netcat: main.o client.o dataExchange.o server.o
	$(CC) $(LDFLAGS) -o $@ $^ $>
	@# $^ on Linux contains what $> on FreeBSD, the other is blank

clean-tmps:
	rm -f *.o
clean: clean-tmps
	rm -f netcat

