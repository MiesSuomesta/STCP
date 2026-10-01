#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include "p2p_noise_probe.h"

#ifndef AF_STCP
#define AF_STCP 45
#endif
#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

extern int stcp_p2p_noise_dialer_new(const uint8_t identity_seed[32], void **out_ctx);
extern void stcp_p2p_noise_dialer_free(void *ctx);
extern int stcp_p2p_noise_dialer_message1(void *ctx, uint8_t *out, size_t cap);
extern int stcp_p2p_noise_dialer_message2(void *ctx, const uint8_t *msg2,
                                          size_t msg2_len, uint8_t *out3,
                                          size_t cap);
extern int stcp_p2p_noise_dialer_complete(void *ctx);

static const uint8_t identity_seed[32] = {
    0x53,0x54,0x43,0x50,0x76,0x32,0x2d,0x50,
    0x32,0x50,0x2d,0x4e,0x4f,0x49,0x53,0x45,
    0x2d,0x42,0x45,0x4e,0x43,0x48,0x2d,0x4b,
    0x45,0x59,0x2d,0x30,0x30,0x30,0x30,0x31
};

static int send_all(int fd, const uint8_t *buf, size_t len)
{
    size_t off = 0;
    while (off < len) {
        ssize_t n = zsock_send(fd, buf + off, len - off, 0);
        if (n > 0) { off += (size_t)n; continue; }
        if (n < 0 && errno == EINTR) continue;
        return n == 0 ? -ECONNRESET : -errno;
    }
    return 0;
}

static int recv_all(int fd, uint8_t *buf, size_t len)
{
    size_t off = 0;
    while (off < len) {
        ssize_t n = zsock_recv(fd, buf + off, len - off, 0);
        if (n > 0) { off += (size_t)n; continue; }
        if (n < 0 && errno == EINTR) continue;
        return n == 0 ? -ECONNRESET : -errno;
    }
    return 0;
}

static int encode_uvarint(size_t value, uint8_t *out, size_t cap)
{
    size_t i = 0;
    do {
        if (i >= cap) return -ENOSPC;
        uint8_t b = (uint8_t)(value & 0x7fU);
        value >>= 7;
        if (value) b |= 0x80U;
        out[i++] = b;
    } while (value);
    return (int)i;
}

static int send_ms_frame(int fd, const uint8_t *payload, size_t len)
{
    uint8_t prefix[10];
    int pn = encode_uvarint(len, prefix, sizeof(prefix));
    if (pn < 0) return pn;
    int rc = send_all(fd, prefix, (size_t)pn);
    return rc < 0 ? rc : send_all(fd, payload, len);
}

static int recv_uvarint(int fd, size_t *value)
{
    size_t v = 0, shift = 0;
    for (size_t i = 0; i < 10; ++i) {
        uint8_t b;
        int rc = recv_all(fd, &b, 1);
        if (rc < 0) return rc;
        v |= ((size_t)(b & 0x7fU)) << shift;
        if (!(b & 0x80U)) { *value = v; return 0; }
        shift += 7;
    }
    return -EPROTO;
}

static int recv_ms_frame(int fd, uint8_t *out, size_t cap, size_t *out_len)
{
    size_t n;
    int rc = recv_uvarint(fd, &n);
    if (rc < 0) return rc;
    if (n > cap) return -EMSGSIZE;
    rc = recv_all(fd, out, n);
    if (rc < 0) return rc;
    *out_len = n;
    return 0;
}

static int send_noise_frame(int fd, const uint8_t *msg, size_t len)
{
    if (len > 65535U) return -EMSGSIZE;
    uint8_t hdr[2] = {(uint8_t)(len >> 8), (uint8_t)len};
    int rc = send_all(fd, hdr, sizeof(hdr));
    return rc < 0 ? rc : send_all(fd, msg, len);
}

static int recv_noise_frame(int fd, uint8_t *out, size_t cap, size_t *out_len)
{
    uint8_t hdr[2];
    int rc = recv_all(fd, hdr, sizeof(hdr));
    if (rc < 0) return rc;
    size_t n = ((size_t)hdr[0] << 8) | hdr[1];
    if (n > cap) return -EMSGSIZE;
    rc = recv_all(fd, out, n);
    if (rc < 0) return rc;
    *out_len = n;
    return 0;
}

