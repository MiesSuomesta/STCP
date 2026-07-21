#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/byteorder.h>

#include <stcp/stcp.h>
#include "echo_benchmark.h"
#include "stcp_lte_transport.h"
#include "modem_status.h"

LOG_MODULE_REGISTER(echo_benchmark, LOG_LEVEL_INF);

#define BENCH_MAGIC 0x42454e32U /* BEN2 */
#define BENCH_VERSION 1U
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
        if (pfd.revents & ZSOCK_POLLNVAL) return -EBADF;
        if (pfd.revents & ZSOCK_POLLERR) return -EIO;
        if (pfd.revents & (events | ZSOCK_POLLHUP)) return 0;
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

static int set_blocking(int fd)
{
    int flags = zsock_fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -errno;
    return zsock_fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0 ? -errno : 0;
}

static int connect_server(const struct bench_config *cfg)
{
    struct zsock_addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};
    struct zsock_addrinfo *res = NULL;
    int fd, rc;

    rc = zsock_getaddrinfo(cfg->host, cfg->port, &hints, &res);
    if (rc || !res) return -EHOSTUNREACH;
if (cfg->transport == BENCH_TRANSPORT_STCP) {
        fd = zsock_socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);
    } else if (cfg->transport == BENCH_TRANSPORT_TCP) {
        fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else {
        zsock_freeaddrinfo(res);
        return -ENOTSUP;
    }
    if (fd < 0) { rc = -errno; zsock_freeaddrinfo(res); return rc; }

    if (cfg->transport == BENCH_TRANSPORT_TCP) {
        rc = stcp_lte_transport_bind_socket(fd);
        if (rc < 0) goto fail;
    }

    rc = set_nonblocking(fd);
    if (rc < 0) goto fail;
    if (zsock_connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        if (errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK) { rc = -errno; goto fail; }
        rc = wait_socket_ready(fd, ZSOCK_POLLOUT, k_uptime_get() + active_cfg->timeout_ms);
        if (rc < 0) goto fail;
        int err = 0; socklen_t sl = sizeof(err);
        if (zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &sl) < 0) { rc = -errno; goto fail; }
        if (err) { rc = -err; goto fail; }
    }
    rc = set_blocking(fd);
    if (rc < 0) goto fail;
    zsock_freeaddrinfo(res);
    return fd;
fail:
    zsock_close(fd); zsock_freeaddrinfo(res); return rc;
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
    return send_all(fd, &r, sizeof(r));
}

static int recv_reply(int fd, uint32_t mode, struct bench_reply *reply)
{
    int rc = recv_all(fd, reply, sizeof(*reply));
    if (rc < 0) return rc;
    if (sys_be32_to_cpu(reply->magic) != BENCH_MAGIC || sys_be32_to_cpu(reply->mode) != mode)
        return -EBADMSG;
    return -(int)sys_be32_to_cpu(reply->status);
}

static uint64_t bps(uint64_t bytes, int64_t elapsed_ms)
{
    return (bytes * 8ULL * 1000ULL) / (uint64_t)MAX(elapsed_ms, 1);
}


static const char *result_status(int status)
{
    return status == 0 ? "OK" : "FAILED";
}

static const char *rat_name(int act)
{
    switch (act) {
    case 7: return "LTE-M";
    case 9: return "NB-IoT";
    default: return "unknown";
    }
}

static void log_rate_line(const char *name, const struct bench_result *r)
{
    LOG_INF("%-10s status=%s(%d) elapsed=%lld ms TX=%llu bit/s RX=%llu bit/s aggregate=%llu bit/s",
            name, result_status(r->status), r->status, r->elapsed_ms,
            r->tx_bps, r->rx_bps, r->aggregate_bps);
}

void bench_print_last_summary(const struct bench_config *cfg)
{
    struct modem_status_snapshot modem;
    bool modem_ok = modem_status_get_snapshot(&modem) == 0;

    LOG_INF("================ BENCHMARK SUMMARY ================");
    LOG_INF("Transport=%s server=%s:%s chunk=%u total=%u bytes",
            bench_transport_name(cfg->transport), cfg->host, cfg->port,
            cfg->chunk_size, cfg->total_bytes);
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
    log_rate_line("UPLOAD", &last_summary.upload);
    log_rate_line("DOWNLOAD", &last_summary.download);
    log_rate_line("FULL", &last_summary.full);
    LOG_INF("===================================================");
}

