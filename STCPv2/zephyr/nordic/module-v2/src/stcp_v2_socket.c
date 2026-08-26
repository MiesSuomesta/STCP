#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/fdtable.h>
#include <stcp/stcp_v2_internal.h>
#include <stcp/stcp_rust_ffi.h>

LOG_MODULE_REGISTER(stcp_v2_socket, CONFIG_STCP_V2_LOG_LEVEL);

/*
 * Persistent lifecycle diagnostics.
 *
 * Robot may discard the serial output belonging to a PASSed test before the
 * following test fails.  Keep the previous socket teardown result in static
 * storage and print it when the next AF_STCP socket is created.
 *
 * stage_mask bits:
 *   0x01 close entered
 *   0x02 RX stop returned
 *   0x04 Rust context released (or there was no Rust context)
 *   0x08 carrier free returned (or there was no carrier)
 *   0x10 socket free entered
 *   0x20 socket free returned / close complete
 */
struct stcp_v2_lifecycle_summary {
    uint32_t close_seq;
    uint32_t create_seq;
    uint32_t stage_mask;
    int app_fd;
    int native_fd;
    int had_rust_ctx;
    int had_carrier;
    long rx_running_before;
    long rx_stop_before;
    long rx_running_after_stop;
    long rx_stop_after_stop;
    int64_t close_started_ms;
    int64_t close_finished_ms;
};

static struct stcp_v2_lifecycle_summary stcp_v2_lifesum;
static struct k_spinlock stcp_v2_lifesum_lock;

static void stcp_v2_lifesum_print_previous(void)
{
    struct stcp_v2_lifecycle_summary snap;
    k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
    snap = stcp_v2_lifesum;
    k_spin_unlock(&stcp_v2_lifesum_lock, key);

    LOG_ERR("LIFESUM PREV close_seq=%u create_seq=%u stage_mask=0x%02x "
            "app_fd=%d native_fd=%d had_ctx=%d had_carrier=%d "
            "rx_before=%ld/%ld rx_after_stop=%ld/%ld "
            "close_ms=%lld duration_ms=%lld complete=%d",
            snap.close_seq, snap.create_seq, snap.stage_mask,
            snap.app_fd, snap.native_fd,
            snap.had_rust_ctx, snap.had_carrier,
            snap.rx_running_before, snap.rx_stop_before,
            snap.rx_running_after_stop, snap.rx_stop_after_stop,
            (long long)snap.close_started_ms,
            snap.close_finished_ms >= snap.close_started_ms && snap.close_started_ms != 0
                ? (long long)(snap.close_finished_ms - snap.close_started_ms) : -1LL,
            (snap.stage_mask & 0x20U) != 0U ? 1 : 0);
}

static uint32_t stcp_v2_lifesum_note_create(void)
{
    uint32_t seq;
    k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
    stcp_v2_lifesum.create_seq++;
    seq = stcp_v2_lifesum.create_seq;
    k_spin_unlock(&stcp_v2_lifesum_lock, key);
    return seq;
}

