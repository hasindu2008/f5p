CC       = gcc
CFLAGS  += -g -rdynamic -Wall -O2
LDFLAGS += -lpthread 

DEPS = socket.h 

.PHONY: clean distclean format test

all: f5p_daemon client
	
f5p_daemon : socket.c f5p_daemon.c $(DEPS)
	$(CC) $(CFLAGS) socket.c f5p_daemon.c $(LDFLAGS) -o $@

client : socket.c client.c $(DEPS)
	$(CC) $(CFLAGS) socket.c client.c $(LDFLAGS) -o $@


clean:
	rm -rf f5p_daemon client *.o *.out

# Autoformat code with clang format
format:
	./scripts/autoformat.sh	