CC ?= cc
CFLAGS ?= -O2 -g -Wall -Wextra -Wpedantic
LDFLAGS ?=

all: stcp-bench-server

stcp-bench-server: bench_server.c
	$(CC) $(CFLAGS) -pthread -o $@ $< $(LDFLAGS)

clean:
	rm -f stcp-bench-server

.PHONY: all clean
