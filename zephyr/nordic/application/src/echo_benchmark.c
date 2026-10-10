#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/byteorder.h>

#include <stcp/stcp.h>
#include "echo_benchmark.h"
#include "ben2_metrics.h"
#if defined(CONFIG_NRF_MODEM_LIB)
#include "stcp_lte_transport.h"
#include "modem_status.h"
#endif

LOG_MODULE_REGISTER(echo_benchmark, LOG_LEVEL_INF);

#define BENCH_MAGIC 0x42454e32U /* BEN2 */
#define BENCH_VERSION BEN2_VERSION
#define BENCH_MODE_UPLOAD 1U
#define BENCH_MODE_DOWNLOAD 2U
#define BENCH_MODE_FULL 3U
#define BENCH_MAX_CHUNK CONFIG_BENCH_MAX_CHUNK_SIZE

struct bench_request {
    uint32_t magic;
    uint32_t version;
    uint32_t mode;
    uint32_t chunk_size;
    uint32_t total_bytes;
} __packed;

struct bench_reply {
    uint32_t magic;
    uint32_t mode;
    uint32_t status;
    uint32_t bytes_received;
    uint32_t bytes_sent;
} __packed;

struct raw_direction {
    uint64_t bytes, elapsed_us, calls, eagain, errors;
};
static struct raw_direction raw_tx, raw_rx;
static uint64_t raw_connect_us, raw_elapsed_us;
static uint64_t clock_us(void)
{
    return k_ticks_to_us_floor64(k_uptime_ticks());
}

static const struct bench_config *active_cfg;
static struct bench_summary last_summary;

static int validate_config(const struct bench_config *cfg);

static int wait_socket_ready(int fd, short events, int64_t deadline_ms)
{
    struct zsock_pollfd pfd = {.fd = fd, .events = events};

    while (true) {
        int64_t remaining = deadline_ms - k_uptime_get();
        int rc;
        if (remaining <= 0) return -ETIMEDOUT;
        pfd.revents = 0;
        rc = zsock_poll(&pfd, 1, (int)MIN(remaining, (int64_t)INT32_MAX));
        if (rc < 0) {
            if (errno == EINTR) continue;
            return -errno;
        }
        if (rc == 0) return -ETIMEDOUT;

        if (pfd.revents & (ZSOCK_POLLERR | ZSOCK_POLLHUP | ZSOCK_POLLNVAL)) {
            int so_error = 0;
            socklen_t optlen = sizeof(so_error);

            int opt_rc = zsock_getsockopt(
                fd, SOL_SOCKET, SO_ERROR, &so_error, &optlen);

            int opt_errno = (opt_rc < 0) ? errno : 0;
            LOG_ERR("STCP_POLL_DIAG fd=%d events=0x%x revents=0x%x "
                    "poll_rc=%d so_error=%d getsockopt_rc=%d errno=%d",
                    fd, (unsigned int)events,
                    (unsigned int)pfd.revents,
                    rc, so_error, opt_rc, opt_errno);
        }
        if (pfd.revents & ZSOCK_POLLNVAL) return -EBADF;
        if (pfd.revents & ZSOCK_POLLERR) return -EIO;
        if (pfd.revents & ZSOCK_POLLHUP) return -ECONNRESET;
        if (pfd.revents & events) return 0;
    }
}

static int send_all(int fd, const void *buf, size_t len)
{
    const uint8_t *p = buf;
    size_t done = 0;
    int64_t deadline = k_uptime_get() + active_cfg->timeout_ms;
    while (done < len) {
        ssize_t n = zsock_send(fd, p + done, len - done, 0);
        if (n > 0) { done += (size_t)n; deadline = k_uptime_get() + active_cfg->timeout_ms; continue; }
        if (n == 0) return -ECONNRESET;
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            int rc = wait_socket_ready(fd, ZSOCK_POLLOUT, deadline);
            if (rc < 0) return rc;
            continue;
        }
        return -errno;
    }
    return 0;
}

static int recv_all(int fd, void *buf, size_t len)
{
    uint8_t *p = buf;
    size_t done = 0;
    int64_t deadline = k_uptime_get() + active_cfg->timeout_ms;
    while (done < len) {
        ssize_t n = zsock_recv(fd, p + done, len - done, 0);
        if (n > 0) { done += (size_t)n; deadline = k_uptime_get() + active_cfg->timeout_ms; continue; }
        if (n == 0) return -ECONNRESET;
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            int rc = wait_socket_ready(fd, ZSOCK_POLLIN, deadline);
            if (rc < 0) return rc;
            continue;
        }
        return -errno;
    }
    return 0;
}

