#include "stcp_ping.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/net/icmp.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/byteorder.h>

#define STCP_PING_DEFAULT_COUNT 4U
#define STCP_PING_DEFAULT_TIMEOUT_MS 3000U
#define STCP_PING_INTERVAL_MS 1000U
#define STCP_PING_PAYLOAD_SIZE 32U

#define STCP_PING_NET_READY_TIMEOUT_MS 15000U
#define STCP_PING_NET_READY_POLL_MS 100U

static int wait_for_network_ready(const struct shell *shell)
{
    struct net_if *iface = net_if_get_default();
    int64_t deadline = k_uptime_get() + STCP_PING_NET_READY_TIMEOUT_MS;
    bool announced = false;

    if (iface == NULL) {
        shell_error(shell, "No default network interface");
        return -ENODEV;
    }

    while (k_uptime_get() < deadline) {
        if (net_if_is_up(iface) && net_if_is_carrier_ok(iface)) {
            if (announced) {
                shell_print(shell, "Network interface ready");
            }
            /* Allow static IPv4 and neighbor processing to settle. */
            k_sleep(K_MSEC(250));
            return 0;
        }

        if (!announced) {
            shell_print(shell, "Waiting for Ethernet interface and link...");
            announced = true;
        }
        k_sleep(K_MSEC(STCP_PING_NET_READY_POLL_MS));
    }

    shell_error(shell, "Network not ready: admin=%s carrier=%s",
                net_if_is_up(iface) ? "up" : "down",
                net_if_is_carrier_ok(iface) ? "up" : "down");
    return -ENETDOWN;
}

struct ping_wait {
    struct k_sem reply_sem;
    int64_t sent_at_ms;
    int64_t received_at_ms;
};

static int ping_reply_handler(struct net_icmp_ctx *ctx,
                              struct net_pkt *pkt,
                              struct net_icmp_ip_hdr *ip_hdr,
                              struct net_icmp_hdr *icmp_hdr,
                              void *user_data)
{
    struct ping_wait *wait = user_data;

    ARG_UNUSED(ctx);
    ARG_UNUSED(pkt);
    ARG_UNUSED(ip_hdr);
    ARG_UNUSED(icmp_hdr);

    if (wait != NULL) {
        wait->received_at_ms = k_uptime_get();
        k_sem_give(&wait->reply_sem);
    }

    return 0;
}

static int resolve_ipv4(const char *host, struct sockaddr_in *dst,
                        char *resolved, size_t resolved_size)
{
    struct zsock_addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    struct zsock_addrinfo *result = NULL;
    int rc;

    memset(dst, 0, sizeof(*dst));
    dst->sin_family = AF_INET;

    rc = zsock_inet_pton(AF_INET, host, &dst->sin_addr);
    if (rc == 1) {
        zsock_inet_ntop(AF_INET, &dst->sin_addr, resolved, resolved_size);
        return 0;
    }

    rc = zsock_getaddrinfo(host, NULL, &hints, &result);
    if (rc != 0 || result == NULL) {
        if (result != NULL) {
            zsock_freeaddrinfo(result);
        }
        return -EHOSTUNREACH;
    }

    memcpy(dst, result->ai_addr, sizeof(*dst));
    dst->sin_port = 0;
    zsock_inet_ntop(AF_INET, &dst->sin_addr, resolved, resolved_size);
    zsock_freeaddrinfo(result);
    return 0;
}

