#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

//
// STCP TCP RECV
//

int stcp_tcp_recv(
        void *sock,
        uint8_t *buf,
        int len,
        int non_blocking,
        int flags,
        int *recv_len)
{
    int fd = *(int*)sock;

    int rc = recv(fd, buf, len, 0);

    if (rc < 0) {

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -EAGAIN; 
        }

        return -errno;
    }

    if (recv_len)
        *recv_len = rc;

    return rc;
}

//
// STCP TCP SEND
//

int stcp_tcp_send(
        void *sock,
        const uint8_t *buf,
        int len)
{
    int fd = *(int*)sock;

    int rc = send(fd, buf, len, 0);

    if (rc < 0) {

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -EAGAIN;
        }

        return -errno;
    }

    return rc;
}

//
// STCP LOG
//

void stcp_rust_log(const char *msg, int len)
{
    if (!msg || len <= 0)
        return;

    printf("[STCP][LINUX] ");

    fwrite(msg, 1, len, stdout);

    printf("\n");
    fflush(stdout);
}