static int set_nonblocking(int fd)
{
    int flags = zsock_fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -errno;
    return zsock_fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0 ? -errno : 0;
}

static int configure_socket_timeouts(int fd, uint32_t timeout_ms)
{
    struct zsock_timeval tv = {
        .tv_sec = (long)(timeout_ms / 1000U),
        .tv_usec = (long)((timeout_ms % 1000U) * 1000U),
    };
    int rc_send;
    int rc_recv;
    int send_errno = 0;
    int recv_errno = 0;

    errno = 0;
    rc_send = zsock_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
                               &tv, sizeof(tv));
    if (rc_send < 0) {
        send_errno = errno;
    }

    errno = 0;
    rc_recv = zsock_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
                               &tv, sizeof(tv));
    if (rc_recv < 0) {
        recv_errno = errno;
    }

    LOG_DBG("APP SOCKET TIMEOUTS fd=%d timeout_ms=%u "
            "snd_rc=%d snd_errno=%d rcv_rc=%d rcv_errno=%d",
            fd, timeout_ms, rc_send, send_errno, rc_recv, recv_errno);

    /* Timeouts are best effort because some socket offloads reject one or
     * both options. The benchmark still has its own operation deadlines.
     */
    return 0;
}

static int connect_server_impl(const struct bench_config *cfg)
{
    struct zsock_addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};
    struct zsock_addrinfo *res = NULL;
    struct sockaddr_in literal_peer = {0};
    const struct sockaddr *connect_addr = NULL;
    socklen_t connect_addrlen = 0;
    bool resolved_with_dns = false;
    unsigned long port_value;
    char *port_end = NULL;
    int fd, rc;

    LOG_DBG("APP CONNECT START transport=%s host=%s port=%s",
            bench_transport_name(cfg->transport), cfg->host, cfg->port);

    /* Avoid DNS entirely when the configured host is already an IPv4
     * literal. This is the normal Ethernet benchmark path and keeps STCP
     * testing independent of DNS configuration.
     */
    port_value = strtoul(cfg->port, &port_end, 10);
    if (port_end == cfg->port || *port_end != '\0' ||
        port_value == 0UL || port_value > 65535UL) {
        LOG_ERR("APP CONNECT EXIT A: invalid port string=%s", cfg->port);
        return -EINVAL;
    }

    literal_peer.sin_family = AF_INET;
    literal_peer.sin_port = htons((uint16_t)port_value);
    rc = zsock_inet_pton(AF_INET, cfg->host, &literal_peer.sin_addr);
    if (rc == 1) {
        connect_addr = (const struct sockaddr *)&literal_peer;
        connect_addrlen = sizeof(literal_peer);
        LOG_DBG("APP RESOLVE IPV4 FAST PATH host=%s port=%lu family=%d addrlen=%u",
                cfg->host, port_value, literal_peer.sin_family,
                (unsigned int)connect_addrlen);
    } else {
        LOG_DBG("APP DNS LOOKUP START host=%s port=%s", cfg->host, cfg->port);
        rc = zsock_getaddrinfo(cfg->host, cfg->port, &hints, &res);
        LOG_DBG("APP DNS LOOKUP RETURN rc=%d result=%p", rc, res);
        if (rc || !res) {
            LOG_ERR("APP CONNECT EXIT B: DNS failed host=%s port=%s rc=%d; "
                    "use 'stcp config host <IPv4>' to bypass DNS",
                    cfg->host, cfg->port, rc);
            return -EHOSTUNREACH;
        }
        resolved_with_dns = true;
        connect_addr = res->ai_addr;
        connect_addrlen = res->ai_addrlen;
        LOG_DBG("APP DNS RESULT family=%d socktype=%d protocol=%d addrlen=%u",
                res->ai_family, res->ai_socktype, res->ai_protocol,
                (unsigned int)connect_addrlen);
    }

    errno = 0;
    if (cfg->transport == BENCH_TRANSPORT_STCP) {
        LOG_DBG("APP SOCKET CALL family=AF_STCP type=SOCK_STREAM protocol=%d",
                IPPROTO_STCP);
        fd = zsock_socket(AF_STCP, SOCK_STREAM, IPPROTO_STCP);
    } else if (cfg->transport == BENCH_TRANSPORT_TCP) {
        LOG_DBG("APP SOCKET CALL family=AF_INET type=SOCK_STREAM protocol=TCP");
        fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else {
        LOG_ERR("APP CONNECT EXIT B: unsupported transport=%d",
                cfg->transport);
        if (resolved_with_dns) {
            zsock_freeaddrinfo(res);
        }
        return -ENOTSUP;
    }
    if (fd < 0) {
        LOG_DBG("APP SOCKET RETURN fd=%d errno=%d", fd, errno);
        rc = -errno;
        LOG_ERR("APP CONNECT EXIT C: socket failed rc=%d", rc);
        if (resolved_with_dns) {
            zsock_freeaddrinfo(res);
        }
        return rc;
    }
    LOG_DBG("APP SOCKET RETURN fd=%d success", fd);

    (void)configure_socket_timeouts(fd, active_cfg->timeout_ms);

#if defined(CONFIG_STCP_LTE_CUSTOM_PDN) && \
    defined(CONFIG_LTE_LINK_CONTROL) && \
    !defined(CONFIG_STCP_TRANSPORT_ETHERNET)
    if (cfg->transport == BENCH_TRANSPORT_TCP ||
        cfg->transport == BENCH_TRANSPORT_STCP) {
        rc = stcp_lte_transport_bind_socket(fd);
        if (rc < 0) goto fail;
    }
#endif

#if defined(CONFIG_BENCH_STCP_BLOCKING_CONNECT)
    if (cfg->transport == BENCH_TRANSPORT_STCP) {
        LOG_DBG("APP SOCKET FLAGS transport=stcp mode=blocking "
                "nonblock_setup=skipped feature=BENCH_STCP_BLOCKING_CONNECT");
        errno = 0;
        LOG_DBG("APP ZSOCK_CONNECT CALL fd=%d addrlen=%u mode=blocking",
                fd, (unsigned int)connect_addrlen);
        rc = zsock_connect(fd, connect_addr, connect_addrlen);
        if (rc < 0) {
            int saved_errno = errno;
            rc = -saved_errno;
            LOG_DBG("APP ZSOCK_CONNECT RETURN fd=%d rc=-1 errno=%d "
                    "mapped_rc=%d mode=blocking",
                    fd, saved_errno, rc);
            goto fail;
        }
        LOG_DBG("APP ZSOCK_CONNECT RETURN fd=%d rc=0 mode=blocking", fd);
    } else
#endif
    {
        rc = set_nonblocking(fd);
        LOG_DBG("APP NONBLOCK RETURN fd=%d rc=%d errno=%d", fd, rc, errno);
        if (rc < 0) goto fail;

        errno = 0;
        LOG_DBG("APP ZSOCK_CONNECT CALL fd=%d addrlen=%u mode=nonblocking",
                fd, (unsigned int)connect_addrlen);
        rc = zsock_connect(fd, connect_addr, connect_addrlen);
        LOG_DBG("APP ZSOCK_CONNECT RETURN fd=%d rc=%d errno=%d mode=nonblocking",
                fd, rc, errno);
        if (rc < 0) {
            if (errno != EINPROGRESS && errno != EAGAIN &&
                errno != EWOULDBLOCK && errno != EALREADY) {
                rc = -errno;
                LOG_ERR("APP CONNECT EXIT D: immediate failure rc=%d", rc);
                goto fail;
            }
            LOG_DBG("APP POLL CONNECT fd=%d timeout_ms=%u",
                    fd, active_cfg->timeout_ms);
            rc = wait_socket_ready(fd, ZSOCK_POLLOUT,
                                   k_uptime_get() + active_cfg->timeout_ms);
            LOG_DBG("APP POLL CONNECT RETURN fd=%d rc=%d", fd, rc);
            if (rc < 0) goto fail;
            int err = 0; socklen_t sl = sizeof(err);
            if (zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &sl) < 0) {
                rc = -errno;
                LOG_DBG("APP SO_ERROR READ FAILED fd=%d rc=%d", fd, rc);
                goto fail;
            }
            LOG_DBG("APP SO_ERROR fd=%d error=%d", fd, err);
            if (err) { rc = -err; goto fail; }
        }
        /* Keep the connected socket non-blocking for the benchmark data plane.
         * The stream helpers below enforce their own inactivity deadlines with
         * poll(), so they must not be able to block indefinitely inside
         * zsock_send()/zsock_recv() when SO_SNDTIMEO/SO_RCVTIMEO are rejected
         * by a socket provider/offload.
         */
        LOG_DBG("APP DATA PLANE fd=%d mode=nonblocking", fd);
    }

