#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/posix/fcntl.h>
#include <zephyr/sys/fdtable.h>
#include <stcp/stcp_internal.h>
#include <stcp/stcp_debug.h>
#ifdef CONFIG_STCP_RUST_CORE
#include <stcp/stcp_rust_ffi.h>
#endif

LOG_MODULE_REGISTER(stcp_offload, CONFIG_STCP_LOG_LEVEL);

#define STCPPOLL_INF(...) STCP_POLL_INF(__VA_ARGS__)
#define STCPPOLL_WRN(...) STCP_POLL_WRN(__VA_ARGS__)
#define STCPPOLL_ERR(...) LOG_ERR(__VA_ARGS__)

#ifdef CONFIG_STCP_RUST_CORE
static int rust_errno(int rc)
{
    if (rc >= 0) return rc;
    errno = -rc;
    return -1;
}

static int wait_rust_ready(struct stcp_ctx *ctx, int timeout_ms)
{
    int64_t deadline = k_uptime_get() + timeout_ms;
    while (k_uptime_get() < deadline) {
        int ready = stcp_rust_is_connected(ctx->rust_ctx);
        if (ready > 0) return 0;
        if (ready < 0 && ready != -EAGAIN) return ready;
        (void)k_sem_take(&ctx->rust_event, K_MSEC(25));
        (void)stcp_rust_tick(ctx->rust_ctx);
    }
    return -ETIMEDOUT;
}
#endif

static int stcp_close(void *obj)
{
    struct stcp_ctx *ctx = obj;
    if (!ctx) return 0;
#ifdef CONFIG_STCP_RUST_CORE
    stcp_rust_rx_stop(ctx);
    if (ctx->rust_ctx) {
        stcp_rust_set_carrier(ctx->rust_ctx, NULL);
        stcp_rust_release(ctx->rust_ctx);
        ctx->rust_ctx = NULL;
    }
#endif
    if (ctx->carrier_fd >= 0) zsock_close(ctx->carrier_fd);
    stcp_ctx_free(ctx);
    return 0;
}

static ssize_t stcp_recv_common(struct stcp_ctx *ctx, void *buf, size_t len,
                                int flags)
{
#ifdef CONFIG_STCP_RUST_CORE
    const bool nonblocking = ctx->nonblocking ||
                             ((flags & ZSOCK_MSG_DONTWAIT) != 0);

    while (true) {
        ssize_t rc = stcp_rust_recv(ctx->rust_ctx, buf, len, flags);

        if (rc >= 0) {
            /* Keep the readiness latch asserted while more complete
             * application data remains buffered. */
            ctx->poll_read_wakeup = stcp_rust_has_data(ctx->rust_ctx) > 0;
            return rc;
        }
        if (rc == -EAGAIN) {
            ctx->poll_read_wakeup = false;
        }
        if (rc != -EAGAIN) {
            errno = (int)-rc;
            return -1;
        }
        if (nonblocking) {
            errno = EAGAIN;
            return -1;
        }
        (void)k_sem_take(&ctx->rust_event, K_MSEC(100));
    }
#else
    return zsock_recv(ctx->carrier_fd, buf, len, flags);
#endif
}

static ssize_t stcp_send_common(struct stcp_ctx *ctx, const void *buf,
                                size_t len, int flags)
{
#ifdef CONFIG_STCP_RUST_CORE
    const bool nonblocking = ctx->nonblocking ||
                             ((flags & ZSOCK_MSG_DONTWAIT) != 0);

    while (true) {
        ssize_t rc = stcp_rust_send(ctx->rust_ctx, buf, len, flags);

        if (rc >= 0) {
            return rc;
        }
        if (rc != -EAGAIN) {
            errno = (int)-rc;
            return -1;
        }
        if (nonblocking) {
            errno = EAGAIN;
            return -1;
        }
        (void)k_sem_take(&ctx->rust_event, K_MSEC(10));
        (void)stcp_rust_tick(ctx->rust_ctx);
    }
#else
    return zsock_send(ctx->carrier_fd, buf, len, flags);
#endif
}

static ssize_t stcp_read(void *obj, void *buf, size_t len)
{
    return stcp_recv_common(obj, buf, len, 0);
}

static ssize_t stcp_write(void *obj, const void *buf, size_t len)
{
    return stcp_send_common(obj, buf, len, 0);
}