static void report_progress(const char *label, uint32_t done, uint32_t total,
                            int64_t started_ms, int64_t *last_report_ms,
                            bool force)
{
    int64_t now = k_uptime_get();
    uint32_t remaining = total - done;
    uint32_t percent = total ? (uint32_t)(((uint64_t)done * 100ULL) / total) : 100U;
    int64_t elapsed = MAX(now - started_ms, 1);

    if (!force && active_cfg->report_interval_ms == 0) return;
    if (!force && now - *last_report_ms < active_cfg->report_interval_ms) return;

    LOG_INF("%s progress=%u%% transferred=%u/%u bytes remaining=%u rate=%llu bit/s",
            label, percent, done, total, remaining, bps(done, elapsed));
    *last_report_ms = now;
}

static int stream_send(int fd, uint8_t *tx_buf, uint32_t total, uint32_t chunk_size, uint8_t salt,
                       const char *label)
{
    uint32_t off = 0;
    uint32_t send_calls = 0;
    uint32_t partial_calls = 0;
    size_t max_send = 0;
    int64_t started = k_uptime_get();
    int64_t last_report = started;

    while (off < total) {
        size_t chunk_len = MIN(chunk_size, total - off);
        size_t chunk_off = 0;

        fill_pattern(tx_buf, chunk_len, off, salt);

        while (chunk_off < chunk_len) {
            size_t remaining = chunk_len - chunk_off;
            ssize_t n = zsock_send(fd, tx_buf + chunk_off, remaining, 0);
            send_calls++;

            if (n > 0) {
                if ((size_t)n < remaining) partial_calls++;
                if ((size_t)n > max_send) max_send = (size_t)n;
                chunk_off += (size_t)n;
                off += (uint32_t)n;
                report_progress(label, off, total, started, &last_report, off == total);
                continue;
            }
            if (n == 0) return -ECONNRESET;
            if (errno == EINTR) continue;
            return -errno;
        }
    }

    LOG_INF("%s I/O stats calls=%u partial=%u max_send=%u",
            label, send_calls, partial_calls, (uint32_t)max_send);
    return 0;
}

static int stream_recv(int fd, uint8_t *rx_buf, uint32_t total, uint32_t rx_buf_size, uint8_t salt,
                       const char *label)
{
    uint32_t off = 0;
    uint32_t recv_calls = 0;
    size_t max_recv = 0;
    int64_t started = k_uptime_get();
    int64_t last_report = started;

    while (off < total) {
        size_t wanted = MIN(rx_buf_size, total - off);
        ssize_t n = zsock_recv(fd, rx_buf, wanted, 0);
        recv_calls++;

        if (n > 0) {
            int rc = verify_pattern(rx_buf, (size_t)n, off, salt);
            if (rc < 0) return rc;
            if ((size_t)n > max_recv) max_recv = (size_t)n;
            off += (uint32_t)n;
            report_progress(label, off, total, started, &last_report, off == total);
            continue;
        }
        if (n == 0) return -ECONNRESET;
        if (errno == EINTR) continue;
        return -errno;
    }

    LOG_INF("%s I/O stats calls=%u max_recv=%u", label, recv_calls, (uint32_t)max_recv);
    return 0;
}

int bench_run_upload(const struct bench_config *cfg)
{
    memset(&last_summary.upload, 0, sizeof(last_summary.upload));
    last_summary.upload.status = -ECANCELED;
    int valid = validate_config(cfg);
    if (valid < 0) return valid;
    struct bench_reply reply;
    active_cfg = cfg;
    uint8_t *tx_buf = k_malloc(cfg->chunk_size);
    if (!tx_buf) {
        LOG_ERR("UPLOAD buffer allocation failed: %u bytes", cfg->chunk_size);
        return -ENOMEM;
    }
    int fd = connect_server(cfg);
    if (fd < 0) { k_free(tx_buf); return fd; }
    int rc = send_request(fd, BENCH_MODE_UPLOAD, cfg);
    int64_t start = k_uptime_get();
    if (!rc) rc = stream_send(fd, tx_buf, cfg->total_bytes, cfg->chunk_size, 0x17, "UPLOAD TX");
    if (!rc) rc = recv_reply(fd, BENCH_MODE_UPLOAD, &reply);
    int64_t ms = MAX(k_uptime_get() - start, 1);
    zsock_close(fd);
    k_free(tx_buf);
    last_summary.upload.status = rc;
    last_summary.upload.bytes_tx = rc < 0 ? 0 : cfg->total_bytes;
    last_summary.upload.elapsed_ms = ms;
    last_summary.upload.tx_bps = rc < 0 ? 0 : bps(cfg->total_bytes, ms);
    last_summary.upload.aggregate_bps = last_summary.upload.tx_bps;
    if (rc < 0) return rc;
    LOG_INF("UPLOAD bytes=%u elapsed=%lld ms throughput=%llu bit/s", cfg->total_bytes, ms, last_summary.upload.tx_bps);
    return 0;
}