#if defined(CONFIG_BENCH_STCP_BLOCKING_CONNECT)
    if (cfg->transport == BENCH_TRANSPORT_STCP) {
        /* AF_STCP socket offload does not implement F_GETFL/F_SETFL.
         * Keep STCP in blocking mode after a successful blocking connect.
         * Native TCP continues to use the non-blocking + poll data plane.
         */
        LOG_DBG("APP DATA PLANE fd=%d transport=stcp mode=blocking "
                "nonblock_setup=skipped feature=BENCH_STCP_BLOCKING_CONNECT",
                fd);
    }
#endif

    if (resolved_with_dns) {
        zsock_freeaddrinfo(res);
    }
    return fd;
fail:
    LOG_ERR("APP CONNECT FAIL fd=%d rc=%d errno=%d", fd, rc, errno);
    zsock_close(fd);
    if (resolved_with_dns) {
        zsock_freeaddrinfo(res);
    }
    return rc;
}

static int connect_server(const struct bench_config *cfg)
{
    memset(&raw_tx, 0, sizeof(raw_tx));
    memset(&raw_rx, 0, sizeof(raw_rx));
    uint64_t start = clock_us();
    int fd = connect_server_impl(cfg);
    raw_connect_us = clock_us() - start;
    return fd;
}

static int send_metrics(int fd)
{
    struct ben2_metrics m = {
        .carrier =
#if defined(CONFIG_STCP_TRANSPORT_ETHERNET)
            BEN2_CARRIER_ETHERNET,
#else
            BEN2_CARRIER_LTE,
#endif
        .tx_bytes = raw_tx.bytes, .rx_bytes = raw_rx.bytes,
        .tx_us = raw_tx.elapsed_us, .rx_us = raw_rx.elapsed_us,
        .send_calls = raw_tx.calls, .recv_calls = raw_rx.calls,
        .eagain = raw_tx.eagain + raw_rx.eagain,
        .errors = raw_tx.errors + raw_rx.errors,
        .connect_us = raw_connect_us,
        .elapsed_us = raw_elapsed_us,
    };
#if defined(CONFIG_THREAD_STACK_INFO) && defined(CONFIG_INIT_STACKS)
    size_t unused = 0;
    if (k_thread_stack_space_get(k_current_get(), &unused) == 0) {
        size_t size = k_current_get()->stack_info.size;
        m.stack_free_bytes = unused;
        m.stack_used_bytes = size >= unused ? size - unused : 0;
    }
#endif
    uint8_t record[BEN2_METRICS_SIZE];
    ben2_metrics_encode(record, &m);
    return send_all(fd, record, sizeof(record));
}

