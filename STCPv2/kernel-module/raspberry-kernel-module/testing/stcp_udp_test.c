#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define AF_STCP 45
#define STCP_PROTO_UDP 254

static void make_addr(struct sockaddr_in *addr)
{
	memset(addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(7778);
	inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr);
}

static int run_server(int clients)
{
	struct sockaddr_in addr;
	int listener;
	int i;

	make_addr(&addr);
	listener = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_UDP);
	if (listener < 0) { perror("udp socket"); return 1; }
	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("udp bind"); return 1;
	}
	if (listen(listener, 32) < 0) { perror("udp listen"); return 1; }

	for (i = 0; i < clients; ++i) {
		char buffer[256];
		int fd = accept(listener, NULL, NULL);
		ssize_t n;
		if (fd < 0) { perror("udp accept"); return 1; }
		n = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (n < 0) { perror("udp recv"); return 1; }
		buffer[n] = '\0';
		printf("udp server received: %s\n", buffer);
		if (send(fd, buffer, (size_t)n, 0) != n) {
			perror("udp send"); return 1;
		}
		close(fd);
	}
	close(listener);
	return 0;
}

static int run_client(const char *message)
{
	struct sockaddr_in addr;
	char buffer[256];
	int fd;
	ssize_t n;

	make_addr(&addr);
	fd = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_UDP);
	if (fd < 0) { perror("udp socket"); return 1; }
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("udp connect"); return 1;
	}
	if (send(fd, message, strlen(message), 0) != (ssize_t)strlen(message)) {
		perror("udp send"); return 1;
	}
	n = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (n < 0) { perror("udp recv"); return 1; }
	buffer[n] = '\0';
	if (strcmp(buffer, message) != 0) {
		fprintf(stderr, "bad UDP reply: %s\n", buffer);
		return 1;
	}
	printf("udp client verified: %s\n", buffer);
	close(fd);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc >= 2 && !strcmp(argv[1], "server"))
		return run_server(argc >= 3 ? atoi(argv[2]) : 1);
	if (argc >= 2 && !strcmp(argv[1], "client"))
		return run_client(argc >= 3 ? argv[2] : "udp hello");
	fprintf(stderr, "usage: %s server [clients] | client [message]\n", argv[0]);
	return 2;
}
