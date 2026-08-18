#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define AF_STCP 45
#define STCP_PROTO_UDP 254
#define TEST_SIZE (256 * 1024)

static void make_addr(struct sockaddr_in *addr)
{
	memset(addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(7780);
	inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr);
}

static unsigned char expected_byte(size_t index)
{
	return (unsigned char)((index * 131u + 17u) & 0xffu);
}

static int run_server(void)
{
	struct sockaddr_in addr;
	unsigned char buffer[8192];
	size_t received = 0;
	int listener, fd;

	make_addr(&addr);
	listener = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_UDP);
	if (listener < 0) { perror("reliability socket"); return 1; }
	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("reliability bind"); return 1;
	}
	if (listen(listener, 16) < 0) { perror("reliability listen"); return 1; }
	fd = accept(listener, NULL, NULL);
	if (fd < 0) { perror("reliability accept"); return 1; }

	while (received < TEST_SIZE) {
		ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
		size_t i;
		if (n < 0) { perror("reliability recv"); return 1; }
		if (n == 0) { fprintf(stderr, "unexpected EOF at %zu\n", received); return 1; }
		for (i = 0; i < (size_t)n; ++i) {
			if (buffer[i] != expected_byte(received + i)) {
				fprintf(stderr, "data mismatch at %zu\n", received + i);
				return 1;
			}
		}
		received += (size_t)n;
	}

	if (send(fd, "OK", 2, MSG_NOSIGNAL) != 2) {
		perror("reliability reply"); return 1;
	}
	printf("udp reliability server verified %zu bytes\n", received);
	close(fd); close(listener);
	return 0;
}

static int run_client(void)
{
	struct sockaddr_in addr;
	unsigned char *data;
	char reply[2];
	size_t sent = 0, i;
	int fd;

	make_addr(&addr);
	data = malloc(TEST_SIZE);
	if (!data) { perror("malloc"); return 1; }
	for (i = 0; i < TEST_SIZE; ++i) data[i] = expected_byte(i);

	fd = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_UDP);
	if (fd < 0) { perror("reliability socket"); return 1; }
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("reliability connect"); return 1;
	}
	while (sent < TEST_SIZE) {
		size_t chunk = TEST_SIZE - sent;
		ssize_t n;
		if (chunk > 128 * 1024) chunk = 128 * 1024;
		n = send(fd, data + sent, chunk, MSG_NOSIGNAL);
		if (n < 0) { perror("reliability send"); return 1; }
		sent += (size_t)n;
	}
	if (recv(fd, reply, sizeof(reply), 0) != 2 || memcmp(reply, "OK", 2)) {
		fprintf(stderr, "bad reliability reply\n"); return 1;
	}
	printf("udp reliability client sent %zu bytes\n", sent);
	free(data); close(fd);
	return 0;
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	if (argc == 2 && !strcmp(argv[1], "server")) return run_server();
	if (argc == 2 && !strcmp(argv[1], "client")) return run_client();
	fprintf(stderr, "usage: %s server|client\n", argv[0]);
	return 2;
}