static void fill_pattern(uint8_t *buf, size_t len, uint32_t offset, uint8_t salt)
{
    for (size_t i = 0; i < len; i++) buf[i] = (uint8_t)(((offset + i) * 31U + salt) & 0xffU);
}

static int verify_pattern(const uint8_t *buf, size_t len, uint32_t offset, uint8_t salt)
{
    for (size_t i = 0; i < len; i++)
        if (buf[i] != (uint8_t)(((offset + i) * 31U + salt) & 0xffU)) return -EBADMSG;
    return 0;
}

static int send_request(int fd, uint32_t mode, const struct bench_config *cfg)
{
    struct bench_request r = {
        .magic = sys_cpu_to_be32(BENCH_MAGIC), .version = sys_cpu_to_be32(BENCH_VERSION),
        .mode = sys_cpu_to_be32(mode), .chunk_size = sys_cpu_to_be32(cfg->chunk_size),
        .total_bytes = sys_cpu_to_be32(cfg->total_bytes),
    };

    LOG_DBG("BENCHCHK R01 SEND_ALL ENTER fd=%d mode=%u len=%u",
            fd, mode, (unsigned int)sizeof(r));
    int rc = send_all(fd, &r, sizeof(r));
    LOG_DBG("BENCHCHK R02 SEND_ALL RETURN fd=%d mode=%u rc=%d",
            fd, mode, rc);
    return rc;
}

static int recv_reply(int fd, uint32_t mode, struct bench_reply *reply)
{
    LOG_DBG("BENCHCHK P01 RECV_ALL ENTER fd=%d mode=%u len=%u",
            fd, mode, (unsigned int)sizeof(*reply));
    int rc = recv_all(fd, reply, sizeof(*reply));
    LOG_DBG("BENCHCHK P02 RECV_ALL RETURN fd=%d mode=%u rc=%d",
            fd, mode, rc);

    if (rc < 0) {
        return rc;
    }

    if (sys_be32_to_cpu(reply->magic) != BENCH_MAGIC ||
        sys_be32_to_cpu(reply->mode) != mode) {
        LOG_DBG("BENCHCHK P03 REPLY INVALID fd=%d mode=%u magic=0x%08x reply_mode=%u",
                fd, mode,
                (unsigned int)sys_be32_to_cpu(reply->magic),
                (unsigned int)sys_be32_to_cpu(reply->mode));
        return -EBADMSG;
    }

    rc = -(int)sys_be32_to_cpu(reply->status);
    LOG_DBG("BENCHCHK P04 REPLY VALID fd=%d mode=%u status_rc=%d rx=%u tx=%u",
            fd, mode, rc,
            (unsigned int)sys_be32_to_cpu(reply->bytes_received),
            (unsigned int)sys_be32_to_cpu(reply->bytes_sent));
    return rc;
}

