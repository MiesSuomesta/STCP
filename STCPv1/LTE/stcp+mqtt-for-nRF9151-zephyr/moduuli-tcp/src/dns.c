#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include <stcp/debug.h>
#include <stcp/utils.h>
#include <stcp/dns.h>
#include <stcp/low_level_pointer.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>

/* ============================================================
 * CONFIG
 * ============================================================ */

#define STCP_DNS_MAX_HOSTNAME 256

/* ============================================================
 * CACHE STRUCT
 * ============================================================ */

struct stcp_dns_cache {
    char hostname[STCP_DNS_MAX_HOSTNAME];

    struct zsock_addrinfo *ai;     // original addrinfo (Zephyr alloc)
    struct sockaddr_storage *addr_copy;    // deep copy of ai_addr

    int valid;

    struct k_mutex lock;
};

/* ============================================================
 * GLOBAL CACHE
 * ============================================================ */

static struct stcp_dns_cache g_dns_cache;

/* ============================================================
 * INIT
 * ============================================================ */

void stcp_dns_init(void)
{
    memset(&g_dns_cache, 0, sizeof(g_dns_cache));
    k_mutex_init(&g_dns_cache.lock);
}

/* ============================================================
 * INTERNAL FREE (LOCKED)
 * ============================================================ */

static void stcp_dns_cache_free_locked(void)
{
    if (g_dns_cache.ai) {
        LDBG("[DNS] Freeing addrinfo %p", g_dns_cache.ai);
        zsock_freeaddrinfo(g_dns_cache.ai);
        g_dns_cache.ai = NULL;
    }

    if (g_dns_cache.addr_copy) {
        LDBG("[DNS] Freeing addr copy %p", g_dns_cache.addr_copy);
        STCP_MEMORY_DEALLOC(g_dns_cache.addr_copy);
        g_dns_cache.addr_copy = NULL;
    }

    g_dns_cache.valid = 0;
}

/* ============================================================
 * PUBLIC FREE
 * ============================================================ */

int stcp_dns_free_cache(void)
{
    k_mutex_lock(&g_dns_cache.lock, K_FOREVER);

    stcp_dns_cache_free_locked();

    k_mutex_unlock(&g_dns_cache.lock);
    return 0;
}

int stcp_dns_free(struct zsock_addrinfo *ptr)
{
    zsock_freeaddrinfo(ptr);
    return 0;
}


/* ============================================================
 * RESOLVE (CACHED)
 * ============================================================ */

int stcp_dns_resolve(const char *host, int port, struct zsock_addrinfo **out)
{
    if (!host || !out) {
        return -EINVAL;
    }

    k_mutex_lock(&g_dns_cache.lock, K_FOREVER);

    /* ========================================================
     * CACHE HIT
     * ======================================================== */

    if (g_dns_cache.valid &&
        strcmp(g_dns_cache.hostname, host) == 0) {

        LDBG("[DNS] Cache HIT for %s", host);

        *out = g_dns_cache.ai;

        k_mutex_unlock(&g_dns_cache.lock);
        return 0;
    }

    /* ========================================================
     * CACHE MISS → FREE OLD
     * ======================================================== */

    LDBG("[DNS] Cache MISS for %s", host);

    stcp_dns_cache_free_locked();

    /* ========================================================
     * DO RESOLVE
     * ======================================================== */

    char port_buf[10];
    snprintk(port_buf, sizeof(port_buf), "%d", port);

    struct zsock_addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct zsock_addrinfo *res = NULL;

    LDBG("[DNS] Resolving %s:%d ...", host, port);

    int rc = zsock_getaddrinfo(host, port_buf, &hints, &res);

    if (rc < 0 || !res) {
        LERR("[DNS] getaddrinfo failed rc=%d", rc);
        k_mutex_unlock(&g_dns_cache.lock);
        return -EIO;
    }

    /* ========================================================
     * DEBUG PRINT
     * ======================================================== */

    struct sockaddr_in *sin = (struct sockaddr_in *)res->ai_addr;

    LDBG("[DNS] Resolved %s:%d => %d.%d.%d.%d:%d",
        host,
        port,
        sin->sin_addr.s4_addr[0],
        sin->sin_addr.s4_addr[1],
        sin->sin_addr.s4_addr[2],
        sin->sin_addr.s4_addr[3],
        ntohs(sin->sin_port)
    );

    /* ========================================================
     * DEEP COPY ADDR (CRITICAL!)
     * ======================================================== */

    struct sockaddr_storage *addr_copy = STCP_MEMORY_ALLOC(sizeof(struct sockaddr_storage));

    if (!addr_copy) {
        LERR("[DNS] OOM while copying sockaddr");
        zsock_freeaddrinfo(res);
        k_mutex_unlock(&g_dns_cache.lock);
        return -ENOMEM;
    }

    if (res->ai_addrlen > sizeof(struct sockaddr_storage)) {
        LERR("[DNS] addr too large: %d", res->ai_addrlen);
        STCP_MEMORY_DEALLOC(addr_copy);
        zsock_freeaddrinfo(res);
        k_mutex_unlock(&g_dns_cache.lock);
        return -EINVAL;
    }
    // OK
    memset(addr_copy, 0, sizeof(struct sockaddr_storage));
    memcpy(addr_copy, res->ai_addr, res->ai_addrlen);
    /* ========================================================
     * SAVE CACHE
     * ======================================================== */

    g_dns_cache.ai = res;
    g_dns_cache.addr_copy = addr_copy;
    g_dns_cache.valid = 1;

    memset(g_dns_cache.hostname, 0, sizeof(g_dns_cache.hostname));
    snprintk(g_dns_cache.hostname,
             sizeof(g_dns_cache.hostname) - 1,
             "%s", host);

    *out = res;

    LDBG("[DNS] Cached result for %s", host);

    k_mutex_unlock(&g_dns_cache.lock);
    return 0;
}

/* ============================================================
 * OPTIONAL: GET SAFE ADDR (COPY)
 * ============================================================ */

const struct sockaddr_storage *stcp_dns_get_cached_addr(void)
{
    return g_dns_cache.addr_copy;
}

int stcp_dns_resolve_all(void)
{

#if CONFIG_STCP_TESTING
	char *pHostName = CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT;
	int pHostPort = CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT;
#else
	char *pHostName = "80.186.162.127"; // CONFIG_STCP_CONNECT_TO_HOST;
	int pHostPort = 7777; // CONFIG_STCP_CONNECT_TO_PORT;
#endif

    LINFBIG("[DNS] Fetching %s:%d first time, filling the cache.", pHostName, pHostPort);
    int rc = stcp_dns_resolve(pHostName, pHostPort, NULL);

}

int stcp_dns_resolve_stcp_context(struct stcp_ctx *ctx, struct zsock_addrinfo **out) {
    LINFBIG("[DNS] Fetching %s:%d ...", ctx->ctx_hostname, ctx->ctx_port);
    int rc = stcp_dns_resolve(ctx->ctx_hostname, ctx->ctx_port, out);
}

