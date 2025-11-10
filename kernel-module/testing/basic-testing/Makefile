CC = gcc
CFLAGS = -O2 -Wall -Wextra -Werror -pedantic
LDFLAGS =

all: stcp-openclose stcp-mt stcp-connect

stcp-openclose: stcp_openclose.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

stcp-mt: stcp_mt.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

stcp-connect: stcp_connect.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f stcp_connect stcp_mt stcp_openclose