static int wait_connected(struct stcp_v2_socket *sock)
{
    int64_t started = k_uptime_get();
    int64_t deadline = started + CONFIG_STCP_V2_CONNECT_TIMEOUT_MS;
    unsigned int iter = 0;

    LOG_INF("CONNDIAG wait ENTER sock=%p app_fd=%d native_fd=%d ctx=%p "
            "timeout_ms=%d rx_running=%ld",
            sock,
            sock->fd,
            sock->carrier != NULL ? sock->carrier->fd : -1,
            sock->rust_ctx,
            CONFIG_STCP_V2_CONNECT_TIMEOUT_MS,
            (long)atomic_get(&sock->rx_running));

    while (k_uptime_get() < deadline) {
        int rc;
        int tick_rc;
        int sem_rc;

        iter++;
        rc = stcp_rust_is_connected(sock->rust_ctx);

        if (rc > 0) {
            LOG_INF("CONNDIAG CONNECTED iter=%u elapsed_ms=%lld "
                    "rx_running=%ld",
                    iter,
                    (long long)(k_uptime_get() - started),
                    (long)atomic_get(&sock->rx_running));
            return 0;
        }

        if (rc < 0 && rc != -EAGAIN) {
            LOG_ERR("CONNDIAG is_connected FAILED iter=%u rc=%d "
                    "elapsed_ms=%lld",
                    iter,
                    rc,
                    (long long)(k_uptime_get() - started));
            return rc;
        }

        tick_rc = stcp_rust_tick(sock->rust_ctx);

        if (tick_rc < 0 && tick_rc != -EAGAIN) {
            LOG_ERR("CONNDIAG tick rc=%d iter=%u elapsed_ms=%lld",
                    tick_rc,
                    iter,
                    (long long)(k_uptime_get() - started));
        }

        sem_rc = k_sem_take(&sock->event, K_MSEC(20));

        if (iter == 1 || (iter % 25) == 0) {
            LOG_INF("CONNDIAG wait iter=%u elapsed_ms=%lld connected_rc=%d "
                    "tick_rc=%d sem_rc=%d rx_running=%ld rx_stop=%ld",
                    iter,
                    (long long)(k_uptime_get() - started),
                    rc,
                    tick_rc,
                    sem_rc,
                    (long)atomic_get(&sock->rx_running),
                    (long)atomic_get(&sock->rx_stop));
        }
    }

    LOG_ERR("CONNDIAG TIMEOUT iter=%u elapsed_ms=%lld rx_running=%ld "
            "rx_stop=%ld native_fd=%d",
            iter,
            (long long)(k_uptime_get() - started),
            (long)atomic_get(&sock->rx_running),
            (long)atomic_get(&sock->rx_stop),
            sock->carrier != NULL ? sock->carrier->fd : -1);

    return -ETIMEDOUT;
}

static int close_socket(void *obj)
{
    struct stcp_v2_socket *sock = obj;
    int native_fd;

    if (sock == NULL) {
        return 0;
    }

    native_fd = sock->carrier != NULL ? sock->carrier->fd : -1;
    {
        k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
        stcp_v2_lifesum.close_seq++;
        stcp_v2_lifesum.stage_mask = 0x01U;
        stcp_v2_lifesum.app_fd = sock->fd;
        stcp_v2_lifesum.native_fd = native_fd;
        stcp_v2_lifesum.had_rust_ctx = sock->rust_ctx != NULL ? 1 : 0;
        stcp_v2_lifesum.had_carrier = sock->carrier != NULL ? 1 : 0;
        stcp_v2_lifesum.rx_running_before = (long)atomic_get(&sock->rx_running);
        stcp_v2_lifesum.rx_stop_before = (long)atomic_get(&sock->rx_stop);
        stcp_v2_lifesum.rx_running_after_stop = -1;
        stcp_v2_lifesum.rx_stop_after_stop = -1;
        stcp_v2_lifesum.close_started_ms = k_uptime_get();
        stcp_v2_lifesum.close_finished_ms = 0;
        k_spin_unlock(&stcp_v2_lifesum_lock, key);
    }
    LOG_ERR("LIFECYCLE CLOSE ENTER sock=%p app_fd=%d native_fd=%d ctx=%p carrier=%p rx_running=%ld rx_stop=%ld",
            sock, sock->fd, native_fd, sock->rust_ctx, sock->carrier,
            (long)atomic_get(&sock->rx_running),
            (long)atomic_get(&sock->rx_stop));

    LOG_ERR("LIFECYCLE RX STOP ENTER sock=%p native_fd=%d", sock, native_fd);
    stcp_v2_rx_stop(sock);
    LOG_ERR("LIFECYCLE RX STOP RETURN sock=%p native_fd=%d rx_running=%ld rx_stop=%ld",
            sock, native_fd,
            (long)atomic_get(&sock->rx_running),
            (long)atomic_get(&sock->rx_stop));
    {
        k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
        stcp_v2_lifesum.rx_running_after_stop = (long)atomic_get(&sock->rx_running);
        stcp_v2_lifesum.rx_stop_after_stop = (long)atomic_get(&sock->rx_stop);
        stcp_v2_lifesum.stage_mask |= 0x02U;
        k_spin_unlock(&stcp_v2_lifesum_lock, key);
    }

    if (sock->rust_ctx != NULL) {
        LOG_ERR("LIFECYCLE RUST RELEASE ENTER sock=%p ctx=%p", sock, sock->rust_ctx);
        stcp_rust_set_owner(sock->rust_ctx, NULL);
        stcp_rust_set_carrier(sock->rust_ctx, NULL);
        stcp_rust_release(sock->rust_ctx);
        sock->rust_ctx = NULL;
        LOG_ERR("LIFECYCLE RUST RELEASE RETURN sock=%p", sock);
    }
    {
        k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
        stcp_v2_lifesum.stage_mask |= 0x04U;
        k_spin_unlock(&stcp_v2_lifesum_lock, key);
    }

    LOG_ERR("LIFECYCLE CARRIER FREE ENTER sock=%p carrier=%p native_fd=%d",
            sock, sock->carrier, native_fd);
    stcp_v2_carrier_free(sock->carrier);
    sock->carrier = NULL;
    LOG_ERR("LIFECYCLE CARRIER FREE RETURN sock=%p old_native_fd=%d", sock, native_fd);
    {
        k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
        stcp_v2_lifesum.stage_mask |= 0x08U;
        k_spin_unlock(&stcp_v2_lifesum_lock, key);
    }

    LOG_ERR("LIFECYCLE SOCKET FREE ENTER sock=%p app_fd=%d", sock, sock->fd);
    {
        k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
        stcp_v2_lifesum.stage_mask |= 0x10U;
        k_spin_unlock(&stcp_v2_lifesum_lock, key);
    }
    stcp_v2_socket_free(sock);
    {
        k_spinlock_key_t key = k_spin_lock(&stcp_v2_lifesum_lock);
        stcp_v2_lifesum.stage_mask |= 0x20U;
        stcp_v2_lifesum.close_finished_ms = k_uptime_get();
        k_spin_unlock(&stcp_v2_lifesum_lock, key);
    }
    LOG_ERR("LIFECYCLE CLOSE DONE sock=%p old_native_fd=%d", sock, native_fd);
    return 0;
}

