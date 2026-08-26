CC ?= cc
CFLAGS ?= -O2 -g -Wall -Wextra -Werror

all: stcp-v2-bench-server

stcp-v2-bench-server: stcp_v2_bench_server.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f stcp-v2-bench-server