static const char *result_status(int status)
{
    return status == 0 ? "OK" : "FAILED";
}

#if defined(CONFIG_NRF_MODEM_LIB)
static const char *rat_name(int act)
{
    switch (act) {
    case 7: return "LTE-M";
    case 9: return "NB-IoT";
    default: return "unknown";
    }
}

#endif

static void log_result_line(const char *name, const struct bench_result *r)
{
    LOG_INF("%s status=%s(%d) elapsed=%lld ms TX=%u bytes RX=%u bytes; rates on server",
            name, result_status(r->status), r->status, r->elapsed_ms,
            r->bytes_tx, r->bytes_rx);
}

void bench_print_last_summary(const struct bench_config *cfg)
{
    LOG_INF("================ BENCHMARK SUMMARY ================");
    LOG_INF("Transport=%s server=%s:%s chunk=%u total=%u bytes",
            bench_transport_name(cfg->transport), cfg->host, cfg->port,
            cfg->chunk_size, cfg->total_bytes);
#if defined(CONFIG_NRF_MODEM_LIB)
    {
        struct modem_status_snapshot modem;
        bool modem_ok = modem_status_get_snapshot(&modem) == 0;

        if (modem_ok) {
            LOG_INF("Modem operator=%s RAT=%s band=%d RSRP=%d dBm RSRQ=%d.%d dB SNR_raw=%d APN=%s",
                    modem.monitor_valid ? modem.operator_name : "unknown",
                    rat_name(modem.current_act), modem.band,
                    modem.rsrp_dbm, modem.rsrq_tenths_db / 10,
                    abs(modem.rsrq_tenths_db % 10), modem.snr_raw,
                    modem.pdp_valid ? modem.apn : "unknown");
        } else {
            LOG_WRN("Modem status snapshot unavailable");
        }
    }
#else
    LOG_INF("Network backend=Ethernet/native Zephyr stack");
#endif
    log_result_line("UPLOAD", &last_summary.upload);
    log_result_line("DOWNLOAD", &last_summary.download);
    log_result_line("FULL", &last_summary.full);
    LOG_INF("===================================================");
}

static int stream_send(int fd, uint8_t *tx_buf, uint32_t total, uint32_t chunk_size,
                       uint8_t salt, const char *label)
{
    ARG_UNUSED(label);
    uint32_t off = 0;
    int rc = 0;
    uint64_t started = clock_us();
    int64_t deadline = k_uptime_get() + active_cfg->timeout_ms;
    while (off < total) {
        size_t count = MIN(chunk_size, total - off);
        fill_pattern(tx_buf, count, off, salt);
        size_t position = 0;
        while (position < count) {
            size_t wanted = count - position;
            if (active_cfg->transport == BENCH_TRANSPORT_STCP)
                wanted = MIN(wanted, (size_t)1024U);
            ssize_t n = zsock_send(fd, tx_buf + position, wanted, 0);
            int saved_errno = n < 0 ? errno : 0;
            raw_tx.calls++;
            if (n > 0) {
                position += (size_t)n; off += (uint32_t)n;
                raw_tx.bytes += (uint64_t)n;
                deadline = k_uptime_get() + active_cfg->timeout_ms;
                continue;
            }
            if (!n) { rc = -ECONNRESET; goto done; }
            if (saved_errno == EINTR) continue;
            if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
                raw_tx.eagain++;
                rc = wait_socket_ready(fd, ZSOCK_POLLOUT, deadline);
                if (rc < 0) goto done;
                continue;
            }
            rc = saved_errno ? -saved_errno : -EIO;
            goto done;
        }
    }
done:
    raw_tx.elapsed_us = clock_us() - started;
    if (rc < 0) raw_tx.errors++;
    return rc;
}