static ssize_t read_socket(void *obj, void *buf, size_t len)
{
    struct stcp_v2_socket *sock = obj;
    while (true) {
        ssize_t rc = stcp_rust_recv(sock->rust_ctx, buf, len, 0);
        if (rc >= 0) {
            return rc;
        }
        if (rc != -EAGAIN) {
            errno = (int)-rc;
            return -1;
        }
        (void)k_sem_take(&sock->event, K_MSEC(100));
        (void)stcp_rust_tick(sock->rust_ctx);
    }
}

static ssize_t write_socket(void *obj, const void *buf, size_t len)
{
    struct stcp_v2_socket *sock = obj;
    while (true) {
        ssize_t rc = stcp_rust_send(sock->rust_ctx, buf, len, 0);
        if (rc >= 0) {
            return rc;
        }
        if (rc != -EAGAIN) {
            errno = (int)-rc;
            return -1;
        }
        (void)k_sem_take(&sock->event, K_MSEC(10));
        (void)stcp_rust_tick(sock->rust_ctx);
    }
}

static int ioctl_socket(void *obj, unsigned int request, va_list args)
{
    ARG_UNUSED(obj);
    ARG_UNUSED(request);
    ARG_UNUSED(args);
    errno = ENOTSUP;
    return -1;
}

static int bind_socket(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_v2_socket *sock = obj;
    const struct sockaddr_in *sin = (const struct sockaddr_in *)addr;
    if (sock == NULL || addr == NULL || addrlen < sizeof(*sin) || sin->sin_family != AF_INET) {
        errno = EINVAL;
        return -1;
    }
    if (zsock_bind(sock->carrier->fd, addr, addrlen) < 0) {
        return -1;
    }
    int rc = stcp_rust_bind(sock->rust_ctx, sin->sin_addr.s_addr, sin->sin_port);
    if (rc < 0) {
        errno = -rc;
        return -1;
    }
    memcpy(&sock->local, sin, sizeof(*sin));
    return 0;
}

