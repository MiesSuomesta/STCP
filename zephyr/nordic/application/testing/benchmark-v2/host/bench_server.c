#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#ifndef AF_STCP
#define AF_STCP 45
#endif
#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#define BENCH_MAGIC   0x42454e32U /* BEN2 */
#define BENCH_VERSION 1U
#define MODE_UPLOAD   1U
#define MODE_DOWNLOAD 2U
#define MODE_FULL     3U
#define MAX_CHUNK     (1024U * 1024U)
#define MAX_TOTAL     (1024U * 1024U * 1024U)

struct __attribute__((packed)) bench_request {
    uint32_t magic;
    uint32_t version;
    uint32_t mode;
    uint32_t chunk_size;
    uint32_t total_bytes;
};

struct __attribute__((packed)) bench_reply {
    uint32_t magic;
    uint32_t mode;
    uint32_t status;
    uint32_t bytes_received;
    uint32_t bytes_sent;
};

static volatile sig_atomic_t stop_flag;

#define PROGRESS_STEP (1024U * 1024U)
#define STCP_TX_SLICE 1024U

static long long mono_us(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return -1;
    return (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
}

static void on_signal(int sig)
{
    (void)sig;
    stop_flag = 1;
}

static const char *mode_name(uint32_t mode)
{
    switch (mode) {
    case MODE_UPLOAD: return "upload";
    case MODE_DOWNLOAD: return "download";
    case MODE_FULL: return "full";
    default: return "unknown";
    }
}

static uint8_t pattern(uint32_t offset, uint8_t salt)
{
    return (uint8_t)(((offset * 31U) + salt) & 0xffU);
}

static int read_all(int fd, void *buf, size_t len)
{
    uint8_t *p = buf;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(fd, p + off, len - off);
        if (n > 0) {
            off += (size_t)n;
            continue;
        }
        if (n == 0)
            return -ECONNRESET;
        if (errno == EINTR)
            continue;
        return -errno;
    }
    return 0;
}

static int write_all_limited(int fd, const void *buf, size_t len, size_t max_write)
{
    const uint8_t *p = buf;
    size_t off = 0;

    while (off < len) {
        size_t want = len - off;
        if (max_write != 0 && want > max_write)
            want = max_write;

        ssize_t n = send(fd, p + off, want, MSG_NOSIGNAL);
        if (n > 0) {
            off += (size_t)n;
            continue;
        }
        if (n == 0)
            return -EPIPE;
        if (errno == EINTR)
            continue;
        return -errno;
    }
    return 0;
}

static int write_all(int fd, const void *buf, size_t len)
{
    return write_all_limited(fd, buf, len, 0);
}

static int receive_pattern(int fd, uint32_t total, uint32_t chunk, uint8_t salt,
                           uint32_t *received, unsigned long long cid)
{
    uint8_t *buf = malloc(chunk);
    uint32_t off = 0;
    uint32_t next_progress = PROGRESS_STEP;
    int rc = 0;

    if (!buf)
        return -ENOMEM;

    while (off < total) {
        size_t want = chunk;
        if (want > total - off)
            want = total - off;

        ssize_t n = read(fd, buf, want);
        if (n <= 0) {
            rc = n == 0 ? -ECONNRESET : -errno;
            break;
        }

        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] != pattern(off + (uint32_t)i, salt)) {
                rc = -EBADMSG;
                goto out;
            }
        }
        off += (uint32_t)n;
        while (off >= next_progress && next_progress <= total) {
            fprintf(stderr,
                    "BENCH_SERVER_RX cid=%llu bytes=%u total=%u\n",
                    cid, off, total);
            next_progress += PROGRESS_STEP;
        }
    }

out:
    free(buf);
    if (received)
        *received = off;
    return rc;
}

static int send_pattern(int fd, uint32_t total, uint32_t chunk, uint8_t salt,
                        uint32_t *sent, size_t max_write)
{
    uint8_t *buf = malloc(chunk);
    uint32_t off = 0;
    int rc = 0;

    if (!buf)
        return -ENOMEM;

    while (off < total) {
        uint32_t n = chunk;
        if (n > total - off)
            n = total - off;

        for (uint32_t i = 0; i < n; i++)
            buf[i] = pattern(off + i, salt);

        rc = write_all_limited(fd, buf, n, max_write);
        if (rc < 0)
            break;
        off += n;
    }

    free(buf);
    if (sent)
        *sent = off;
    return rc;
}

