// stcp_tcp_send_linux.c

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>
#include <time.h>

#include "kernel_socket.h"
#include "misc.h"

intptr_t stcp_tcp_send(
    void *sock_vp,
    const uint8_t *buf,
    uintptr_t len
)
{
    struct kernel_socket *sock =
        (struct kernel_socket *)sock_vp;

    if (!sock) {
        fprintf(stderr,
            "SEND: sock == NULL\n");
        return -EINVAL;
    }

    fprintf(stderr,
        "SEND: sock=%p fd=%d kctx=%p len=%lu\n",
        sock,
        sock->fd,
        sock->kctx,
        (unsigned long)len
    );

    int fd = sock->fd;

    if (fd < 0) {
        fprintf(stderr,
            "SEND: invalid fd=%d\n",
            fd
        );
        return -EBADFD;
    }

    if (!buf) {
        fprintf(stderr,
            "SEND: buf == NULL\n");
        return -EINVAL;
    }

    if (len < 1) {
        fprintf(stderr,
            "SEND: len < 1\n");
        return -EINVAL;
    }

    int64_t timeout_ms =
        10 * 1000;

    int64_t start_ms =
        stcp_uptime_ms();

    size_t total = 0;

    while (total < len) {

        fprintf(stderr,
            "SEND: fd=%d total=%lu left=%lu\n",
            fd,
            (unsigned long)total,
            (unsigned long)(len - total)
        );

        ssize_t sent =
            send(
                fd,
                buf + total,
                len - total,
                0
            );

        fprintf(stderr,
            "SEND: sent=%ld errno=%d\n",
            (long)sent,
            errno
        );

        if (sent < 0) {

            if (errno == EAGAIN ||
                errno == EWOULDBLOCK) {

                if ((stcp_uptime_ms() - start_ms)
                        > timeout_ms) {

                    fprintf(stderr,
                        "SEND: timeout fd=%d\n",
                        fd
                    );

                    return -ETIMEDOUT;
                }

                usleep(10 * 1000);
                continue;
            }

            if (errno == ENOMEM ||
                errno == ENOBUFS) {

                usleep(50 * 1000);
                continue;
            }

            if (errno == ECONNRESET ||
                errno == ENOTCONN ||
                errno == EPIPE) {

                fprintf(stderr,
                    "SEND: socket dead errno=%d\n",
                    errno
                );

                return -errno;
            }

            return -errno;
        }

        total += sent;
    }

    fprintf(stderr,
        "SEND: OK fd=%d total=%lu\n",
        fd,
        (unsigned long)total
    );

    return total;
}

intptr_t stcp_tcp_send_iovec(
    void *sock_vp,
    void *msg_vp,
    int flags
)
{
    struct kernel_socket *sock =
        (struct kernel_socket *)sock_vp;

    struct msghdr *msg =
        (struct msghdr *)msg_vp;

    if (!sock) {
        fprintf(stderr,
            "SENDMSG: sock == NULL\n");
        return -EINVAL;
    }

    if (!msg) {
        fprintf(stderr,
            "SENDMSG: msg == NULL\n");
        return -EINVAL;
    }

    int fd = sock->fd;

    fprintf(stderr,
        "SENDMSG: sock=%p fd=%d msg=%p\n",
        sock,
        fd,
        msg
    );

    if (fd < 0) {
        return -EBADFD;
    }

    int64_t timeout_ms =
        10 * 1000;

    int64_t start_ms =
        stcp_uptime_ms();

    ssize_t total = 0;

    size_t remaining = 0;

    for (size_t i = 0; i < msg->msg_iovlen; i++) {
        remaining += msg->msg_iov[i].iov_len;
    }

    while (remaining > 0) {

        ssize_t sent =
            sendmsg(
                fd,
                msg,
                flags
            );

        fprintf(stderr,
            "SENDMSG: sent=%ld errno=%d\n",
            (long)sent,
            errno
        );

        if (sent < 0) {

            if (errno == EAGAIN ||
                errno == EWOULDBLOCK) {

                if ((stcp_uptime_ms() - start_ms)
                        > timeout_ms) {

                    return -ETIMEDOUT;
                }

                usleep(10 * 1000);
                continue;
            }

            if (errno == ENOMEM ||
                errno == ENOBUFS) {

                usleep(50 * 1000);
                continue;
            }

            return -errno;
        }

        total += sent;
        remaining -= sent;

        size_t consumed = sent;

        for (size_t i = 0;
             i < msg->msg_iovlen && consumed > 0;
             i++) {

            if (consumed >= msg->msg_iov[i].iov_len) {

                consumed -= msg->msg_iov[i].iov_len;

                msg->msg_iov[i].iov_base =
                    (uint8_t *)
                    msg->msg_iov[i].iov_base +
                    msg->msg_iov[i].iov_len;

                msg->msg_iov[i].iov_len = 0;

            } else {

                msg->msg_iov[i].iov_base =
                    (uint8_t *)
                    msg->msg_iov[i].iov_base +
                    consumed;

                msg->msg_iov[i].iov_len -= consumed;

                consumed = 0;
            }
        }
    }

    fprintf(stderr,
        "SENDMSG: OK fd=%d total=%ld\n",
        fd,
        (long)total
    );

    return total;
}