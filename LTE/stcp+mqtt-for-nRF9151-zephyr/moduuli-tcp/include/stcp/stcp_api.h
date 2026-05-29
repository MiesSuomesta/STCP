#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>

#include <stcp/stcp_generated.h>
#include <stcp/stcp_mutex.h>
#include <stcp/stcp_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stcp_api __aligned(16);

#define VOID_TO_API(vp)    ((struct stcp_api*)(vp))

#define API_GET_LOCK_FROM_API_PTR(api)   stcp_api_context_get_lock(VOID_TO_API(api))

#define API_CONTEXT_LOCK_TRACE_FROM \
        stcp_debug_find_symbol_name(        \
            __builtin_return_address(0)     \
        )

#define STCP_USE_OWN_MUTEX_IMPL         0
#define STCP_CONTEXT_LOCK_VERBOSE       0

#if STCP_USE_OWN_MUTEX_IMPL
#define API_MUTES_LOCK_TYPE         struct stcp_mutex
#define API_MUTEX_LOCK_FN_NAME      stcp_mutex_lock
#define API_MUTEX_UNLOCK_FN_NAME    stcp_mutex_unlock
#define API_MUTEX_LOCK_PARAMETER    -1
#else 
#define API_MUTES_LOCK_TYPE         struct k_mutex
#define API_MUTEX_LOCK_FN_NAME      k_mutex_lock
#define API_MUTEX_UNLOCK_FN_NAME    k_mutex_unlock
#define API_MUTEX_LOCK_PARAMETER    K_FOREVER
#endif

#define VOID_TO_API_LOCK_PTR(lockPtr)  ((API_MUTES_LOCK_TYPE *)(lockPtr))
#define API_GET_LOCK_PTR(api)   VOID_TO_API_LOCK_PTR(API_GET_LOCK_FROM_API_PTR(api))


#if STCP_CONTEXT_LOCK_VERBOSE

#define API_MUTEX_LOG(state, api)                   \
    do {                                            \
        const char* _call_from =                    \
            API_CONTEXT_LOCK_TRACE_FROM;            \
        LDBG("[API(%p)] %s called from %s ",        \
            api,                                    \
            state,                                  \
            _call_from);                              \
    } while (0)

#else // NOT STCP_CONTEXT_LOCK_VERBOSE

#define API_MUTEX_LOG(state, api)
            
#endif // LOCK VERBOSE


#if STCP_USE_OWN_MUTEX_IMPL
#define API_MUTEX_LOCK_FN_NAME      stcp_mutex_lock
#define API_MUTEX_UNLOCK_FN_NAME    stcp_mutex_unlock
#define API_MUTEX_LOCK_PARAMETER    -1
#else 
#define API_MUTEX_LOCK_FN_NAME      k_mutex_lock
#define API_MUTEX_UNLOCK_FN_NAME    k_mutex_unlock
#define API_MUTEX_LOCK_PARAMETER    K_FOREVER
#endif


#define API_CONTEXT_LOCK_IMPL(api)      \
    API_MUTEX_LOCK_FN_NAME(             \
            API_GET_LOCK_PTR(api),      \
            API_MUTEX_LOCK_PARAMETER    \
        )

#define API_CONTEXT_UNLOCK_IMPL(api)               \
        API_MUTEX_UNLOCK_FN_NAME(                  \
                API_GET_LOCK_PTR(api)              \
            )

#if STCP_USE_OWN_MUTEX_IMPL

#define API_CONTEXT_LOCK(api) \
    do {                                    \
        API_MUTEX_LOCK_LOG("Locking", api); \
            API_CONTEXT_LOCK_IMPL(api);     \
        API_MUTEX_LOCK_LOG("Locked", api);  \
    } while(0)

#define API_CONTEXT_UNLOCK(api) \
    do {                                      \
        API_MUTEX_LOCK_LOG("Unlocking", api); \
            API_CONTEXT_UNLOCK_IMPL(api);     \
        API_MUTEX_LOCK_LOG("Unlocked", api);  \
    } while(0)

#else // not STCP_USE_OWN_MUTEX_IMPL

#define API_CONTEXT_LOCK(api) \
        API_CONTEXT_LOCK_IMPL(api)

#define API_CONTEXT_UNLOCK(api) \
        API_CONTEXT_UNLOCK_IMPL(api)


#endif // Own impl

