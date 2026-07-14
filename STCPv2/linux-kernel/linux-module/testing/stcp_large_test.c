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
#define TEST_SIZE (200 * 1024)

static void make_addr(struct sockaddr_in *addr)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(7778);
    inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr);
}

static int server(void)
{
    struct sockaddr_in addr;
    unsigned char *buffer;
    size_t total = 0;
    int listener;
    int client;

    make_addr(&addr);
    buffer = malloc(TEST_SIZE);
    if (!buffer)
        return 1;

    listener = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_ID);
    if (listener < 0 ||
        bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
        listen(listener, 8) < 0) {
        perror("server setup");
        return 1;
    }

    client = accept(listener, NULL, NULL);
    if (client < 0) {
        perror("accept");
        return 1;
    }

    while (total < TEST_SIZE) {
        ssize_t n = recv(client, buffer + total, TEST_SIZE - total, 0);
        if (n <= 0) {
            perror("recv");
            return 1;
        }
        total += (size_t)n;
    }

    for (size_t i = 0; i < TEST_SIZE; ++i) {
        if (buffer[i] != (unsigned char)(i & 0xff)) {
            fprintf(stderr, "data mismatch at %zu\n", i);
            return 1;
        }
    }

    printf("server: verified %zu framed bytes\n", total);
    send(client, "OK", 2, 0);
    close(client);
    close(listener);
    free(buffer);
    return 0;
}

static int client(void)
{
    struct sockaddr_in addr;
    unsigned char *buffer;
    char reply[2];
    size_t sent = 0;
    int fd;

    make_addr(&addr);
    buffer = malloc(TEST_SIZE);
    if (!buffer)
        return 1;

    for (size_t i = 0; i < TEST_SIZE; ++i)
        buffer[i] = (unsigned char)(i & 0xff);

    fd = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_ID);
    if (fd < 0 || connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("client setup");
        return 1;
    }

    while (sent < TEST_SIZE) {
        size_t chunk = TEST_SIZE - sent;
        if (chunk > 128 * 1024)
            chunk = 128 * 1024;

        ssize_t n = send(fd, buffer + sent, chunk, 0);
        if (n <= 0) {
            perror("send");
            return 1;
        }
        sent += (size_t)n;
    }

    if (recv(fd, reply, sizeof(reply), 0) != 2 || memcmp(reply, "OK", 2)) {
        fprintf(stderr, "bad reply\n");
        return 1;
    }

    printf("client: sent %zu bytes through STCP frames\n", sent);
    close(fd);
    free(buffer);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;

    if (!strcmp(argv[1], "server"))
        return server();
    if (!strcmp(argv[1], "client"))
        return client();

    return 2;
}