int stcp_ping_run(const struct shell *shell, size_t argc, char **argv)
{
    struct sockaddr_in dst;
    struct net_icmp_ctx ctx;
    struct ping_wait wait;
    struct net_icmp_ping_params params;
    uint8_t payload[STCP_PING_PAYLOAD_SIZE];
    char resolved[NET_IPV4_ADDR_LEN];
    uint32_t count = STCP_PING_DEFAULT_COUNT;
    uint32_t timeout_ms = STCP_PING_DEFAULT_TIMEOUT_MS;
    uint32_t sent = 0U;
    uint32_t received = 0U;
    int64_t rtt_min = INT64_MAX;
    int64_t rtt_max = 0;
    int64_t rtt_sum = 0;
    uint16_t identifier;
    int rc;

    if (argc < 2U) {
        shell_error(shell, "Usage: stcp ping <ip|name> [count] [timeout_ms]");
        return -EINVAL;
    }

    if (argc >= 3U) {
        char *end = NULL;
        unsigned long value = strtoul(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || value < 1UL || value > 100UL) {
            shell_error(shell, "Invalid count '%s' (1..100)", argv[2]);
            return -EINVAL;
        }
        count = (uint32_t)value;
    }

    if (argc >= 4U) {
        char *end = NULL;
        unsigned long value = strtoul(argv[3], &end, 10);
        if (end == argv[3] || *end != '\0' || value < 100UL || value > 60000UL) {
            shell_error(shell, "Invalid timeout '%s' (100..60000 ms)", argv[3]);
            return -EINVAL;
        }
        timeout_ms = (uint32_t)value;
    }

    rc = wait_for_network_ready(shell);
    if (rc < 0) {
        return rc;
    }

    rc = resolve_ipv4(argv[1], &dst, resolved, sizeof(resolved));
    if (rc < 0) {
        shell_error(shell, "Cannot resolve %s: %d", argv[1], rc);
        return rc;
    }

    memset(payload, 0xA5, sizeof(payload));
    k_sem_init(&wait.reply_sem, 0, 1);
    identifier = (uint16_t)sys_rand32_get();

    rc = net_icmp_init_ctx(&ctx, AF_INET, NET_ICMPV4_ECHO_REPLY, 0,
                           ping_reply_handler);
    if (rc < 0) {
        shell_error(shell, "ICMP context initialization failed: %d", rc);
        return rc;
    }

    shell_print(shell, "PING %s (%s): %u data bytes, count=%u, timeout=%u ms",
                argv[1], resolved, STCP_PING_PAYLOAD_SIZE, count, timeout_ms);

    for (uint32_t seq = 1U; seq <= count; seq++) {
        memset(&params, 0, sizeof(params));
        params.identifier = identifier;
        params.sequence = (uint16_t)seq;
        params.data = payload;
        params.data_size = sizeof(payload);
        params.priority = -1;

        while (k_sem_take(&wait.reply_sem, K_NO_WAIT) == 0) {
        }

        wait.sent_at_ms = k_uptime_get();
        wait.received_at_ms = 0;
        ctx.user_data = &wait;

        rc = net_icmp_send_echo_request(&ctx, NULL,
                                        (struct net_sockaddr *)&dst,
                                        &params, &wait);
        sent++;
        if (rc < 0) {
            shell_error(shell, "seq=%u send failed: %d", seq, rc);
        } else if (k_sem_take(&wait.reply_sem, K_MSEC(timeout_ms)) == 0) {
            int64_t rtt = wait.received_at_ms - wait.sent_at_ms;
            if (rtt < 0) {
                rtt = 0;
            }
            received++;
            rtt_sum += rtt;
            if (rtt < rtt_min) rtt_min = rtt;
            if (rtt > rtt_max) rtt_max = rtt;
            shell_print(shell, "%u bytes from %s: seq=%u time=%lld ms",
                        STCP_PING_PAYLOAD_SIZE, resolved, seq, rtt);
        } else {
            shell_warn(shell, "Request timeout: seq=%u", seq);
        }

        if (seq < count) {
            k_sleep(K_MSEC(STCP_PING_INTERVAL_MS));
        }
    }

    net_icmp_cleanup_ctx(&ctx);

    shell_print(shell, "--- %s ping statistics ---", resolved);
    shell_print(shell, "sent=%u received=%u lost=%u loss=%u%%",
                sent, received, sent - received,
                sent == 0U ? 0U : ((sent - received) * 100U) / sent);

    if (received > 0U) {
        shell_print(shell, "rtt min/avg/max = %lld/%lld/%lld ms",
                    rtt_min, rtt_sum / received, rtt_max);
        return 0;
    }

    return -ETIMEDOUT;
}