static short stcp_poll_revents(struct stcp_ctx *ctx, short events)
{
    short revents = 0;

    if (!ctx) {
        return ZSOCK_POLLNVAL;
    }

    if (ctx->last_error != 0) {
        revents |= ZSOCK_POLLERR;
    }

    if (ctx->peer_closed || ctx->state == STCP_CLOSED || ctx->read_shutdown) {
        revents |= ZSOCK_POLLHUP;
    }

#ifdef CONFIG_STCP_RUST_CORE
    if ((events & ZSOCK_POLLIN) != 0) {
        int ready;

        if (ctx->state == STCP_LISTENING) {
            ready = stcp_rust_has_accept(ctx->rust_ctx);
        } else {
            ready = stcp_rust_has_data(ctx->rust_ctx);
        }

        STCPPOLL_INF("STCPPOLL READY CHECK fd=%d state=%d has_data=%d wake_latch=%d peer_closed=%d error=%d events=0x%x",
                ctx->fd, (int)ctx->state, ready,
                ctx->poll_read_wakeup ? 1 : 0,
                ctx->peer_closed ? 1 : 0, ctx->last_error, events);

        if (ready > 0 || ctx->poll_read_wakeup ||
            ctx->peer_closed || ctx->read_shutdown) {
            revents |= ZSOCK_POLLIN;
        } else if (ready < 0 && ready != -EAGAIN) {
            ctx->last_error = -ready;
            revents |= ZSOCK_POLLERR;
        }
    }

    if ((events & ZSOCK_POLLOUT) != 0 && !ctx->write_shutdown &&
        !ctx->peer_closed) {
        int connected = stcp_rust_is_connected(ctx->rust_ctx);

        if (connected > 0 && stcp_rust_can_send(ctx->rust_ctx, 1) > 0) {
            revents |= ZSOCK_POLLOUT;
        } else if (connected < 0 && connected != -EAGAIN) {
            ctx->last_error = -connected;
            revents |= ZSOCK_POLLERR;
        }
    }
#else
    ARG_UNUSED(events);
#endif

    return revents;
}

static int normalize_poll_ioctl_result(int rc, const char *phase,
                                       struct stcp_ctx *ctx)
{
    if (rc == -1) {
        int err = errno != 0 ? errno : EIO;

        STCPPOLL_ERR("STCPPOLL %s returned POSIX -1; normalize errno=%d ctx=%p fd=%d state=%d",
                phase, err, ctx, ctx ? ctx->fd : -1,
                ctx ? (int)ctx->state : -1);
        return -err;
    }

    return rc;
}

extern const struct socket_op_vtable stcp_vtable;

