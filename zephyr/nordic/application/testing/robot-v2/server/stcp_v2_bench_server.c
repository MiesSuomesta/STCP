#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
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
#define BENCH_MAGIC 0x42454e32U
#define BENCH_VERSION 1U
#define MODE_UPLOAD 1U
#define MODE_DOWNLOAD 2U
#define MODE_FULL 3U

struct __attribute__((packed)) request {
    uint32_t magic, version, mode, chunk_size, total_bytes;
};
struct __attribute__((packed)) reply {
    uint32_t magic, mode, status, bytes_received, bytes_sent;
};

static volatile sig_atomic_t stop_flag;
static unsigned long long dbg_seq;
static unsigned long long conn_seq;

static long long mono_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return -1;
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

#define DBG(fmt, ...) do { \
    fprintf(stderr, "[SERVERDBG #%06llu t=%lld] " fmt "\n", \
            ++dbg_seq, mono_ms(), ##__VA_ARGS__); \
    fflush(stderr); \
} while (0)

static void on_signal(int sig)
{
    DBG("SIGNAL sig=%d stop=1", sig);
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

static void dump_endpoint(int fd, const char *what, unsigned long long cid)
{
    struct sockaddr_in a;
    socklen_t len = sizeof(a);
    char ip[INET_ADDRSTRLEN] = "?";

    memset(&a, 0, sizeof(a));
    if ((strcmp(what, "LOCAL") == 0 ? getsockname(fd, (struct sockaddr *)&a, &len)
                                    : getpeername(fd, (struct sockaddr *)&a, &len)) == 0) {
        if (!inet_ntop(AF_INET, &a.sin_addr, ip, sizeof(ip)))
            strcpy(ip, "?");
        DBG("CONN cid=%llu %s fd=%d addr=%s:%u", cid, what, fd, ip,
            (unsigned)ntohs(a.sin_port));
    } else {
        DBG("CONN cid=%llu %s fd=%d query_failed errno=%d(%s)", cid, what, fd,
            errno, strerror(errno));
    }
}

static int read_all_dbg(int fd, void *buf, size_t len, unsigned long long cid, const char *tag)
{
    uint8_t *p = buf;
    size_t off = 0;
    unsigned calls = 0;

    DBG("IO cid=%llu %s READ_ALL ENTER fd=%d len=%zu", cid, tag, fd, len);
    while (off < len) {
        ssize_t n = read(fd, p + off, len - off);
        calls++;
        if (n > 0) {
            off += (size_t)n;
            if (calls <= 3 || off == len)
                DBG("IO cid=%llu %s READ progress n=%zd off=%zu/%zu calls=%u",
                    cid, tag, n, off, len, calls);
            continue;
        }
        if (n == 0) {
            DBG("IO cid=%llu %s READ EOF off=%zu/%zu calls=%u", cid, tag, off, len, calls);
            return -ECONNRESET;
        }
        if (errno == EINTR)
            continue;
        DBG("IO cid=%llu %s READ ERROR off=%zu/%zu calls=%u errno=%d(%s)",
            cid, tag, off, len, calls, errno, strerror(errno));
        return -errno;
    }
    DBG("IO cid=%llu %s READ_ALL DONE bytes=%zu calls=%u", cid, tag, off, calls);
    return 0;
}

static int write_all_dbg(int fd, const void *buf, size_t len, unsigned long long cid, const char *tag)
{
    const uint8_t *p = buf;
    size_t off = 0;
    unsigned calls = 0;

    DBG("IO cid=%llu %s WRITE_ALL ENTER fd=%d len=%zu", cid, tag, fd, len);
    while (off < len) {
        ssize_t n = send(fd, p + off, len - off, MSG_NOSIGNAL);
        calls++;
        if (n > 0) {
            off += (size_t)n;
            if (calls <= 3 || off == len)
                DBG("IO cid=%llu %s WRITE progress n=%zd off=%zu/%zu calls=%u",
                    cid, tag, n, off, len, calls);
            continue;
        }
        if (n == 0) {
            DBG("IO cid=%llu %s WRITE ZERO off=%zu/%zu calls=%u", cid, tag, off, len, calls);
            return -EPIPE;
        }
        if (errno == EINTR)
            continue;
        DBG("IO cid=%llu %s WRITE ERROR off=%zu/%zu calls=%u errno=%d(%s)",
            cid, tag, off, len, calls, errno, strerror(errno));
        return -errno;
    }
    DBG("IO cid=%llu %s WRITE_ALL DONE bytes=%zu calls=%u", cid, tag, off, calls);
    return 0;
}

static uint8_t pattern(uint32_t offset, uint8_t salt)
{
    return (uint8_t)((offset * 31U + salt) & 0xffU);
}

static int receive_pattern(int fd, uint32_t total, uint32_t chunk, uint8_t salt,
                           unsigned long long cid)
{
    uint8_t *buf = malloc(chunk);
    uint32_t off = 0;
    uint32_t next_report = 0;
    unsigned calls = 0;
    int rc = 0;

    if (!buf) {
        DBG("BENCH cid=%llu UPLOAD malloc failed chunk=%u", cid, chunk);
        return -ENOMEM;
    }
    DBG("BENCH cid=%llu UPLOAD RX ENTER total=%u chunk=%u", cid, total, chunk);
    while (off < total) {
        size_t want = chunk;
        if (want > total - off)
            want = total - off;
        ssize_t n = read(fd, buf, want);
        calls++;
        if (n <= 0) {
            rc = n == 0 ? -ECONNRESET : -errno;
            DBG("BENCH cid=%llu UPLOAD RX FAIL n=%zd off=%u/%u calls=%u errno=%d(%s) rc=%d",
                cid, n, off, total, calls, errno, strerror(errno), rc);
            break;
        }
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] != pattern(off + (uint32_t)i, salt)) {
                rc = -EBADMSG;
                DBG("BENCH cid=%llu UPLOAD PATTERN FAIL off=%u index=%zd got=%02x expected=%02x",
                    cid, off, i, buf[i], pattern(off + (uint32_t)i, salt));
                goto out;
            }
        }
        off += (uint32_t)n;
        if (off >= next_report || off == total) {
            DBG("BENCH cid=%llu UPLOAD RX progress=%u/%u calls=%u", cid, off, total, calls);
            next_report = off + 65536U;
        }
    }