static int stream_recv(int fd, uint8_t *rx_buf, uint32_t total, uint32_t rx_buf_size,
                       uint8_t salt, const char *label)
{
    ARG_UNUSED(label);
    uint32_t off = 0;
    int rc = 0;
    uint64_t started = clock_us();
    int64_t deadline = k_uptime_get() + active_cfg->timeout_ms;
    while (off < total) {
        size_t wanted = MIN(rx_buf_size, total - off);
        ssize_t n = zsock_recv(fd, rx_buf, wanted, 0);
        int saved_errno = n < 0 ? errno : 0;
        raw_rx.calls++;
        if (n > 0) {
            raw_rx.bytes += (uint64_t)n;
            rc = verify_pattern(rx_buf, (size_t)n, off, salt);
            if (rc < 0) goto done;
            off += (uint32_t)n;
            deadline = k_uptime_get() + active_cfg->timeout_ms;
            continue;
        }
        if (!n) { rc = -ECONNRESET; goto done; }
        if (saved_errno == EINTR) continue;
        if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
            raw_rx.eagain++;
            rc = wait_socket_ready(fd, ZSOCK_POLLIN, deadline);
            if (rc < 0) goto done;
            continue;
        }
        rc = saved_errno ? -saved_errno : -EIO;
        goto done;
    }
done:
    raw_rx.elapsed_us = clock_us() - started;
    if (rc < 0) raw_rx.errors++;
    return rc;
}

int bench_run_upload(const struct bench_config *cfg)
{
    memset(&last_summary.upload, 0, sizeof(last_summary.upload));
    last_summary.upload.status = -ECANCELED;
    int valid = validate_config(cfg);
    if (valid < 0) { last_summary.upload.status = valid; return valid; }
    struct bench_reply reply;
    active_cfg = cfg;
    uint8_t *tx_buf = k_malloc(cfg->chunk_size);
    if (!tx_buf) {
        LOG_ERR("UPLOAD buffer allocation failed: %u bytes", cfg->chunk_size);
        last_summary.upload.status = -ENOMEM;
        return -ENOMEM;
    }
    LOG_DBG("BENCHCHK U01 CONNECT ENTER");
    int fd = connect_server(cfg);
    LOG_DBG("BENCHCHK U02 CONNECT RETURN fd=%d", fd);
    if (fd < 0) {
        last_summary.upload.status = fd;
        k_free(tx_buf);
        return fd;
    }

    LOG_DBG("BENCHCHK U03 REQUEST SEND ENTER fd=%d len=%u",
            fd, (unsigned int)sizeof(struct bench_request));
    int rc = send_request(fd, BENCH_MODE_UPLOAD, cfg);
    LOG_DBG("BENCHCHK U04 REQUEST SEND RETURN fd=%d rc=%d", fd, rc);

    int64_t start = k_uptime_get();
    uint64_t data_start_us = clock_us();

    if (!rc) {
        LOG_DBG("BENCHCHK U05 UPLOAD STREAM ENTER fd=%d total=%u chunk=%u",
                fd, cfg->total_bytes, cfg->chunk_size);
        rc = stream_send(fd, tx_buf, cfg->total_bytes, cfg->chunk_size,
                         0x17, "UPLOAD TX");
        LOG_DBG("BENCHCHK U06 UPLOAD STREAM RETURN fd=%d rc=%d", fd, rc);
    }

    raw_elapsed_us = clock_us() - data_start_us;
    if (!rc) rc = send_metrics(fd);
    if (!rc) {
        LOG_DBG("BENCHCHK U07 REPLY RECV ENTER fd=%d len=%u",
                fd, (unsigned int)sizeof(struct bench_reply));
        rc = recv_reply(fd, BENCH_MODE_UPLOAD, &reply);
        LOG_DBG("BENCHCHK U08 REPLY RECV RETURN fd=%d rc=%d", fd, rc);
    }
    int64_t ms = MAX(k_uptime_get() - start, 1);
    zsock_close(fd);
    k_free(tx_buf);
    last_summary.upload.status = rc;
    last_summary.upload.bytes_tx = (uint32_t)raw_tx.bytes;
    last_summary.upload.elapsed_ms = ms;


    if (rc < 0) return rc;

    return 0;
}