static int stcp_poll_offload(struct zsock_pollfd *fds, int nfds, int timeout_ms)
{
    struct k_poll_event poll_events[CONFIG_NET_SOCKETS_POLL_MAX];
    struct stcp_ctx *contexts[CONFIG_NET_SOCKETS_POLL_MAX];
    int64_t deadline = timeout_ms < 0 ? INT64_MAX : k_uptime_get() + timeout_ms;
    int event_count;

    if (!fds || nfds < 0 || nfds > CONFIG_NET_SOCKETS_POLL_MAX) {
        errno = EINVAL;
        return -1;
    }

    STCPPOLL_INF("STCPPOLL OFFLOAD enter nfds=%d timeout_ms=%d", nfds, timeout_ms);

    for (;;) {
        int ready_count = 0;
        int i;

        event_count = 0;
        memset(contexts, 0, sizeof(contexts));

        for (i = 0; i < nfds; i++) {
            struct stcp_ctx *ctx;

            fds[i].revents = 0;
            if (fds[i].fd < 0) {
                continue;
            }

            ctx = zvfs_get_fd_obj(fds[i].fd,
                    &stcp_vtable.fd_vtable,
                    ENOTSUP);
            if (!ctx) {
                if (errno == EBADF) {
                    fds[i].revents = ZSOCK_POLLNVAL;
                    ready_count++;
                    continue;
                }

                /* Zephyr does not allow mixing this offload provider with
                 * native sockets or another offload provider in one poll(). */
                errno = EINVAL;
                STCPPOLL_ERR("STCPPOLL OFFLOAD foreign fd=%d index=%d", fds[i].fd, i);
                return -1;
            }

            contexts[i] = ctx;
            fds[i].revents = stcp_poll_revents(ctx, fds[i].events);
            if (fds[i].revents != 0) {
                ready_count++;
                STCPPOLL_INF("STCPPOLL OFFLOAD ready fd=%d events=0x%x revents=0x%x state=%d",
                        fds[i].fd, fds[i].events, fds[i].revents,
                        (int)ctx->state);
            }

#ifdef CONFIG_STCP_RUST_CORE
            if (fds[i].revents == 0 && ctx->state != STCP_LISTENING &&
                event_count < CONFIG_NET_SOCKETS_POLL_MAX) {
                k_poll_event_init(&poll_events[event_count],
                        K_POLL_TYPE_SEM_AVAILABLE,
                        K_POLL_MODE_NOTIFY_ONLY,
                        &ctx->rust_event);
                event_count++;
            }
#endif
        }

        if (ready_count > 0) {
            STCPPOLL_INF("STCPPOLL OFFLOAD return ready=%d", ready_count);
            return ready_count;
        }

        if (timeout_ms == 0) {
            STCPPOLL_INF("STCPPOLL OFFLOAD return timeout=0");
            return 0;
        }

        if (timeout_ms > 0 && k_uptime_get() >= deadline) {
            STCPPOLL_INF("STCPPOLL OFFLOAD return timeout expired");
            return 0;
        }

        /* Session sockets are event driven through rust_event. Listener
         * readiness originates in the native carrier listener, so cap the
         * wait to 25 ms when a listener is present and re-check it. */
        {
            bool has_listener = false;
            int wait_ms;
            int rc;

            for (i = 0; i < nfds; i++) {
                if (contexts[i] && contexts[i]->state == STCP_LISTENING) {
                    struct zsock_pollfd carrier_pfd = {
                        .fd = contexts[i]->carrier_fd,
                        .events = fds[i].events,
                        .revents = 0,
                    };
                    int carrier_rc = zsock_poll(&carrier_pfd, 1, 0);

                    has_listener = true;
                    if (carrier_rc > 0) {
                        fds[i].revents |= carrier_pfd.revents;
                        ready_count++;
                    } else if (carrier_rc < 0) {
                        fds[i].revents |= ZSOCK_POLLERR;
                        ready_count++;
                    }
                }
            }

            if (ready_count > 0) {
                STCPPOLL_INF("STCPPOLL OFFLOAD listener ready=%d", ready_count);
                return ready_count;
            }

            if (timeout_ms < 0) {
                wait_ms = has_listener ? 25 : SYS_FOREVER_MS;
            } else {
                int64_t remaining = deadline - k_uptime_get();

                if (remaining <= 0) {
                    return 0;
                }
                wait_ms = (int)MIN(remaining, has_listener ? 25 : remaining);
            }

            if (event_count == 0) {
                k_msleep(wait_ms == SYS_FOREVER_MS ? 25 : wait_ms);
                continue;
            }

            STCPPOLL_INF("STCPPOLL OFFLOAD wait events=%d wait_ms=%d", event_count, wait_ms);
            rc = k_poll(poll_events, event_count,
                    wait_ms == SYS_FOREVER_MS ? K_FOREVER : K_MSEC(wait_ms));
            if (rc != 0 && rc != -EAGAIN && rc != -EINTR) {
                errno = -rc;
                STCPPOLL_ERR("STCPPOLL OFFLOAD k_poll failed rc=%d errno=%d", rc, errno);
                return -1;
            }

            for (i = 0; i < nfds; i++) {
                if (contexts[i] && contexts[i]->state != STCP_LISTENING) {
                    (void)k_sem_take(&contexts[i]->rust_event, K_NO_WAIT);
                }
            }
        }
    }
}

