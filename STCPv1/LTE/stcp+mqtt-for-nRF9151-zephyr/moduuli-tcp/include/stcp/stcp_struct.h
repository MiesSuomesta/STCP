#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stddef.h>
#include <stdint.h>
#include <stcp/stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/fsm.h>
#include <stcp/lifespan.h>
#include <stcp/low_level_operations.h>

#define STCP_STRUCT_ALIGN __aligned(16)


struct kernel_socket {
	int fd;
	void *kctx;
} STCP_STRUCT_ALIGN;

enum {
    CTX_STATE_INIT = 0,
    CTX_STATE_CONNECTING,
    CTX_STATE_CONNECTED,
    CTX_STATE_CLOSING,
};

enum stcp_ctx_state {
    STCP_STATE_INIT = 0,
    STCP_STATE_CONNECTING,
    STCP_STATE_ESTABLISHED,
    STCP_STATE_CLOSING
};

// 2KB 
#define STCP_RECV_STREAM_BUF_SIZE (256)

// 2KB 
#define STCP_RECV_FRAME_BUF_SIZE (256)

#define STCP_MAX_HOSTNAME_LEN   60
#define STCP_MAX_PORT_LEN       8

#define STCP_CTX_MAGIC_ALIVE            0x53544350
#define STCP_CTX_MAGIC_ALIVE_FOOTER     0x50433553
#define STCP_CTX_MAGIC_POISON           0xDEADBEEF

struct stcp_recv_stream {
    uint8_t *buffer;
    size_t pos;
    size_t len;
} STCP_STRUCT_ALIGN;

#define VOID_TO_API(vp)    ((struct stcp_api*)(vp))
#define VOID_TO_CTX(vp)    ((struct stcp_ctx*)(vp))
#define GET_ATOMIC_VALUE_FROM(val) (int)atomic_get(&(val))

#define CONTEXT_GET_API_ACCESS_FLAG(ctx)       atomic_get(&(VOID_TO_CTX(ctx)->allow_api_access))
#define CONTEXT_SET_API_ACCESS_FLAG(ctx, val)  atomic_set(&(VOID_TO_CTX(ctx)->allow_api_access), val)

#define GET_API_FROM_CTX(ctx)    ((ctx) ? VOID_TO_API( VOID_TO_CTX(ctx)->api ) : NULL)

// Koodi EI saa itsessään tehdä returnia kesken! => menee refcountti
// epäbalanssiin!

#define STCP_API_ACCESS_OK(api)   (stcp_check_if_access_is_ok(api)  > 0)
#define STCP_API_ACCESS_NOK(api)  (stcp_check_if_access_is_ok(api) <= 0)

#define API_DO_IF_ACCESS_IS_NOK(api, name, CODE) \
        do {                                           \
        STCP_REF_COUNT_GET(api->ctx, name, return -ESTALE);  \
            if ( STCP_API_ACCESS_NOK(api) ) {          \
                CODE;                                  \
            }                                          \
        STCP_REF_COUNT_PUT(api->ctx, name);            \
        } while(0)

#define API_DO_IF_ACCESS_IS_OK(api, name, CODE) \
        do {                                           \
        STCP_REF_COUNT_GET(api->ctx, name, LDBG("Stale @ %s", name); return -ESTALE; );  \
            if ( STCP_API_ACCESS_OK(api) ) {           \
                CODE;                                  \
            } else { LDBG("API: Access denied."); }    \
        STCP_REF_COUNT_PUT(api->ctx, name);            \
        } while(0)

#define CONTEXT_REF(ctx, CODE) \
    do { \
        int __get_ret = stcp_context_lifespan_extend(ctx); \
        if (__get_ret < 1) { CODE; } \
    } while (0)

#define CONTEXT_UNREF(ctx) \
    stcp_context_lifespan_shorten(ctx)

#define STRUCT_GET_MEMBER(api, what, def) \
    ((api) ? (VOID_TO_API(api)->what) : def )

#define GET_API_MEMBER_FROM_CTX(ctx, what, def) \
    STRUCT_GET_MEMBER(GET_API_FROM_CTX(ctx), what, def)

#define GET_API_STATE_FROM_CTX(ctx) \
    GET_API_MEMBER_FROM_CTX(ctx, fsm->state, -1)

