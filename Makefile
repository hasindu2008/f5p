CC       = gcc
CFLAGS  += -g -rdynamic -Wall -O2
LDFLAGS += -lpthread 

DEPS = socket.h error.h f5pmisc.h

.PHONY: clean distclean format test

all: f5p_daemon f5p_launch f5p_launch_test
	
f5p_daemon : socket.c f5p_daemon.c error.c $(DEPS)
	$(CC) $(CFLAGS) socket.c f5p_daemon.c error.c $(LDFLAGS) -o $@

f5p_launch : socket.c f5p_launch.c error.c $(DEPS)
	$(CC) $(CFLAGS) socket.c f5p_launch.c error.c $(LDFLAGS) -o $@

f5p_launch_test : socket.c f5p_launch_test.c error.c $(DEPS)
	$(CC) $(CFLAGS) socket.c f5p_launch_test.c error.c $(LDFLAGS) -o $@
	
clean:
	rm -rf f5p_daemon f5p_launch f5p_launch_test *.o *.out *.cfg

# Autoformat code with clang format
format:
	./scripts/autoformat.sh	

test: all
		./f5p_daemon &
		./f5p_launch data/ip_localhost.cfg data/file_list.cfg
		pkill f5p_daemon
		
rsync: all
		make clean
		rsync -rv . rock64@129.94.14.121:~/f5p
		ssh rock64@129.94.14.121 'rsync -rv ~/f5p/* rock1:~/f5p'