static int stcp_ioctl(void *obj, unsigned int request, va_list args)
{
    struct stcp_ctx *ctx = obj;

    if (!ctx) {
        return -EINVAL;
    }

    switch (request) {
    case F_GETFL:
        return ctx->nonblocking ? O_NONBLOCK : 0;

    case F_SETFL: {
        int flags = va_arg(args, int);

        ctx->nonblocking = (flags & O_NONBLOCK) != 0;
        return 0;
    }

    case ZFD_IOCTL_POLL_PREPARE:
        /* AF_STCP is a registered socket-offload provider. Zephyr's socket
         * core requires PREPARE to return -EXDEV, after which it delegates
         * the complete poll() call through ZFD_IOCTL_POLL_OFFLOAD. */
        STCPPOLL_INF("STCPPOLL PREPARE fd=%d -> EXDEV", ctx->fd);
        return -EXDEV;

    case ZFD_IOCTL_POLL_UPDATE:
        STCPPOLL_WRN("STCPPOLL UPDATE unexpected fd=%d", ctx->fd);
        return -EOPNOTSUPP;

    case ZFD_IOCTL_POLL_OFFLOAD: {
        struct zsock_pollfd *fds = va_arg(args, struct zsock_pollfd *);
        int nfds = va_arg(args, int);
        int timeout_ms = va_arg(args, int);

        STCPPOLL_INF("STCPPOLL IOCTL OFFLOAD owner_fd=%d nfds=%d timeout_ms=%d",
                ctx->fd, nfds, timeout_ms);
        return stcp_poll_offload(fds, nfds, timeout_ms);
    }

    case ZFD_IOCTL_SET_LOCK:
        (void)va_arg(args, struct k_mutex *);
        return 0;

    default:
        STCPPOLL_WRN("STCPPOLL unsupported ioctl fd=%d request=0x%x", ctx->fd,
                request);
        return -EOPNOTSUPP;
    }
}

static int stcp_bind(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_ctx *ctx = obj;
    const struct sockaddr_in *sin = (const struct sockaddr_in *)addr;
    if (!ctx || !addr || addrlen < sizeof(*sin)) { errno = EINVAL; return -1; }
    if (zsock_bind(ctx->carrier_fd, addr, addrlen) < 0) return -1;
#ifdef CONFIG_STCP_RUST_CORE
    if (rust_errno(stcp_rust_bind(ctx->rust_ctx, sin->sin_addr.s_addr, sin->sin_port)) < 0) return -1;
#endif
    memcpy(&ctx->local, addr, sizeof(ctx->local));
    ctx->state = STCP_BOUND;
    return 0;
}

static int stcp_build_carrier_peer(const struct sockaddr *addr,
                                   socklen_t addrlen,
                                   struct sockaddr_in *carrier_peer)
{
    const struct sockaddr_in *stcp_peer;

    if (!addr || !carrier_peer || addrlen < sizeof(struct sockaddr_in)) {
        errno = EINVAL;
        return -1;
    }

    /*
     * The public AF_STCP socket still receives an IPv4 sockaddr_in from the
     * application. Never pass that object directly to the carrier socket:
     * build a separate AF_INET sockaddr_in so the W5500 offload sees only
     * the native IPv4 family, port and address.
     */
    stcp_peer = (const struct sockaddr_in *)addr;
    memset(carrier_peer, 0, sizeof(*carrier_peer));
    carrier_peer->sin_family = AF_INET;
    carrier_peer->sin_port = stcp_peer->sin_port;
    carrier_peer->sin_addr.s_addr = stcp_peer->sin_addr.s_addr;

    if (carrier_peer->sin_port == 0 ||
        carrier_peer->sin_addr.s_addr == INADDR_ANY) {
        errno = EDESTADDRREQ;
        return -1;
    }

    return 0;
}

