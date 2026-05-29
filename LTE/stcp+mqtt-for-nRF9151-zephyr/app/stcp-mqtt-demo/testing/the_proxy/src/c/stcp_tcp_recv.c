// stcp_tcp_recv_linux.c

#include <poll.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <fcntl.h>

#include "kernel_socket.h"
#include "misc.h"

#define STCP_RECV_POLL_TIMEOUT_MS (5*1000)

static int stcp_wait_for_data_linux(
    int fd,
    int timeout_ms
)
{
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
    };

    int rc =
        poll(
            &pfd,
            1,
            timeout_ms
        );

    if (rc < 0) {
        return -errno;
    }

    if (rc == 0) {
        return -EAGAIN;
    }

    if (pfd.revents & POLLIN) {
        return 1;
    }

    if (pfd.revents & (POLLERR | POLLHUP)) {
        return -ECONNRESET;
    }

    if (pfd.revents & POLLNVAL) {
        return -EBADF;
    }

    return -EAGAIN;
}

intptr_t stcp_tcp_recv(
    void *sock_vp,
    uint8_t *buf,
    uintptr_t len,
    uint32_t non_bloking,
    uint32_t flags,
    uintptr_t* recv_len
)
{
    struct kernel_socket *sock =
        (struct kernel_socket *)sock_vp;

    if (!sock) {
        fprintf(stderr,
            "RECV: sock == NULL\n");
        return -EINVAL;
    }
    fprintf(stderr,
        "RECV: sock=%p fd=%d kctx=%p nb=%d len=%lu rcv_len=%p\n",
        sock,
        sock->fd,
        sock->kctx,
        non_bloking,
        (unsigned long)len,
        recv_len
    );

#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint 1\n");
#endif
    int fd = sock->fd;

    if (fd < 0) {
        fprintf(stderr,
            "RECV: invalid fd=%d\n",
            fd
        );
        return -EBADFD;
    }

#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint 2\n");
#endif
    if (!buf) {
        fprintf(stderr,
            "RECV: buf == NULL\n");
        return -EINVAL;
    }

#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint 3\n");
#endif
    if (len < 1) {
        fprintf(stderr,
            "RECV: len < 1\n");
        return -EINVAL;
    }

#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint 4\n");
#endif
    int exact_mode =
        flags & STCP_RECV_FLAG_EXACT_MODE;
   
    int my_flags = 
        STCP_RECV_FLAG_EXACT_MODE   |
        STCP_RECV_FLAG_NON_BLOCKING;

    int real_flags =
        flags & ~my_flags;

    if (non_bloking) {
        real_flags |= MSG_DONTWAIT;
#if RECV_DEBUG
        fprintf(stderr, "RECV: MSG_DONTWAIT set\n");
#endif
    }

    // Set non bloking ...
#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint 5\n");
    int fl = fcntl(fd, F_GETFL);

    fprintf(stderr,
        "FD=%d flags=0x%x NONBLOCK=%d\n",
        fd,
        fl,
        !!(fl & O_NONBLOCK)
    );        
#endif

    uintptr_t total = 0;

#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint 6\n");
#endif
    int64_t start = stcp_uptime_ms();

#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint 7\n");
#endif
    int64_t timeout = STCP_RECV_POLL_TIMEOUT_MS;

#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint recv len %p set 0\n", recv_len);
#endif

    *recv_len = 0;

re_read:
#if RECV_DEBUG
    fprintf(stderr, "RECV: checkpoint reread\n");
#endif

    {
        int wait_rc =
            stcp_wait_for_data_linux(
                fd,
                timeout
            );
#if RECV_DEBUG
        fprintf(stderr, "RECV: checkpoint wait data done\n");
#endif

        if (wait_rc < 0) {

#if RECV_DEBUG
            fprintf(stderr, "RECV: checkpoint wait data error\n");
#endif
            if (wait_rc == -EAGAIN) {

                if ((stcp_uptime_ms() - start)
                        > timeout) {

                    fprintf(stderr,
                        "RECV: timeout fd=%d\n",
                        fd
                    );

                    return -EAGAIN;
                }

#if RECV_DEBUG
                fprintf(stderr, "RECV: checkpoint wait data sleep\n");
#endif
                usleep(10 * 1000);
                goto re_read;
            }

#if RECV_DEBUG
            fprintf(stderr, "RECV: checkpoint wait data ret\n");
#endif
            return wait_rc;
        }
    }

    size_t left =
        len - total;

    fprintf(stderr,
        "RECV: recv fd=%d left=%lu total=%ld\n",
        fd,
        (unsigned long)left,
        (long)total
    );

    ssize_t rc =
        recv(
            fd,
            buf + total,
            left,
            real_flags
        );

    fprintf(stderr,
        "RECV: recv rc=%ld errno=%d\n",
        (long)rc,
        errno
    );

    if (rc == 0) {

        if (recv_len) {
#if RECV_DEBUG
            fprintf(stderr,
                "RECV: recv len updated..\n"
            );
#endif

           *recv_len = total;
        }

        if (total > 0) {
            return total;
        }

        return -ENOTCONN;
    }

    if (rc < 0) {

        if (errno == EAGAIN ||
            errno == EWOULDBLOCK) {

            if ((stcp_uptime_ms() - start)
                    > timeout) {
                return -EAGAIN;
            }

            usleep(10 * 1000);
            goto re_read;
        }

        return -errno;
    }

    total += rc;

    if (exact_mode &&
        total < len) {

        usleep(5 * 1000);
        goto re_read;
    }

    fprintf(stderr,
        "RECV: OK fd=%d total=%ld\n",
        fd,
        (long)total
    );
    if (recv_len) *recv_len = (uintptr_t)total;
    return total;
}
