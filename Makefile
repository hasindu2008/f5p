CC       = gcc
CFLAGS  += -g -rdynamic -Wall -O2
LDFLAGS += -lpthread 

DEPS = socket.h error.h f5pmisc.h

.PHONY: clean distclean format test

all: f5pd f5pl f5pl_test
	
f5pd : socket.c f5pd.c error.c $(DEPS)
	$(CC) $(CFLAGS) socket.c f5pd.c error.c $(LDFLAGS) -o $@

f5pl : socket.c f5pl.c error.c $(DEPS)
	$(CC) $(CFLAGS) socket.c f5pl.c error.c $(LDFLAGS) -o $@

f5pl_test : socket.c f5pl_test.c error.c $(DEPS)
	$(CC) $(CFLAGS) socket.c f5pl_test.c error.c $(LDFLAGS) -o $@
	
clean:
	rm -rf f5pd f5pl f5pl_test *.o *.out *.cfg

# Autoformat code with clang format
format:
	./scripts/autoformat.sh	

test: all
		./f5pd &
		./f5pl data/ip_localhost.cfg data/file_list.cfg
		pkill f5pd
		
rsync: all
		make clean
		rsync -rv --delete . rock64@129.94.14.121:~/f5p
		ssh rock64@129.94.14.121 'rsync -rv --delete ~/f5p/* rock1:~/f5p'