static int connect_socket(void *obj, const struct sockaddr *addr, socklen_t addrlen)
{
    struct stcp_v2_socket *sock = obj;
    const struct sockaddr_in *peer = (const struct sockaddr_in *)addr;
    int rc;
    int native_rc;
    int saved_errno;
    int64_t started;

    if (sock == NULL || peer == NULL ||
        addrlen < sizeof(*peer) ||
        peer->sin_family != AF_INET) {
        errno = EINVAL;
        return -1;
    }

    started = k_uptime_get();

    LOG_INF("CONNDIAG connect ENTER sock=%p app_fd=%d carrier=%p "
            "native_fd=%d ctx=%p addrlen=%u peer_port=%u",
            sock,
            sock->fd,
            sock->carrier,
            sock->carrier != NULL ? sock->carrier->fd : -1,
            sock->rust_ctx,
            (unsigned int)addrlen,
            (unsigned int)ntohs(peer->sin_port));

    errno = 0;
    native_rc = zsock_connect(sock->carrier->fd, addr, addrlen);
    saved_errno = errno;

    LOG_INF("CONNDIAG native connect RETURN rc=%d errno=%d "
            "elapsed_ms=%lld native_fd=%d",
            native_rc,
            saved_errno,
            (long long)(k_uptime_get() - started),
            sock->carrier->fd);

    if (native_rc < 0) {
        errno = saved_errno;
        return -1;
    }

    memcpy(&sock->carrier->peer, peer, sizeof(*peer));
    sock->carrier->peer_valid = true;
    memcpy(&sock->peer, peer, sizeof(*peer));

    rc = stcp_rust_connect(sock->rust_ctx,
                           peer->sin_addr.s_addr,
                           peer->sin_port,
                           0);

    LOG_INF("CONNDIAG rust_connect RETURN rc=%d elapsed_ms=%lld ctx=%p",
            rc,
            (long long)(k_uptime_get() - started),
            sock->rust_ctx);

    if (rc < 0) {
        errno = -rc;
        return -1;
    }

    rc = stcp_v2_rx_start(sock);

    LOG_INF("CONNDIAG rx_start RETURN rc=%d elapsed_ms=%lld "
            "rx_running=%ld",
            rc,
            (long long)(k_uptime_get() - started),
            (long)atomic_get(&sock->rx_running));

    if (rc < 0) {
        errno = -rc;
        return -1;
    }

    rc = stcp_rust_start_handshake(sock->rust_ctx);

    LOG_INF("CONNDIAG start_handshake RETURN rc=%d elapsed_ms=%lld "
            "rx_running=%ld",
            rc,
            (long long)(k_uptime_get() - started),
            (long)atomic_get(&sock->rx_running));

    if (rc < 0) {
        errno = -rc;
        return -1;
    }

    rc = wait_connected(sock);

    LOG_INF("CONNDIAG wait_connected RETURN rc=%d elapsed_ms=%lld "
            "rx_running=%ld",
            rc,
            (long long)(k_uptime_get() - started),
            (long)atomic_get(&sock->rx_running));

    if (rc < 0) {
        errno = -rc;
        return -1;
    }

    LOG_INF("connected fd=%d type=%d", sock->fd, sock->socket_type);
    return 0;
}

static int listen_socket(void *obj, int backlog)
{
    struct stcp_v2_socket *sock = obj;
    int rc;

    if (sock->socket_type == SOCK_STREAM) {
        if (zsock_listen(sock->carrier->fd, backlog) < 0) {
            return -1;
        }
    }
    rc = stcp_rust_listen(sock->rust_ctx, backlog);
    if (rc < 0) {
        errno = -rc;
        return -1;
    }
    if (sock->socket_type == SOCK_DGRAM) {
        rc = stcp_v2_rx_start(sock);
        if (rc < 0) {
            errno = -rc;
            return -1;
        }
    }
    return 0;
}