static int connect_stcp(const struct p2p_bench_config *cfg)
{
    struct sockaddr_in peer = {0};
    char *end = NULL;
    unsigned long port = strtoul(cfg->port, &end, 10);
    if (end == cfg->port || *end != '\0' || port == 0 || port > 65535)
        return -EINVAL;

    peer.sin_family = AF_INET;
    peer.sin_port = htons((uint16_t)port);
    if (zsock_inet_pton(AF_INET, cfg->host, &peer.sin_addr) != 1)
        return -EINVAL;

    int fd = zsock_socket(AF_STCP, SOCK_STREAM, IPPROTO_STCP);
    if (fd < 0) return -errno;

    struct timeval tv = {
        .tv_sec = cfg->timeout_ms / 1000U,
        .tv_usec = (cfg->timeout_ms % 1000U) * 1000U,
    };
    (void)zsock_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    (void)zsock_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (zsock_connect(fd, (struct sockaddr *)&peer, sizeof(peer)) < 0) {
        int rc = -errno;
        zsock_close(fd);
        return rc;
    }
    return fd;
}

int p2p_noise_probe_run(const struct shell *sh, const struct p2p_bench_config *cfg)
{
    static const uint8_t ms_id[] = "/multistream/1.0.0\n";
    static const uint8_t noise_id[] = "/noise\n";
    uint8_t frame[512];
    uint8_t noise2[4096];
    uint8_t noise3[4096];
    size_t n = 0;
    void *noise = NULL;
    int fd = -1, rc;

    shell_print(sh, "P2P Noise probe: STCP %s:%s", cfg->host, cfg->port);

    fd = connect_stcp(cfg);
    if (fd < 0) { shell_error(sh, "STCP connect failed rc=%d", fd); return fd; }

    rc = send_ms_frame(fd, ms_id, sizeof(ms_id) - 1U);
    if (rc < 0) goto out;
    rc = recv_ms_frame(fd, frame, sizeof(frame), &n);
    if (rc < 0) goto out;
    if (n != sizeof(ms_id)-1U || memcmp(frame, ms_id, n) != 0) { rc=-EPROTO; goto out; }
    shell_print(sh, "  multistream : PASS");

    rc = send_ms_frame(fd, noise_id, sizeof(noise_id) - 1U);
    if (rc < 0) goto out;
    rc = recv_ms_frame(fd, frame, sizeof(frame), &n);
    if (rc < 0) goto out;
    if (n != sizeof(noise_id)-1U || memcmp(frame, noise_id, n) != 0) { rc=-EPROTO; goto out; }
    shell_print(sh, "  /noise      : PASS");

    rc = stcp_p2p_noise_dialer_new(identity_seed, &noise);
    if (rc < 0) goto out;
    rc = stcp_p2p_noise_dialer_message1(noise, frame, sizeof(frame));
    if (rc < 0) goto out;
    n = (size_t)rc;
    rc = send_noise_frame(fd, frame, n);
    if (rc < 0) goto out;
    shell_print(sh, "  Noise XX m1 : TX %u bytes", (unsigned)n);

    rc = recv_noise_frame(fd, noise2, sizeof(noise2), &n);
    if (rc < 0) goto out;
    shell_print(sh, "  Noise XX m2 : RX %u bytes", (unsigned)n);

    rc = stcp_p2p_noise_dialer_message2(noise, noise2, n, noise3, sizeof(noise3));
    if (rc < 0) goto out;
    n = (size_t)rc;
    rc = send_noise_frame(fd, noise3, n);
    if (rc < 0) goto out;
    shell_print(sh, "  Noise XX m3 : TX %u bytes", (unsigned)n);

    if (!stcp_p2p_noise_dialer_complete(noise)) { rc=-EPROTO; goto out; }
    shell_print(sh, "Noise XX + libp2p identity: PASS");
    shell_print(sh, "Golden peer authenticated; Yamux negotiation is the next phase.");
    rc = 0;

out:
    if (noise) stcp_p2p_noise_dialer_free(noise);
    if (fd >= 0) zsock_close(fd);
    if (rc < 0) shell_error(sh, "P2P Noise probe FAIL rc=%d errno=%d", rc, errno);
    return rc;
}