#define CONTEXT_LOCK(ctx) \
    do {                                                     \
        LDBG("[Context(%p / state: %d)] Locking..",          \
            ctx, GET_API_STATE_FROM_CTX(ctx));               \
        if (ctx != NULL) {                                   \
            int to = k_mutex_lock(&(VOID_TO_CTX(ctx)->lock), \
                           K_MSEC(500));                     \
            if (to != 0) {                                   \
                LDBG("[Context(%p / state: %d)] TIMEOUT!",   \
                    ctx, GET_API_STATE_FROM_CTX(ctx));       \
            }                                                \
        }                                                    \
        LDBG("[Context(%p / state: %d)] Locked..",           \
                    ctx, GET_API_STATE_FROM_CTX(ctx));       \
    } while(0)

#define CONTEXT_UNLOCK(ctx) \
    do {                                                                   \
        LDBG("[Context(%p / state: %d)] Unlocking..",                      \
                    ctx, GET_API_STATE_FROM_CTX(ctx));                     \
        if (ctx != NULL) {                                                 \
            k_mutex_unlock(&(VOID_TO_CTX(ctx)->lock));                     \
        }                                                                  \
        LDBG("[Context(%p / state: %d)] Unlocked..",                       \
                    ctx, GET_API_STATE_FROM_CTX(ctx));                     \
    } while(0)

#define _REAL_API_LOCK(api) \
    do {                                            \
        LDBG("[API(%p)] Locking..", api);           \
        if (api != NULL) {                          \
            CONTEXT_LOCK(VOID_TO_API(api)->ctx);    \
        }                                           \
        LDBG("[API(%p)] Locked..", api);            \
    } while(0)


#define _REAL_API_UNLOCK(api) \
    do {                                            \
        LDBG("[API(%p)] Unlocking..", api);         \
        if (api != NULL) {                          \
            CONTEXT_UNLOCK(VOID_TO_API(api)->ctx);  \
        }                                           \
        LDBG("[API(%p)] Unlocked..", api);          \
    } while(0)


#define API_LOCK_ACCESS_UNCHECKED(api) \
    _REAL_API_LOCK(api)

#define API_UNLOCK_ACCESS_UNCHECKED(api) \
    _REAL_API_UNLOCK(api)

#define API_LOCK(api, name) \
    API_DO_IF_ACCESS_IS_OK(api, name, API_LOCK_ACCESS_UNCHECKED(api))

#define API_UNLOCK(api, name) \
    API_DO_IF_ACCESS_IS_OK(api, name, API_UNLOCK_ACCESS_UNCHECKED(api))

struct stcp_ctx {
    uint32_t magic; // Conteksti magikki

	struct k_mutex lock;
	struct k_work_delayable cleanup_work;
	struct k_work_delayable heavy_cleanup_work;
    int worker_init_done;

    enum stcp_ctx_state state;
    atomic_t refcnt;
    atomic_t owns;
    atomic_t closing;
    atomic_t cleanup_running;
    atomic_t cleanup_is_rescheduled;
    atomic_t cleanup_work_owns_ref;
    atomic_t connection_closed;
    atomic_t destroyed;
    atomic_t allow_api_access;
    atomic_t connected;
	int doing_replace;
	int handshake_done;
    int poll_timeouts;
    size_t rx_frame_len;
    size_t rx_payload_pos;
    size_t rx_payload_len;

    struct k_poll_signal handshake_signal;

    void *session;   // Rust STCP session pointer
	void *api;       // Api instance this conntext belongs to. 

	struct kernel_socket ks;
    struct zsock_addrinfo *dns_resolved;
    // Bufferit loppuun ... 
	char ctx_hostname[STCP_MAX_HOSTNAME_LEN];
	int  ctx_port;

    // RX bufferi per konteksti
    char rx_frame[STCP_RECV_FRAME_BUF_SIZE];
    // RX STCP frame puskuri
    char rx_stream_buffer[STCP_RECV_STREAM_BUF_SIZE];
    struct stcp_recv_stream rx_stream;

    // Koonti puskuri
    uint8_t rx_payload[STCP_RECV_FRAME_BUF_SIZE];

    uint32_t magic_footer; // Conteksti magikki

} STCP_STRUCT_ALIGN;

// Kaikkien struktien pitää olla align 16