static int accept_socket(void *obj, struct sockaddr *addr, socklen_t *addrlen)
{
    struct stcp_v2_socket *listener = obj;
    if (listener->socket_type != SOCK_DGRAM) {
        /* Stream accept intentionally waits for a shared-core accepted-stream API.
         * Do not re-introduce the old C-side pseudo-connected child path. */
        errno = EOPNOTSUPP;
        return -1;
    }

    while (true) {
        int ready = stcp_rust_has_accept(listener->rust_ctx);
        if (ready > 0) {
            break;
        }
        if (ready < 0) {
            errno = -ready;
            return -1;
        }
        (void)k_sem_take(&listener->event, K_MSEC(100));
    }

    void *child_rust = NULL;
    int rc = stcp_rust_accept(listener->rust_ctx, &child_rust, 0);
    if (rc < 0 || child_rust == NULL) {
        errno = rc < 0 ? -rc : EIO;
        return -1;
    }

    struct stcp_v2_carrier *carrier = stcp_rust_get_carrier(child_rust);
    if (carrier == NULL) {
        stcp_rust_release(child_rust);
        errno = EIO;
        return -1;
    }

    struct stcp_v2_socket *child = stcp_v2_socket_alloc();
    if (child == NULL) {
        stcp_rust_set_carrier(child_rust, NULL);
        stcp_rust_release(child_rust);
        stcp_v2_carrier_free(carrier);
        errno = ENOMEM;
        return -1;
    }

    int fd = zvfs_reserve_fd();
    if (fd < 0) {
        stcp_v2_socket_free(child);
        stcp_rust_set_carrier(child_rust, NULL);
        stcp_rust_release(child_rust);
        stcp_v2_carrier_free(carrier);
        return -1;
    }

    child->fd = fd;
    child->socket_type = SOCK_DGRAM;
    child->protocol = IPPROTO_STCP;
    child->rust_ctx = child_rust;
    child->carrier = carrier;
    stcp_rust_set_owner(child_rust, child);

    if (addr != NULL && addrlen != NULL && *addrlen >= sizeof(struct sockaddr_in)) {
        memcpy(addr, &carrier->peer, sizeof(carrier->peer));
        *addrlen = sizeof(carrier->peer);
    }

    extern const struct socket_op_vtable stcp_v2_vtable;
    zvfs_finalize_typed_fd(fd, child,
        (const struct fd_op_vtable *)&stcp_v2_vtable, ZVFS_MODE_IFSOCK);
    return fd;
}

static ssize_t sendto_socket(void *obj, const void *buf, size_t len, int flags,
                             const struct sockaddr *dst, socklen_t dstlen)
{
    ARG_UNUSED(flags);
    ARG_UNUSED(dst);
    ARG_UNUSED(dstlen);
    return write_socket(obj, buf, len);
}

static ssize_t recvfrom_socket(void *obj, void *buf, size_t len, int flags,
                               struct sockaddr *src, socklen_t *srclen)
{
    struct stcp_v2_socket *sock = obj;
    ARG_UNUSED(flags);
    ssize_t rc = read_socket(obj, buf, len);
    if (rc >= 0 && src != NULL && srclen != NULL && *srclen >= sizeof(struct sockaddr_in)) {
        if (sock->carrier != NULL && sock->carrier->peer_valid) {
            memcpy(src, &sock->carrier->peer, sizeof(sock->carrier->peer));
            *srclen = sizeof(sock->carrier->peer);
        }
    }
    return rc;
}

static int shutdown_socket(void *obj, int how)
{
    struct stcp_v2_socket *sock = obj;
    stcp_rust_shutdown(sock->rust_ctx, how);
    return zsock_shutdown(sock->carrier->fd, how);
}

static int getsockopt_socket(void *obj, int level, int optname, void *optval, socklen_t *optlen)
{
    return zsock_getsockopt(((struct stcp_v2_socket *)obj)->carrier->fd,
                            level, optname, optval, optlen);
}
static int setsockopt_socket(void *obj, int level, int optname, const void *optval, socklen_t optlen)
{
    return zsock_setsockopt(((struct stcp_v2_socket *)obj)->carrier->fd,
                            level, optname, optval, optlen);
}
static int getpeername_socket(void *obj, struct sockaddr *addr, socklen_t *len)
{
    return zsock_getpeername(((struct stcp_v2_socket *)obj)->carrier->fd, addr, len);
}
static int getsockname_socket(void *obj, struct sockaddr *addr, socklen_t *len)
{
    return zsock_getsockname(((struct stcp_v2_socket *)obj)->carrier->fd, addr, len);
}