static int stcp_connect(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_ctx *ctx = obj;
    const struct sockaddr_in *peer = (const struct sockaddr_in *)addr;
    struct sockaddr_in carrier_peer;
    int rc;

    if (!ctx || !addr) {
        errno = EINVAL;
        return -1;
    }

    if (stcp_build_carrier_peer(addr, addrlen, &carrier_peer) < 0) {
        LOG_ERR("carrier peer conversion failed: source_family=%d source_len=%u errno=%d",
                addr ? addr->sa_family : -1,
                (unsigned int)addrlen, errno);
        return -1;
    }

    ctx->state = STCP_CONNECTING;
    {
        uint32_t peer_host = ntohl(carrier_peer.sin_addr.s_addr);
        int64_t started_ms;
        int saved_errno;
        int so_error = 0;
        socklen_t so_error_len = sizeof(so_error);

        STCP_SOCKET_INF("carrier peer converted: fd=%d source_family=%d source_len=%u "
                "carrier_family=%d carrier_len=%u peer=%u.%u.%u.%u:%u",
                ctx->carrier_fd, addr->sa_family, (unsigned int)addrlen,
                carrier_peer.sin_family,
                (unsigned int)sizeof(carrier_peer),
                (unsigned int)((peer_host >> 24) & 0xffU),
                (unsigned int)((peer_host >> 16) & 0xffU),
                (unsigned int)((peer_host >> 8) & 0xffU),
                (unsigned int)(peer_host & 0xffU),
                ntohs(carrier_peer.sin_port));

        errno = 0;
        started_ms = k_uptime_get();
        rc = zsock_connect(ctx->carrier_fd,
                           (const struct sockaddr *)&carrier_peer,
                           (socklen_t)sizeof(carrier_peer));
        saved_errno = errno;

        STCP_SOCKET_INF("carrier connect return: fd=%d rc=%d errno=%d elapsed_ms=%lld",
                ctx->carrier_fd, rc, saved_errno,
                (long long)(k_uptime_get() - started_ms));

        errno = 0;
        if (zsock_getsockopt(ctx->carrier_fd, SOL_SOCKET, SO_ERROR,
                             &so_error, &so_error_len) == 0) {
            STCP_SOCKET_INF("carrier connect SO_ERROR: fd=%d error=%d",
                    ctx->carrier_fd, so_error);
        } else {
            STCP_SOCKET_DBG("carrier connect SO_ERROR unavailable: fd=%d errno=%d",
                    ctx->carrier_fd, errno);
        }

        if (rc < 0) {
            ctx->last_error = saved_errno != 0 ? saved_errno : EIO;
            ctx->state = STCP_CREATED;
            errno = ctx->last_error;
            return -1;
        }
    }
#ifdef CONFIG_STCP_RUST_CORE
    rc = stcp_rust_connect(ctx->rust_ctx, peer->sin_addr.s_addr,
                           peer->sin_port, 0);
    if (rc < 0) { errno = -rc; return -1; }
    rc = stcp_rust_rx_start(ctx);
    if (rc < 0) { errno = -rc; return -1; }
    STCP_HANDSHAKE_INF("Rust STCP handshake start fd=%d rust_ctx=%p", ctx->fd, ctx->rust_ctx);
    rc = stcp_rust_start_handshake(ctx->rust_ctx);
    if (rc < 0) { errno = -rc; return -1; }
    rc = wait_rust_ready(ctx, CONFIG_STCP_CONNECT_TIMEOUT_MS);
    if (rc < 0) {
        LOG_ERR("Rust STCP handshake failed rc=%d", rc);
        errno = -rc;
        return -1;
    }
    STCP_HANDSHAKE_INF("Rust STCP handshake complete fd=%d", ctx->fd);
#endif
    memcpy(&ctx->peer, &carrier_peer, sizeof(ctx->peer));
    ctx->state = STCP_CONNECTED;
    return 0;
}

static int stcp_listen(void *obj, int backlog)
{
    struct stcp_ctx *ctx = obj;
    if (zsock_listen(ctx->carrier_fd, backlog) < 0) return -1;
#ifdef CONFIG_STCP_RUST_CORE
    if (rust_errno(stcp_rust_listen(ctx->rust_ctx, backlog)) < 0) return -1;
#endif
    ctx->state = STCP_LISTENING;
    return 0;
}

static void stcp_poll_fd_diagnostic(int fd, struct stcp_ctx *expected_ctx,
                                    const char *origin);

static int stcp_accept(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    /* Phase 1 keeps server-side accept on the existing C carrier path.
     * Client connect/send/recv already use the shared Raspberry Rust core. */
    struct stcp_ctx *listener = obj;
    int cfd = zsock_accept(listener->carrier_fd, addr, addrlen);
    if (cfd < 0) return -1;
    struct stcp_ctx *child = stcp_ctx_alloc();
    if (!child) { zsock_close(cfd); errno = ENOMEM; return -1; }
    int fd = zvfs_reserve_fd();
    if (fd < 0) { zsock_close(cfd); stcp_ctx_free(child); return -1; }
    child->fd = fd; child->carrier_fd = cfd;
    child->socket_type = listener->socket_type;
    child->protocol = listener->protocol;
    child->state = STCP_CONNECTED;
    extern const struct socket_op_vtable stcp_vtable;
    zvfs_finalize_typed_fd(fd, child, &stcp_vtable.fd_vtable, ZVFS_MODE_IFSOCK);
    stcp_poll_fd_diagnostic(fd, child, "accept");
    return fd;
}

