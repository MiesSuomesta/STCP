#pragma once
#include <net/sock.h>
#include <linux/printk.h>
#include <linux/socket.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/printk.h>
#include <linux/socket.h>  
#include <linux/kernel.h>  // container_of
#include <net/sock.h>      // struct sock
#include <linux/net.h>     // struct socket
#include <net/inet_connection_sock.h>

// Omat headerit
#include "kmod.h"
#include "lifecycle.h"


#ifdef USE_SAFEGUARD
#define SAFE_GUARD_ME   \
    if (stcp_safe_mode && __builtin_strcmp(__func__, "stcp_release")) {                               \
        pr_info("stcp: SafeGuard @ %s\n", __func__);    \
        return -EOPNOTSUPP;                             \
    } 

#define SAFE_GUARD_ME_VOID                                                 \
    if (stcp_safe_mode && __builtin_strcmp(__func__, "stcp_release")) {    \
        pr_info("stcp: SafeGuard @ %s\n", __func__);                       \
        return ;                                                           \
    } 

#else

#define SAFE_GUARD_ME /* empty */
#define SAFE_GUARD_ME_VOID /* empty */

#endif

/* inner ST check */
#define VERBOSE_CHECK_INNER_ST(st) \
    do { \
        if (!(st)) { pr_info("stcp[%s]: st=NULL\n", __func__); return -EINVAL; } \
        if (!(st)->inner) { pr_info("stcp[%s]: inner=NULL\n", __func__); return -EINVAL; } \
        if (!(st)->inner->sock) { pr_info("stcp[%s]: sock=NULL\n", __func__); return -EINVAL; } \
    } while (0)


#define CHECK_ST(st)                                                     \
    do {                                                                 \
        SAFE_GUARD_ME                                                    \
        pr_info("stcp: Checking pointers @ %s ...\n", __func__);         \
        if (!(st)) {                                                     \
            pr_warn("stcp: error @ %s B (st NULL)\n", __func__);         \
            return -EINVAL;                                              \
        }                                                                \
        if (!(st)->inner) {                                              \
            pr_warn("stcp: error @ %s B (st NULL)\n",  __func__);        \
            return -EINVAL;                                              \
        }                                                                \
        if (!(st)->inner->sock) {                                        \
            pr_warn("stcp: error @ %s C (st NULL)\n",  __func__);        \
            return -EINVAL;                                              \
        }                                                                \
    } while (0)

#define CHECK_ST_VOID(st)                                                \
    do {                                                                 \
        SAFE_GUARD_ME_VOID                                               \
        pr_info("stcp: Checking pointers @ %s ...\n", __func__);         \
        if (!(st)) {                                                     \
            pr_warn("stcp: error @ %s B (st NULL)\n", __func__);         \
            return;                                                      \
        }                                                                \
        if (!(st)->inner) {                                              \
            pr_warn("stcp: error @ %s B (st NULL)\n",  __func__);        \
            return;                                                      \
        }                                                                \
        if (!(st)->inner->sock) {                                        \
            pr_warn("stcp: error @ %s C (st NULL)\n",  __func__);        \
            return;                                                      \
        }                                                                \
    } while (0)

#define STCP_CHECK_ST(st)                            \
    do {                                             \
        if (unlikely(!(st))) {                       \
            pr_warn("stcp: null st\n");              \
            return -EINVAL;                          \
        }                                            \
    } while (0)

#define STCP_CHECK_INNER(st)                         \
    do {                                             \
        if (unlikely(!(st)->inner)) {                \
            pr_warn("stcp: no inner yet\n");         \
            return -ENOTCONN;                        \
        }                                            \
    } while (0)

#define STCP_CHECK_ST_VOID(st)                       \
    do {                                             \
        if (unlikely(!(st))) {                       \
            pr_warn("stcp: null st\n");              \
            return;                                  \
        }                                            \
    } while (0)

#define STCP_CHECK_INNER_VOID(st)                    \
    do {                                             \
        if (unlikely(!(st)->inner)) {                \
            pr_warn("stcp: no inner yet\n");         \
            return ;                                 \
        }                                            \
    } while (0)



/*
    Apureita soketti käsittelyyn jotka varmsitavat että kaikki on OK joka paikassa.
*/

inline struct stcp_sock *stcp_get_st(struct socket *sock, const char *fn);
inline int stcp_guard_inner_required(struct stcp_sock *st, const char *fn);
inline int stcp_ensure_inner(struct stcp_sock *st, const char *fn);
    

/*
    Hakee stcp_sock structin sock pointterista,
    errori: returnaa invalidin jos ei löydy
*/
#define STCP_GET_ST_OR_RET(__sock) \
    do  {                                                   \
        struct stcp_sock *__st = stcp_get_st((__sock), __func__); \
        if (!__st) return -EINVAL; \
        pr_debug("stcp: %s: st=%px inner=%px sock=%px\n",   \
            __func__, __st, __st ? __st->inner : NULL,            \
            (__st && __st->inner) ? __st->inner->sock : NULL);    \
    } while (0)


/*
    Hakee stcp_sock structin sock pointterista,
    
*/
#define STCP_REQUIRE_INNER_OR_RET(__sock) do { \
    struct stcp_sock *__st = stcp_get_st((__sock), __func__); \
    int __g = stcp_guard_inner_required(__st, __func__); \
    if (__g) return __g; \
} while (0)

#define STCP_ENSURE_INNER_OR_RET(__sock) do { \
    struct stcp_sock *__st = stcp_get_st((__sock), __func__); \
    int __e = stcp_ensure_inner(__st, __func__); \
    if (__e) return __e; \
} while (0)

extern inline struct stcp_state *stcp_state_from_socket(const struct socket *sock);
extern inline void stcp_state_attach_state(struct sock *sk, struct stcp_state *st);
extern inline void stcp_state_detach_state(struct sock *sk);
extern inline void stcp_state_free_inner(struct stcp_state *st);


// STCP_SOCK

extern inline struct stcp_sock *stcp_from_sk(struct sock *sk);
extern inline const struct stcp_sock *stcp_from_sk_const(const struct sock *sk);
extern inline struct stcp_sock *stcp_from_socket(const struct socket *sock);
extern inline struct stcp_connection_phase *stcp_connection_phase_from_socket(const struct socket *sock);
extern inline void stcp_flag_set(struct stcp_connection_phase *st, unsigned int idx);
extern inline void stcp_flag_clear(struct stcp_connection_phase *st, unsigned int idx);
extern inline bool stcp_flag_is_set(const struct stcp_connection_phase *st, unsigned int idx);

// Helppereitä soketin käsittelyyn...
extern inline struct stcp_sock *stcp_get_st(struct socket *sock, const char *fn);
extern inline int stcp_guard_inner_required(struct stcp_sock *st, const char *fn);
extern inline int stcp_ensure_inner(struct stcp_sock *st, const char *fn);
extern inline int stcp_ensure_phase(struct stcp_sock *st, const char *fn);

extern inline struct stcp_sock_ctx *stcp_get_context_from_socket(const struct socket *sock);
extern inline int stcp_need_context_from_socket(const struct socket *sock, struct stcp_sock_ctx **out);

inline int stcp_ensure_all_ok(struct socket *sock,
                                     struct stcp_sock **out_st,
                                     const char *fn);