static int send_reply(int fd, uint32_t mode, int status,
                      uint32_t bytes_received, uint32_t bytes_sent)
{
    struct bench_reply r = {
        .magic = htonl(BENCH_MAGIC),
        .mode = htonl(mode),
        .status = htonl((uint32_t)(status < 0 ? -status : status)),
        .bytes_received = htonl(bytes_received),
        .bytes_sent = htonl(bytes_sent),
    };
    return write_all(fd, &r, sizeof(r));
}

struct tx_job {
    int fd;
    uint32_t total;
    uint32_t chunk;
    uint8_t salt;
    uint32_t sent;
    size_t max_write;
    int rc;
};

static void *tx_worker(void *arg)
{
    struct tx_job *job = arg;
    job->rc = send_pattern(job->fd, job->total, job->chunk, job->salt,
                           &job->sent, job->max_write);
    return NULL;
}

static int handle_client(int fd, unsigned long long cid, const char *transport)
{
    struct bench_request q;
    struct bench_reply ack;
    uint32_t magic, version, mode, chunk, total;
    uint32_t br = 0, bs = 0;
    long long start_us, end_us;
    int rc;
    size_t data_tx_limit = !strcmp(transport, "stcp-tcp") ? STCP_TX_SLICE : 0;

    fprintf(stderr, "BENCH_SERVER_CLIENT cid=%llu state=request_wait\n", cid);

    rc = read_all(fd, &q, sizeof(q));
    if (rc < 0) {
        fprintf(stderr,
                "BENCH_SERVER_REQUEST_ERROR cid=%llu rc=%d errno=%d\n",
                cid, rc, errno);
        return rc;
    }

    magic = ntohl(q.magic);
    version = ntohl(q.version);
    mode = ntohl(q.mode);
    chunk = ntohl(q.chunk_size);
    total = ntohl(q.total_bytes);

    fprintf(stderr,
            "BENCH_SERVER_REQUEST cid=%llu magic=0x%08x version=%u "
            "direction=%s mode=%u total=%u chunk=%u\n",
            cid, magic, version, mode_name(mode), mode, total, chunk);

    if (magic != BENCH_MAGIC || version != BENCH_VERSION ||
        chunk == 0 || chunk > MAX_CHUNK || total == 0 || total > MAX_TOTAL) {
        fprintf(stderr,
                "BENCH_SERVER_REQUEST_REJECT cid=%llu magic=0x%08x version=%u "
                "mode=%u total=%u chunk=%u\n",
                cid, magic, version, mode, total, chunk);
        return -EPROTO;
    }

    start_us = mono_us();

    switch (mode) {
    case MODE_UPLOAD:
        rc = receive_pattern(fd, total, chunk, 0x17, &br, cid);
        if (rc == 0)
            rc = send_reply(fd, mode, 0, br, 0);
        break;

    case MODE_DOWNLOAD:
        if (data_tx_limit != 0)
            fprintf(stderr,
                    "BENCH_SERVER_STCP_TX_SLICE cid=%llu bytes=%zu app_chunk=%u total=%u\n",
                    cid, data_tx_limit, chunk, total);
        rc = send_pattern(fd, total, chunk, 0x53, &bs, data_tx_limit);
        if (rc == 0)
            rc = read_all(fd, &ack, sizeof(ack));
        if (rc == 0)
            rc = send_reply(fd, mode, 0, 0, bs);
        break;

    case MODE_FULL: {
        pthread_t tx;
        struct tx_job job = {
            .fd = fd, .total = total, .chunk = chunk, .salt = 0x53,
            .sent = 0, .max_write = data_tx_limit, .rc = 0,
        };

        if (data_tx_limit != 0)
            fprintf(stderr,
                    "BENCH_SERVER_STCP_TX_SLICE cid=%llu bytes=%zu app_chunk=%u total=%u\n",
                    cid, data_tx_limit, chunk, total);

        if (pthread_create(&tx, NULL, tx_worker, &job) != 0)
            return -errno;

        rc = receive_pattern(fd, total, chunk, 0x17, &br, cid);
        if (pthread_join(tx, NULL) != 0 && rc == 0)
            rc = -EIO;
        bs = job.sent;
        if (rc == 0 && job.rc < 0)
            rc = job.rc;
        if (rc == 0)
            rc = read_all(fd, &ack, sizeof(ack));
        if (rc == 0)
            rc = send_reply(fd, mode, 0, br, bs);
        break;
    }

    default:
        rc = -EINVAL;
        break;
    }

    end_us = mono_us();
    fprintf(stderr,
            "BENCH_SERVER_RESULT cid=%llu transport=%s direction=%s "
            "status=%d total=%u chunk=%u bytes_rx=%u bytes_tx=%u elapsed_us=%lld\n",
            cid, transport, mode_name(mode), rc, total, chunk, br, bs,
            end_us >= start_us ? end_us - start_us : -1);
    fflush(stderr);
    return rc;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "usage: %s --transport tcp|stcp-tcp [--bind IP] [--port PORT]\n",
            argv0);
}