__attribute__((used))
__attribute__((noinline))
int stcp_api_get_errno();

int stcp_api_check_magic(struct stcp_api *api);

// Main entrypoint...
int stcp_library_init(void);

void* stcp_api_context_get_lock(struct stcp_api *api);

int stcp_api_get_handshake_status(struct stcp_api *api);

int stcp_api_wait_until_reached_stcp_init_ready(int timeout_sec);
int stcp_api_resolve(const char *host, const char *port, struct zsock_addrinfo **result);
    
// Semaforien odottelut per instanssi
int stcp_api_wait_until_reached_ip_network_up(struct stcp_api *api, int timeout);
int stcp_api_wait_until_reached_lte_ready    (struct stcp_api *api, int timeout);
int stcp_api_wait_until_reached_pdn_ready    (struct stcp_api *api, int timeout);

/* Lifecycle */
int     stcp_api_init(struct stcp_api **api);
int     stcp_api_init_with_fd(struct stcp_api **api, int fd);
int     stcp_api_replace_fd(struct stcp_api *api, int fd);
int     stcp_api_close(struct stcp_api *api);
int     stcp_api_get_connect_in_progress(struct stcp_api *api);
int     stcp_api_acquire(struct stcp_api *api);
void    stcp_api_release(struct stcp_api *api);
int     stcp_api_is_open_for_fd_io(struct stcp_api *api);

/* Server */
int     stcp_api_bind(struct stcp_api *api,
                      const struct zsock_addrinfo *addr,
                      socklen_t addrlen);

int     stcp_api_listen(struct stcp_api *api,
                        int backlog);

int     stcp_api_accept(struct stcp_api *api,
                        struct stcp_api **new_api,
                        struct zsock_addrinfo *peer,
                        socklen_t *peer_len);

/* Client */
int     stcp_api_connect(struct stcp_api *api,
                         const struct zsock_addrinfo *addr,
                         socklen_t addrlen);

int stcp_api_is_open_for_io(struct stcp_api *api);

int stcp_api_wait_until_stcp_handshake_is_done(struct stcp_api *api, int timeout_ms);
int stcp_api_connection_set_as_connected(struct stcp_api *api);
int stcp_api_connection_state_init(struct stcp_api *api);

int stcp_api_connection_reset(struct stcp_api *api);


/* IO */
int stcp_api_set_io_timeout(struct stcp_api *api, int timeout_ms);

ssize_t stcp_api_send(struct stcp_api *api,
                       const void *buf,
                       size_t len,
                       int flags);

ssize_t stcp_api_recv(struct stcp_api *api,
                      void *buf,
                      size_t len,
                      int flags);

ssize_t stcp_api_sendmsg(struct stcp_api *api,
                         const struct msghdr *msg);

/* Nonblocking */
int     stcp_api_set_nonblocking(struct stcp_api *api,
                                 bool enable);

/* Poll */
int stcp_api_poll(struct stcp_api *api,
                  int events,
                  int timeout_ms,
                  int *revents);

/* Optional FD access */
int     stcp_api_get_fd(struct stcp_api *api);

int     stcp_api_get_dns_info(struct stcp_api *api, 
                              struct zsock_addrinfo **res);
/* State */
void stcp_api_try_to_wakeup_radio(void);
int stcp_api_wait_for_radio_connected(int seconds);
void stcp_api_request_reset();
// API on kunnossa?
int stcp_api_is_valid(struct stcp_api *api);
int stcp_api_pointer_valid(struct stcp_api *api);

// Onko API avoina liikenteelle?
int stcp_api_is_usable(struct stcp_api *api);

// Api on hengissä ?
int stcp_api_is_alive(struct stcp_api *api);

int stcp_api_wait_until_connected_to_peer(struct stcp_api *api, int timeout_ms);
int stcp_api_wait_until_connected_to_peer_no_lock(struct stcp_api *api, int timeout_ms);

int stcp_api_wait_until_modem_lte_ready(struct stcp_api *api, int timeout_ms);
int stcp_api_wait_until_modem_pdn_ready(struct stcp_api *api, int timeout_ms);
int stcp_api_wait_until_modem_ip_network_up(struct stcp_api *api, int timeout_ms);
int stcp_api_get_modem_state(int *lte, int *pdn, int *ip, int *radio, int *connection_ok);

int stcp_rust_api_transport_get_fd(void *pks_void);

#ifdef __cplusplus
}
#endif
