#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef AF_STCP
#define AF_STCP 45
#endif

#define STCP_PROTO_ID 253

static int make_addr(struct sockaddr_in *addr)
{
	memset(addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(7777);

	if (inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr) != 1)
		return -1;

	return 0;
}

static int run_server(void)
{
	struct sockaddr_in addr;
	char buffer[256];
	int listener;
	int client;
	ssize_t n;

	if (make_addr(&addr) < 0)
		return 1;

	listener = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_ID);
	if (listener < 0) {
		perror("socket");
		return 1;
	}

	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(listener, 16) < 0) {
		perror("listen");
		return 1;
	}

	printf("server: waiting in accept()\n");

	client = accept(listener, NULL, NULL);
	if (client < 0) {
		perror("accept");
		return 1;
	}

	n = recv(client, buffer, sizeof(buffer) - 1, 0);
	if (n < 0) {
		perror("recv");
		return 1;
	}

	buffer[n] = '\0';
	printf("server received: %s\n", buffer);

	if (send(client, "reply from kernel STCP", 22, 0) < 0) {
		perror("send");
		return 1;
	}

	close(client);
	close(listener);
	return 0;
}

static int run_client(void)
{
	struct sockaddr_in addr;
	char buffer[256];
	int fd;
	ssize_t n;

	if (make_addr(&addr) < 0)
		return 1;

	fd = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_ID);
	if (fd < 0) {
		perror("socket");
		return 1;
	}

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		return 1;
	}

	if (send(fd, "hello through BSD API", 21, 0) < 0) {
		perror("send");
		return 1;
	}

	n = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (n < 0) {
		perror("recv");
		return 1;
	}

	buffer[n] = '\0';
	printf("client received: %s\n", buffer);

	close(fd);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s server|client\n", argv[0]);
		return 2;
	}

	if (!strcmp(argv[1], "server"))
		return run_server();

	if (!strcmp(argv[1], "client"))
		return run_client();

	fprintf(stderr, "unknown mode: %s\n", argv[1]);
	return 2;
}