int main(int argc, char **argv)
{
    const char *transport = NULL;
    const char *bind_ip = "0.0.0.0";
    int port = 19000;
    int family = AF_INET;
    int protocol = 0;
    int one = 1;
    int listener;
    unsigned long long cid = 0;
    struct sockaddr_in a;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--transport") && i + 1 < argc) {
            transport = argv[++i];
        } else if (!strcmp(argv[i], "--bind") && i + 1 < argc) {
            bind_ip = argv[++i];
        } else if (!strcmp(argv[i], "--port") && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--help")) {
            usage(argv[0]);
            return 0;
        } else {
            usage(argv[0]);
            return 2;
        }
    }

    if (!transport || port < 1 || port > 65535) {
        usage(argv[0]);
        return 2;
    }

    if (!strcmp(transport, "tcp")) {
        family = AF_INET;
        protocol = 0;
    } else if (!strcmp(transport, "stcp-tcp")) {
        family = AF_STCP;
        protocol = IPPROTO_STCP;
    } else {
        fprintf(stderr, "unsupported transport: %s\n", transport);
        return 2;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGPIPE, SIG_IGN);
    setvbuf(stderr, NULL, _IONBF, 0);

    listener = socket(family, SOCK_STREAM, protocol);
    if (listener < 0) {
        perror("socket");
        return 2;
    }

    (void)setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET; /* AF_STCP currently consumes sockaddr_in endpoints. */
    a.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, bind_ip, &a.sin_addr) != 1) {
        fprintf(stderr, "invalid bind address: %s\n", bind_ip);
        close(listener);
        return 2;
    }

    if (bind(listener, (struct sockaddr *)&a, sizeof(a)) < 0) {
        perror("bind");
        close(listener);
        return 2;
    }
    if (listen(listener, 16) < 0) {
        perror("listen");
        close(listener);
        return 2;
    }

    fprintf(stderr,
            "BENCH_SERVER_READY transport=%s bind=%s port=%d pid=%ld\n",
            transport, bind_ip, port, (long)getpid());

    while (!stop_flag) {
        int fd = accept(listener, NULL, NULL);
        if (fd < 0) {
            if (errno == EINTR)
                continue;
            if (!strcmp(transport, "stcp-tcp") &&
                (errno == ETIMEDOUT || errno == EAGAIN || errno == EWOULDBLOCK))
                continue;
            perror("accept");
            break;
        }

        cid++;
        fprintf(stderr,
                "BENCH_SERVER_ACCEPT cid=%llu transport=%s fd=%d\n",
                cid, transport, fd);
        int rc = handle_client(fd, cid, transport);
        if (rc < 0)
            fprintf(stderr, "BENCH_SERVER_ERROR cid=%llu rc=%d errno=%d\n", cid, rc, errno);
        close(fd);
    }

    close(listener);
    return 0;
}