int bench_run_download(const struct bench_config *cfg)
{
    memset(&last_summary.download, 0, sizeof(last_summary.download));
    last_summary.download.status = -ECANCELED;
    int valid = validate_config(cfg);
    if (valid < 0) { last_summary.download.status = valid; return valid; }
    struct bench_reply reply;
    active_cfg = cfg;
    uint8_t *rx_buf = k_malloc(cfg->chunk_size);
    if (!rx_buf) {
        LOG_ERR("DOWNLOAD buffer allocation failed: %u bytes", cfg->chunk_size);
        last_summary.download.status = -ENOMEM;
        return -ENOMEM;
    }
    int fd = connect_server(cfg);
    if (fd < 0) { last_summary.download.status = fd; k_free(rx_buf); return fd; }
    int rc = send_request(fd, BENCH_MODE_DOWNLOAD, cfg);
    int64_t start = k_uptime_get();
    uint64_t data_start_us = clock_us();
    if (!rc) rc = stream_recv(fd, rx_buf, cfg->total_bytes, cfg->chunk_size, 0x53, "DOWNLOAD RX");
    raw_elapsed_us = clock_us() - data_start_us;
    if (!rc) {
        struct bench_reply ack = {.magic=sys_cpu_to_be32(BENCH_MAGIC), .mode=sys_cpu_to_be32(BENCH_MODE_DOWNLOAD), .status=0,
            .bytes_received=sys_cpu_to_be32(cfg->total_bytes), .bytes_sent=0};
        rc = send_all(fd, &ack, sizeof(ack));
    }
    if (!rc) rc = send_metrics(fd);
    if (!rc) rc = recv_reply(fd, BENCH_MODE_DOWNLOAD, &reply);
    int64_t ms = MAX(k_uptime_get() - start, 1);
    zsock_close(fd);
    k_free(rx_buf);
    last_summary.download.status = rc;
    last_summary.download.bytes_rx = (uint32_t)raw_rx.bytes;
    last_summary.download.elapsed_ms = ms;


    if (rc < 0) return rc;

    return 0;
}

struct sender_ctx { int fd; int rc; uint8_t *tx_buf; uint32_t total; uint32_t chunk; struct k_sem done; };
K_THREAD_STACK_DEFINE(sender_stack, CONFIG_BENCH_SENDER_STACK_SIZE);
static struct k_thread sender_thread;

static void sender_entry(void *a, void *b, void *c)
{
    ARG_UNUSED(b); ARG_UNUSED(c);
    struct sender_ctx *ctx = a;
    ctx->rc = stream_send(ctx->fd, ctx->tx_buf, ctx->total, ctx->chunk, 0x17, "FULL TX");
    k_sem_give(&ctx->done);
}

int bench_run_full(const struct bench_config *cfg)
{
    memset(&last_summary.full, 0, sizeof(last_summary.full));
    last_summary.full.status = -ECANCELED;
    int valid = validate_config(cfg);
    if (valid < 0) { last_summary.full.status = valid; return valid; }
    struct bench_reply reply;
    struct sender_ctx ctx;
    active_cfg = cfg;
    const uint32_t rx_chunk = MIN(cfg->chunk_size, (uint32_t)CONFIG_BENCH_FULL_RX_BUFFER_SIZE);
    uint8_t *tx_buf = k_malloc(cfg->chunk_size);
    uint8_t *rx_buf = k_malloc(rx_chunk);
    if (!tx_buf || !rx_buf) {
        LOG_ERR("FULL buffers allocation failed: TX=%u RX=%u bytes",
                cfg->chunk_size, rx_chunk);
        k_free(tx_buf);
        k_free(rx_buf);
        last_summary.full.status = -ENOMEM;
        return -ENOMEM;
    }
    int fd = connect_server(cfg);
    if (fd < 0) { last_summary.full.status = fd; k_free(tx_buf); k_free(rx_buf); return fd; }
    int rc = send_request(fd, BENCH_MODE_FULL, cfg);
    if (rc < 0) { last_summary.full.status = rc; zsock_close(fd); k_free(tx_buf); k_free(rx_buf); return rc; }
    ctx.fd = fd; ctx.rc = 0; ctx.tx_buf = tx_buf; ctx.total = cfg->total_bytes; ctx.chunk = cfg->chunk_size; k_sem_init(&ctx.done, 0, 1);
    int64_t start = k_uptime_get();
    uint64_t data_start_us = clock_us();
    k_thread_create(&sender_thread, sender_stack, K_THREAD_STACK_SIZEOF(sender_stack), sender_entry,
                    &ctx, NULL, NULL, K_PRIO_PREEMPT(5), 0, K_NO_WAIT);
    rc = stream_recv(fd, rx_buf, cfg->total_bytes, rx_chunk, 0x53, "FULL RX");
    k_sem_take(&ctx.done, K_FOREVER);
    k_thread_join(&sender_thread, K_FOREVER);
    if (!rc) rc = ctx.rc;
    raw_elapsed_us = clock_us() - data_start_us;
    if (!rc) {
        struct bench_reply ack = {
            .magic = sys_cpu_to_be32(BENCH_MAGIC),
            .mode = sys_cpu_to_be32(BENCH_MODE_FULL),
            .status = 0,
            .bytes_received = sys_cpu_to_be32(cfg->total_bytes),
            .bytes_sent = sys_cpu_to_be32(cfg->total_bytes),
        };
        rc = send_all(fd, &ack, sizeof(ack));
    }
    if (!rc) rc = send_metrics(fd);
    if (!rc) rc = recv_reply(fd, BENCH_MODE_FULL, &reply);
    int64_t ms = MAX(k_uptime_get() - start, 1);
    zsock_close(fd);
    k_free(tx_buf);
    k_free(rx_buf);
    last_summary.full.status = rc;
    last_summary.full.bytes_tx = (uint32_t)raw_tx.bytes;
    last_summary.full.bytes_rx = (uint32_t)raw_rx.bytes;
    last_summary.full.elapsed_ms = ms;



    if (rc < 0) return rc;
    LOG_INF("FULL upload=%u download=%u elapsed=%lld ms", cfg->total_bytes, cfg->total_bytes, ms);

    return 0;
}