static ssize_t stcp_sendto(void *obj, const void *buf, size_t len, int flags,
                           const struct sockaddr *dst, socklen_t dstlen)
{
    struct stcp_ctx *ctx = obj;

#ifdef CONFIG_STCP_RUST_CORE
    ARG_UNUSED(dst);
    ARG_UNUSED(dstlen);
    return stcp_send_common(ctx, buf, len, flags);
#else
    return dst ? zsock_sendto(ctx->carrier_fd, buf, len, flags, dst, dstlen)
               : zsock_send(ctx->carrier_fd, buf, len, flags);
#endif
}

static ssize_t stcp_recvfrom(void *obj, void *buf, size_t len, int flags,
                             struct sockaddr *src, socklen_t *srclen)
{
    struct stcp_ctx *ctx = obj;

#ifdef CONFIG_STCP_RUST_CORE
    ARG_UNUSED(src);
    ARG_UNUSED(srclen);
    return stcp_recv_common(ctx, buf, len, flags);
#else
    return src ? zsock_recvfrom(ctx->carrier_fd, buf, len, flags, src, srclen)
               : zsock_recv(ctx->carrier_fd, buf, len, flags);
#endif
}

static int stcp_shutdown(void *obj, int how)
{
    struct stcp_ctx *ctx = obj;

    if (how == ZSOCK_SHUT_RD || how == ZSOCK_SHUT_RDWR) {
        ctx->read_shutdown = true;
    }
    if (how == ZSOCK_SHUT_WR || how == ZSOCK_SHUT_RDWR) {
        ctx->write_shutdown = true;
    }
#ifdef CONFIG_STCP_RUST_CORE
    stcp_rust_shutdown(ctx->rust_ctx, how);
    k_sem_give(&ctx->rust_event);
#endif
    return zsock_shutdown(ctx->carrier_fd, how);
}
static int stcp_getsockopt(void *obj,int level,int optname,void *optval,socklen_t *optlen)
{ return zsock_getsockopt(((struct stcp_ctx *)obj)->carrier_fd,level,optname,optval,optlen); }
static int stcp_setsockopt(void *obj,int level,int optname,const void *optval,socklen_t optlen)
{ return zsock_setsockopt(((struct stcp_ctx *)obj)->carrier_fd,level,optname,optval,optlen); }
static int stcp_getpeername(void *obj, struct sockaddr *addr, socklen_t *len)
{ return zsock_getpeername(((struct stcp_ctx *)obj)->carrier_fd,addr,len); }
static int stcp_getsockname(void *obj, struct sockaddr *addr, socklen_t *len)
{ return zsock_getsockname(((struct stcp_ctx *)obj)->carrier_fd,addr,len); }

const struct socket_op_vtable stcp_vtable = {
    .fd_vtable = { .read = stcp_read, .write = stcp_write, .close = stcp_close, .ioctl = stcp_ioctl },
    .bind = stcp_bind, .connect = stcp_connect, .listen = stcp_listen, .accept = stcp_accept,
    .sendto = stcp_sendto, .recvfrom = stcp_recvfrom, .shutdown = stcp_shutdown,
    .getsockopt = stcp_getsockopt, .setsockopt = stcp_setsockopt,
    .getpeername = stcp_getpeername, .getsockname = stcp_getsockname,
};

/*
 * One-shot fd-table diagnostic used while bringing up AF_STCP poll support.
 * This verifies the exact object/vtable stored by zvfs_finalize_typed_fd(),
 * then directly invokes POLL_PREPARE through the vtable Zephyr will use.
 */