out:
    free(buf);
    DBG("BENCH cid=%llu UPLOAD RX RETURN rc=%d bytes=%u/%u calls=%u", cid, rc, off, total, calls);
    return rc;
}

static int send_pattern(int fd, uint32_t total, uint32_t chunk, uint8_t salt,
                        unsigned long long cid)
{
    uint8_t *buf = malloc(chunk);
    uint32_t off = 0;
    uint32_t next_report = 0;
    unsigned chunks = 0;
    int rc = 0;

    if (!buf) {
        DBG("BENCH cid=%llu DOWNLOAD malloc failed chunk=%u", cid, chunk);
        return -ENOMEM;
    }
    DBG("BENCH cid=%llu DOWNLOAD TX ENTER total=%u chunk=%u", cid, total, chunk);
    while (off < total) {
        uint32_t n = chunk;
        if (n > total - off)
            n = total - off;
        for (uint32_t i = 0; i < n; i++)
            buf[i] = pattern(off + i, salt);
        rc = write_all_dbg(fd, buf, n, cid, "DOWNLOAD-DATA");
        if (rc < 0)
            break;
        off += n;
        chunks++;
        if (off >= next_report || off == total) {
            DBG("BENCH cid=%llu DOWNLOAD TX progress=%u/%u chunks=%u", cid, off, total, chunks);
            next_report = off + 65536U;
        }
    }
    free(buf);
    DBG("BENCH cid=%llu DOWNLOAD TX RETURN rc=%d bytes=%u/%u chunks=%u", cid, rc, off, total, chunks);
    return rc;
}

static int send_reply(int fd, uint32_t mode, uint32_t status, uint32_t br, uint32_t bs,
                      unsigned long long cid)
{
    struct reply r = {
        htonl(BENCH_MAGIC), htonl(mode), htonl(status), htonl(br), htonl(bs)
    };
    DBG("BENCH cid=%llu REPLY ENTER mode=%s(%u) status=%u br=%u bs=%u",
        cid, mode_name(mode), mode, status, br, bs);
    int rc = write_all_dbg(fd, &r, sizeof(r), cid, "REPLY");
    DBG("BENCH cid=%llu REPLY RETURN rc=%d", cid, rc);
    return rc;
}

