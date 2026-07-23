#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef AF_STCP
#define AF_STCP 45
#endif
#define STCP_PROTO_TCP 253

static volatile sig_atomic_t stop_requested;
static void on_signal(int sig) { (void)sig; stop_requested = 1; }

static int recv_all(int fd, void *buf, size_t len) {
    uint8_t *p = buf;
    while (len) {
        ssize_t n = recv(fd, p, len, 0);
        if (n == 0) return -ECONNRESET;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -errno;
        }
        p += n; len -= (size_t)n;
    }
    return 0;
}
static int send_all(int fd, const void *buf, size_t len) {
    const uint8_t *p = buf;
    while (len) {
        ssize_t n = send(fd, p, len, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -errno;
        }
        p += n; len -= (size_t)n;
    }
    return 0;
}
static void serve(int fd) {
    for (;;) {
        uint32_t net_len, len;
        uint8_t *buf;
        if (recv_all(fd, &net_len, sizeof(net_len))) break;
        len = ntohl(net_len);
        if (len > 64U * 1024U * 1024U) break;
        buf = malloc(len ? len : 1);
        if (!buf) break;
        if (recv_all(fd, buf, len) || send_all(fd, &net_len, sizeof(net_len)) || send_all(fd, buf, len)) {
            free(buf); break;
        }
        free(buf);
    }
    close(fd);
}
int main(int argc, char **argv) {
    int port = argc > 1 ? atoi(argv[1]) : 19002;
    int fd = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons((uint16_t)port), .sin_addr.s_addr = htonl(INADDR_ANY) };
    if (fd < 0) { perror("socket(AF_STCP)"); return 1; }
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) < 0) { perror("bind"); return 1; }
    if (listen(fd, 256) < 0) { perror("listen"); return 1; }
    signal(SIGINT, on_signal); signal(SIGTERM, on_signal); signal(SIGPIPE, SIG_IGN);
    fprintf(stderr, "benchmark server: native STCP/TCP listen=0.0.0.0:%d fd=%d READY\n", port, fd);
    while (!stop_requested) {
        fprintf(stderr, "benchmark server: native waiting in accept fd=%d\n", fd);
        int cfd = accept4(fd, NULL, NULL, SOCK_CLOEXEC);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            perror("accept4(AF_STCP)");
            sleep(1); continue;
        }
        fprintf(stderr, "benchmark server: native accepted fd=%d\n", cfd);
        serve(cfd);
    }
    close(fd);
    return 0;
}