static void stcp_poll_fd_diagnostic(int fd, struct stcp_ctx *expected_ctx,
                                    const char *origin)
{
    const struct fd_op_vtable *actual_vtable = NULL;
    const struct fd_op_vtable *expected_vtable = &stcp_vtable.fd_vtable;
    struct k_mutex *fd_lock = NULL;
    struct zvfs_pollfd pfd = {
        .fd = fd,
        .events = ZVFS_POLLOUT,
        .revents = 0,
    };
    struct k_poll_event events[1];
    struct k_poll_event *pev = events;
    struct k_poll_event *pev_end = events + ARRAY_SIZE(events);
    void *actual_obj;
    int rc;
    int saved_errno;

    errno = 0;
    actual_obj = zvfs_get_fd_obj_and_vtable(fd, &actual_vtable, &fd_lock);
    saved_errno = errno;

    STCPPOLL_INF("STCPPOLL FD CHECK origin=%s fd=%d obj=%p expected_obj=%p "
                 "vtable=%p expected_vtable=%p ioctl=%p expected_ioctl=%p "
                 "lock=%p errno=%d",
                 origin, fd, actual_obj, expected_ctx,
                 actual_vtable, expected_vtable,
                 actual_vtable ? actual_vtable->ioctl : NULL,
                 expected_vtable->ioctl, fd_lock, saved_errno);

    if (actual_obj == NULL || actual_vtable == NULL ||
        actual_vtable->ioctl == NULL) {
        STCPPOLL_ERR("STCPPOLL FD CHECK invalid origin=%s fd=%d", origin, fd);
        return;
    }

    errno = 0;
    rc = zvfs_fdtable_call_ioctl(actual_vtable, actual_obj,
                                 ZFD_IOCTL_POLL_PREPARE,
                                 &pfd, &pev, pev_end);
    saved_errno = errno;

    STCPPOLL_INF("STCPPOLL DIRECT PREPARE origin=%s fd=%d rc=%d errno=%d "
                 "events_used=%d revents=0x%x",
                 origin, fd, rc, saved_errno,
                 (int)(pev - events), pfd.revents);

    if (actual_obj != expected_ctx) {
        STCPPOLL_ERR("STCPPOLL FD OBJ MISMATCH origin=%s fd=%d", origin, fd);
    }
    if (actual_vtable != expected_vtable) {
        STCPPOLL_ERR("STCPPOLL FD VTABLE MISMATCH origin=%s fd=%d", origin, fd);
    }
    if (actual_vtable->ioctl != stcp_ioctl) {
        STCPPOLL_ERR("STCPPOLL FD IOCTL MISMATCH origin=%s fd=%d", origin, fd);
    }
    if (rc != -EXDEV) {
        STCPPOLL_ERR("STCPPOLL DIRECT PREPARE unexpected origin=%s fd=%d rc=%d",
                     origin, fd, rc);
    }
}

static bool stcp_supported(int family, int type, int protocol)
{
    return family == AF_STCP && (type == SOCK_STREAM || type == SOCK_DGRAM) &&
           (protocol == 0 || protocol == IPPROTO_STCP);
}

static int stcp_socket(int family, int type, int protocol)
{
    struct stcp_ctx *ctx;
    int fd;
    int rc;
    ARG_UNUSED(family);
    if (protocol == 0) protocol = IPPROTO_STCP;
    ctx = stcp_ctx_alloc();
    if (!ctx) { errno = ENOMEM; return -1; }
    errno = 0;
    ctx->carrier_fd = stcp_carrier_open(type);
    if (ctx->carrier_fd < 0) {
        int saved_errno = errno;
        LOG_ERR("carrier socket creation failed: type=%d errno=%d",
                type, saved_errno);
        stcp_ctx_free(ctx);
        errno = saved_errno;
        return -1;
    }
    STCP_SOCKET_INF("carrier socket ready: type=%d carrier_fd=%d",
            type, ctx->carrier_fd);
#ifdef CONFIG_STCP_RUST_CORE
    rc = stcp_rust_create(type == SOCK_DGRAM ? 254 : 253, &ctx->rust_ctx);
    if (rc < 0) {
        zsock_close(ctx->carrier_fd); stcp_ctx_free(ctx); errno = -rc; return -1;
    }
    stcp_rust_set_owner(ctx->rust_ctx, ctx);
    stcp_rust_set_carrier(ctx->rust_ctx, ctx);
#endif
    fd = zvfs_reserve_fd();
    if (fd < 0) { stcp_close(ctx); return -1; }
    ctx->fd = fd; ctx->socket_type = type; ctx->protocol = protocol;
    zvfs_finalize_typed_fd(fd, ctx, &stcp_vtable.fd_vtable, ZVFS_MODE_IFSOCK);
    stcp_poll_fd_diagnostic(fd, ctx, "socket");
    STCP_SOCKET_INF("STCP fd=%d type=%d protocol=%d carrier_fd=%d rust=%p",
            fd, type, protocol, ctx->carrier_fd,
#ifdef CONFIG_STCP_RUST_CORE
            ctx->rust_ctx
#else
            NULL
#endif
    );
    return fd;
}
NET_SOCKET_OFFLOAD_REGISTER(stcp, CONFIG_STCP_SOCKET_PRIORITY, AF_STCP, stcp_supported, stcp_socket);