static int handle(int fd, unsigned long long cid)
{
    struct request q;
    int rc;
    uint32_t magic, ver, mode, chunk, total;

    DBG("HANDLE cid=%llu ENTER fd=%d", cid, fd);
    rc = read_all_dbg(fd, &q, sizeof(q), cid, "REQUEST");
    if (rc < 0) {
        DBG("HANDLE cid=%llu REQUEST FAIL rc=%d", cid, rc);
        return rc;
    }

    magic = ntohl(q.magic);
    ver = ntohl(q.version);
    mode = ntohl(q.mode);
    chunk = ntohl(q.chunk_size);
    total = ntohl(q.total_bytes);
    DBG("HANDLE cid=%llu REQUEST magic=0x%08x ver=%u mode=%s(%u) chunk=%u total=%u",
        cid, magic, ver, mode_name(mode), mode, chunk, total);

    if (magic != BENCH_MAGIC || ver != BENCH_VERSION || chunk == 0 || total == 0) {
        DBG("HANDLE cid=%llu REQUEST INVALID", cid);
        return -EPROTO;
    }

    if (mode == MODE_UPLOAD) {
        rc = receive_pattern(fd, total, chunk, 0x17, cid);
        if (rc < 0)
            return rc;
        return send_reply(fd, mode, 0, total, 0, cid);
    }

    if (mode == MODE_DOWNLOAD) {
        struct reply ack;
        rc = send_pattern(fd, total, chunk, 0x53, cid);
        if (rc < 0)
            return rc;
        DBG("BENCH cid=%llu DOWNLOAD waiting client ack", cid);
        rc = read_all_dbg(fd, &ack, sizeof(ack), cid, "DOWNLOAD-ACK");
        if (rc < 0)
            return rc;
        DBG("BENCH cid=%llu DOWNLOAD client ack received", cid);
        return send_reply(fd, mode, 0, 0, total, cid);
    }

    if (mode == MODE_FULL) {
        uint8_t *rx = malloc(chunk), *tx = malloc(chunk);
        uint32_t ro = 0, so = 0;
        struct reply ack;

        if (!rx || !tx) {
            free(rx);
            free(tx);
            DBG("BENCH cid=%llu FULL malloc failed chunk=%u", cid, chunk);
            return -ENOMEM;
        }
        DBG("BENCH cid=%llu FULL ENTER total=%u chunk=%u", cid, total, chunk);
        rc = 0;
        while (ro < total || so < total) {
            if (so < total) {
                uint32_t n = chunk;
                if (n > total - so)
                    n = total - so;
                for (uint32_t i = 0; i < n; i++)
                    tx[i] = pattern(so + i, 0x53);
                rc = write_all_dbg(fd, tx, n, cid, "FULL-TX");
                if (rc < 0)
                    break;
                so += n;
            }
            if (ro < total) {
                uint32_t n = chunk;
                if (n > total - ro)
                    n = total - ro;
                ssize_t got = read(fd, rx, n);
                if (got <= 0) {
                    rc = got == 0 ? -ECONNRESET : -errno;
                    DBG("BENCH cid=%llu FULL RX FAIL got=%zd ro=%u/%u errno=%d(%s) rc=%d",
                        cid, got, ro, total, errno, strerror(errno), rc);
                    break;
                }
                for (ssize_t i = 0; i < got; i++) {
                    if (rx[i] != pattern(ro + (uint32_t)i, 0x17)) {
                        rc = -EBADMSG;
                        DBG("BENCH cid=%llu FULL PATTERN FAIL ro=%u index=%zd", cid, ro, i);
                        break;
                    }
                }
                if (rc < 0)
                    break;
                ro += (uint32_t)got;
            }
            if (((ro | so) & 0xffffU) == 0 || (ro == total && so == total))
                DBG("BENCH cid=%llu FULL progress rx=%u/%u tx=%u/%u", cid, ro, total, so, total);
        }
        free(rx);
        free(tx);
        DBG("BENCH cid=%llu FULL DATA RETURN rc=%d rx=%u/%u tx=%u/%u", cid, rc, ro, total, so, total);
        if (rc < 0)
            return rc;
        DBG("BENCH cid=%llu FULL waiting client ack", cid);
        rc = read_all_dbg(fd, &ack, sizeof(ack), cid, "FULL-ACK");
        if (rc < 0)
            return rc;
        return send_reply(fd, mode, 0, total, total, cid);
    }

    DBG("HANDLE cid=%llu UNKNOWN MODE=%u", cid, mode);
    return -EINVAL;
}