const struct bench_result *bench_get_last_result(const char *direction)
{
    if (!direction) return NULL;
    if (!strcmp(direction, "upload")) return &last_summary.upload;
    if (!strcmp(direction, "download")) return &last_summary.download;
    if (!strcmp(direction, "full")) return &last_summary.full;
    return NULL;
}

void bench_config_defaults(struct bench_config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    strncpy(cfg->host, CONFIG_ECHO_SERVER_HOST, sizeof(cfg->host) - 1);
    strncpy(cfg->port, CONFIG_ECHO_SERVER_PORT, sizeof(cfg->port) - 1);
    cfg->chunk_size = CONFIG_BENCH_CHUNK_SIZE;
    cfg->total_bytes = CONFIG_BENCH_TOTAL_BYTES;
    cfg->timeout_ms = CONFIG_ECHO_SOCKET_TIMEOUT_MS;
    cfg->report_interval_ms = CONFIG_BENCH_REPORT_INTERVAL_MS;
#if defined(CONFIG_ECHO_TRANSPORT_STCP)
    cfg->transport = BENCH_TRANSPORT_STCP;
#else
    cfg->transport = BENCH_TRANSPORT_TCP;
#endif
}

const char *bench_transport_name(enum bench_transport transport)
{
    switch (transport) {
    case BENCH_TRANSPORT_TCP: return "tcp";
    case BENCH_TRANSPORT_STCP: return "stcp";
    case BENCH_TRANSPORT_TLS: return "tls";
    default: return "unknown";
    }
}

static int validate_config(const struct bench_config *cfg)
{
    if (!cfg || !cfg->host[0] || !cfg->port[0]) return -EINVAL;
    if (cfg->chunk_size < 1 || cfg->chunk_size > BENCH_MAX_CHUNK) return -EINVAL;
    if (cfg->total_bytes < cfg->chunk_size || cfg->total_bytes > 256U * 1024U * 1024U || cfg->timeout_ms < 1000) return -EINVAL;
    if (cfg->report_interval_ms > 600000U) return -EINVAL;
    return 0;
}

int bench_run_all(const struct bench_config *cfg)
{
    int rc = validate_config(cfg);
    memset(&last_summary, 0, sizeof(last_summary));
    last_summary.upload.status = -ECANCELED;
    last_summary.download.status = -ECANCELED;
    last_summary.full.status = -ECANCELED;
    if (rc < 0) return rc;
    LOG_INF("Benchmark transport=%s host=%s port=%s total=%u chunk=%u",
            bench_transport_name(cfg->transport), cfg->host, cfg->port,
            cfg->total_bytes, cfg->chunk_size);
    LOG_INF("BEN2 v2: raw metrics sent to server; no progress logging during data transfer");
    rc = bench_run_upload(cfg);
    if (rc < 0) { bench_print_last_summary(cfg); return rc; }
    k_sleep(K_SECONDS(CONFIG_BENCH_PAUSE_SECONDS));
    rc = bench_run_download(cfg);
    if (rc < 0) { bench_print_last_summary(cfg); return rc; }
    k_sleep(K_SECONDS(CONFIG_BENCH_PAUSE_SECONDS));
    rc = bench_run_full(cfg);
    bench_print_last_summary(cfg);
    return rc;
}

int echo_benchmark_run(void)
{
    struct bench_config cfg;
    bench_config_defaults(&cfg);
    return bench_run_all(&cfg);
}