int bench_run_download(const struct bench_config *cfg)
{
    memset(&last_summary.download, 0, sizeof(last_summary.download));
    last_summary.download.status = -ECANCELED;
    int valid = validate_config(cfg);
    if (valid < 0) return valid;
    struct bench_reply reply;
    active_cfg = cfg;
    uint8_t *rx_buf = k_malloc(cfg->chunk_size);
    if (!rx_buf) {
        LOG_ERR("DOWNLOAD buffer allocation failed: %u bytes", cfg->chunk_size);
        return -ENOMEM;
    }
    int fd = connect_server(cfg);
    if (fd < 0) { k_free(rx_buf); return fd; }
    int rc = send_request(fd, BENCH_MODE_DOWNLOAD, cfg);
    int64_t start = k_uptime_get();
    if (!rc) rc = stream_recv(fd, rx_buf, cfg->total_bytes, cfg->chunk_size, 0x53, "DOWNLOAD RX");
    if (!rc) {
        struct bench_reply ack = {.magic=sys_cpu_to_be32(BENCH_MAGIC), .mode=sys_cpu_to_be32(BENCH_MODE_DOWNLOAD), .status=0,
            .bytes_received=sys_cpu_to_be32(cfg->total_bytes), .bytes_sent=0};
        rc = send_all(fd, &ack, sizeof(ack));
    }
    if (!rc) rc = recv_reply(fd, BENCH_MODE_DOWNLOAD, &reply);
    int64_t ms = MAX(k_uptime_get() - start, 1);
    zsock_close(fd);
    k_free(rx_buf);
    last_summary.download.status = rc;
    last_summary.download.bytes_rx = rc < 0 ? 0 : cfg->total_bytes;
    last_summary.download.elapsed_ms = ms;
    last_summary.download.rx_bps = rc < 0 ? 0 : bps(cfg->total_bytes, ms);
    last_summary.download.aggregate_bps = last_summary.download.rx_bps;
    if (rc < 0) return rc;
    LOG_INF("DOWNLOAD bytes=%u elapsed=%lld ms throughput=%llu bit/s", cfg->total_bytes, ms, last_summary.download.rx_bps);
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
    if (valid < 0) return valid;
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
        return -ENOMEM;
    }
    int fd = connect_server(cfg);
    if (fd < 0) { k_free(tx_buf); k_free(rx_buf); return fd; }
    int rc = send_request(fd, BENCH_MODE_FULL, cfg);
    if (rc < 0) { zsock_close(fd); k_free(tx_buf); k_free(rx_buf); return rc; }
    ctx.fd = fd; ctx.rc = 0; ctx.tx_buf = tx_buf; ctx.total = cfg->total_bytes; ctx.chunk = cfg->chunk_size; k_sem_init(&ctx.done, 0, 1);
    int64_t start = k_uptime_get();
    k_thread_create(&sender_thread, sender_stack, K_THREAD_STACK_SIZEOF(sender_stack), sender_entry,
                    &ctx, NULL, NULL, K_PRIO_PREEMPT(5), 0, K_NO_WAIT);
    rc = stream_recv(fd, rx_buf, cfg->total_bytes, rx_chunk, 0x53, "FULL RX");
    k_sem_take(&ctx.done, K_FOREVER);
    if (!rc) rc = ctx.rc;
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
    if (!rc) rc = recv_reply(fd, BENCH_MODE_FULL, &reply);
    int64_t ms = MAX(k_uptime_get() - start, 1);
    zsock_close(fd);
    k_free(tx_buf);
    k_free(rx_buf);
    last_summary.full.status = rc;
    last_summary.full.bytes_tx = rc < 0 ? 0 : cfg->total_bytes;
    last_summary.full.bytes_rx = rc < 0 ? 0 : cfg->total_bytes;
    last_summary.full.elapsed_ms = ms;
    last_summary.full.tx_bps = rc < 0 ? 0 : bps(cfg->total_bytes, ms);
    last_summary.full.rx_bps = last_summary.full.tx_bps;
    last_summary.full.aggregate_bps = rc < 0 ? 0 : bps((uint64_t)cfg->total_bytes * 2ULL, ms);
    if (rc < 0) return rc;
    LOG_INF("FULL upload=%u download=%u elapsed=%lld ms", cfg->total_bytes, cfg->total_bytes, ms);
    LOG_INF("FULL TX=%llu bit/s RX=%llu bit/s aggregate=%llu bit/s",
            last_summary.full.tx_bps, last_summary.full.rx_bps,
            last_summary.full.aggregate_bps);
    return 0;
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
    if (cfg->total_bytes < cfg->chunk_size || cfg->timeout_ms < 1000) return -EINVAL;
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
    LOG_INF("Progress report interval=%u ms", cfg->report_interval_ms);
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