int main(int argc, char **argv)
{
    const char *bind_ip = argc > 1 ? argv[1] : "0.0.0.0";
    int port = argc > 2 ? atoi(argv[2]) : 19000;
    int one = 1;
    int s;
    struct sockaddr_in a = {0};

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    setvbuf(stderr, NULL, _IONBF, 0);

    DBG("MAIN START pid=%ld bind=%s:%d AF_STCP=%d proto=%d", (long)getpid(), bind_ip, port,
        AF_STCP, IPPROTO_STCP);
    s = socket(AF_STCP, SOCK_STREAM, IPPROTO_STCP);
    if (s < 0) {
        DBG("MAIN socket(AF_STCP) FAIL errno=%d(%s)", errno, strerror(errno));
        return 2;
    }
    DBG("MAIN LISTENER SOCKET fd=%d", s);

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        DBG("MAIN SO_REUSEADDR FAIL errno=%d(%s)", errno, strerror(errno));
        close(s);
        return 1;
    }

    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, bind_ip, &a.sin_addr) != 1) {
        DBG("MAIN BAD BIND IP %s", bind_ip);
        close(s);
        return 2;
    }
    if (bind(s, (struct sockaddr *)&a, sizeof(a)) < 0) {
        DBG("MAIN BIND FAIL fd=%d errno=%d(%s)", s, errno, strerror(errno));
        close(s);
        return 2;
    }
    DBG("MAIN BIND OK fd=%d", s);
    if (listen(s, 8) < 0) {
        DBG("MAIN LISTEN FAIL fd=%d errno=%d(%s)", s, errno, strerror(errno));
        close(s);
        return 2;
    }
    DBG("MAIN LISTEN OK fd=%d backlog=8", s);
    fprintf(stderr, "[server] STCP BEN2 debug listening %s:%d\n", bind_ip, port);

    while (!stop_flag) {
        int c;
        int rc;
        int close_rc;
        unsigned long long cid;

        DBG("ACCEPT ENTER listener_fd=%d next_cid=%llu", s, conn_seq + 1);
        errno = 0;
        c = accept(s, NULL, NULL);
        if (c < 0) {
            int accept_errno = errno;

            if (accept_errno == EINTR) {
                DBG("ACCEPT EINTR stop=%d", stop_flag ? 1 : 0);
                continue;
            }

            /*
             * AF_STCP may time out an idle accept window while leaving the
             * listener valid.  Robot has gaps between benchmark cases, so
             * these are transient conditions, not listener failures.
             */
            if (accept_errno == ETIMEDOUT ||
                accept_errno == EAGAIN ||
                accept_errno == EWOULDBLOCK) {
                DBG("ACCEPT TRANSIENT listener_fd=%d errno=%d(%s) retry=1",
                    s, accept_errno, strerror(accept_errno));
                continue;
            }

            DBG("ACCEPT FAIL listener_fd=%d errno=%d(%s)",
                s, accept_errno, strerror(accept_errno));
            break;
        }

        cid = ++conn_seq;
        DBG("ACCEPT RETURN cid=%llu client_fd=%d", cid, c);
        dump_endpoint(c, "LOCAL", cid);
        dump_endpoint(c, "PEER", cid);

        DBG("CONN cid=%llu HANDLE ENTER", cid);
        rc = handle(c, cid);
        DBG("CONN cid=%llu HANDLE RETURN rc=%d", cid, rc);
        fprintf(stderr, "[server] client cid=%llu rc=%d\n", cid, rc);

        DBG("CONN cid=%llu CLOSE ENTER fd=%d", cid, c);
        errno = 0;
        close_rc = close(c);
        DBG("CONN cid=%llu CLOSE RETURN fd=%d rc=%d errno=%d(%s)", cid, c, close_rc,
            close_rc < 0 ? errno : 0, close_rc < 0 ? strerror(errno) : "OK");
        DBG("CONN cid=%llu DONE next_accept=1", cid);
    }

    DBG("MAIN LISTENER CLOSE ENTER fd=%d", s);
    close(s);
    DBG("MAIN EXIT connections=%llu stop=%d", conn_seq, stop_flag ? 1 : 0);
    return 0;
}