const struct socket_op_vtable stcp_v2_vtable = {
    .fd_vtable = {
        .read = read_socket,
        .write = write_socket,
        .close = close_socket,
        .ioctl = ioctl_socket,
    },
    .bind = bind_socket,
    .connect = connect_socket,
    .listen = listen_socket,
    .accept = accept_socket,
    .sendto = sendto_socket,
    .recvfrom = recvfrom_socket,
    .shutdown = shutdown_socket,
    .getsockopt = getsockopt_socket,
    .setsockopt = setsockopt_socket,
    .getpeername = getpeername_socket,
    .getsockname = getsockname_socket,
};

static bool stcp_v2_supported(int family, int type, int protocol)
{
    return family == AF_STCP &&
           (type == SOCK_STREAM || type == SOCK_DGRAM) &&
           (protocol == 0 || protocol == IPPROTO_STCP);
}

static int stcp_v2_socket_create(int family, int type, int protocol)
{
    uint32_t create_seq;

    ARG_UNUSED(family);
    stcp_v2_lifesum_print_previous();
    create_seq = stcp_v2_lifesum_note_create();
    LOG_ERR("LIFESUM CREATE BEGIN create_seq=%u type=%d protocol=%d",
            create_seq, type, protocol);

    struct stcp_v2_socket *sock = stcp_v2_socket_alloc();
    if (sock == NULL) {
        errno = ENOMEM;
        return -1;
    }

    LOG_ERR("LIFECYCLE CREATE ALLOC sock=%p fd=%d type=%d rx_running=%ld rx_stop=%ld ctx=%p carrier=%p",
            sock, sock->fd, type,
            (long)atomic_get(&sock->rx_running),
            (long)atomic_get(&sock->rx_stop),
            sock->rust_ctx, sock->carrier);

    sock->carrier = stcp_v2_carrier_open(type);
    if (sock->carrier == NULL) {
        LOG_ERR("LIFECYCLE CREATE CARRIER FAIL sock=%p errno=%d", sock, errno);
        stcp_v2_socket_free(sock);
        return -1;
    }

    LOG_ERR("LIFECYCLE CREATE CARRIER sock=%p carrier=%p native_fd=%d owns_fd=%d",
            sock, sock->carrier, sock->carrier->fd, sock->carrier->owns_fd ? 1 : 0);

    int rc = stcp_rust_create(type == SOCK_DGRAM ? 254 : 253, &sock->rust_ctx);
    if (rc < 0 || sock->rust_ctx == NULL) {
        stcp_v2_carrier_free(sock->carrier);
        stcp_v2_socket_free(sock);
        errno = rc < 0 ? -rc : EIO;
        return -1;
    }

    LOG_ERR("LIFECYCLE CREATE RUST sock=%p ctx=%p native_fd=%d",
            sock, sock->rust_ctx, sock->carrier->fd);
    stcp_rust_set_owner(sock->rust_ctx, sock);
    stcp_rust_set_carrier(sock->rust_ctx, sock->carrier);

    int fd = zvfs_reserve_fd();
    if (fd < 0) {
        close_socket(sock);
        return -1;
    }
    sock->fd = fd;
    sock->socket_type = type;
    sock->protocol = protocol == 0 ? IPPROTO_STCP : protocol;
    zvfs_finalize_typed_fd(fd, sock,
        (const struct fd_op_vtable *)&stcp_v2_vtable, ZVFS_MODE_IFSOCK);

    LOG_ERR("LIFECYCLE CREATE DONE create_seq=%u sock=%p app_fd=%d native_fd=%d ctx=%p rx_running=%ld rx_stop=%ld",
            create_seq, sock, fd, sock->carrier->fd, sock->rust_ctx,
            (long)atomic_get(&sock->rx_running),
            (long)atomic_get(&sock->rx_stop));
    LOG_INF("AF_STCP fd=%d type=%d native_fd=%d", fd, type, sock->carrier->fd);
    return fd;
}

/* Important: this is a custom AF_STCP provider, NOT a generic socket offload.
 * Using NET_SOCKET_REGISTER keeps native AF_INET/W5500 sockets on Zephyr's
 * native stack and avoids CONFIG_NET_SOCKETS_OFFLOAD entirely. */
NET_SOCKET_REGISTER(stcp_v2, CONFIG_STCP_V2_SOCKET_PRIORITY, AF_STCP,
                    stcp_v2_supported, stcp_v2_socket_create);
