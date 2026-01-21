#pragma once
#include <stcp/debug.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <linux/rcupdate.h>

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>

enum stcp_handshake_status {
    STCP_HS_STATUS_NONE = 0,
    STCP_HS_STATUS_HS_PENDING,
    STCP_HS_STATUS_HS_IN_PROGRESS,
    STCP_HS_STATUS_HS_DONE,
    STCP_HS_STATUS_HS_FAILED,
};

/*
 * ============================================================
 * STCP FLAGS (bit flags, st->flags)
 * ============================================================
 */

/* Internal recursion bypass (send/recv from Rust) */
#define STCP_FLAG_INTERNAL_IO_BIT                    0

/* Handshake lifecycle */
#define STCP_FLAG_HS_START_FROM                      1
#define STCP_FLAG_HS_PENDING_BIT                     (0 + STCP_FLAG_HS_START_FROM)
#define STCP_FLAG_HS_QUEUED_BIT                      (1 + STCP_FLAG_HS_START_FROM)
#define STCP_FLAG_HS_COMPLETE_BIT                    (2 + STCP_FLAG_HS_START_FROM)
#define STCP_FLAG_HS_FAILED_BIT                      (3 + STCP_FLAG_HS_START_FROM)
#define STCP_FLAG_HS_EXIT_MODE_BIT                   (4 + STCP_FLAG_HS_START_FROM)
#define STCP_FLAG_HS_STARTED_BIT                     (5 + STCP_FLAG_HS_START_FROM)


/* Role */
#define STCP_FLAG_ROLE_START_FROM                   10
#define STCP_FLAG_HS_SERVER_BIT                     (0 + STCP_FLAG_ROLE_START_FROM)
#define STCP_FLAG_HS_CLIENT_BIT                     (1 + STCP_FLAG_ROLE_START_FROM)

/* Socket / fatal */
#define STCP_FLAG_SOCKET_START_FROM                 15
#define STCP_FLAG_SOCKET_FATAL_ERROR_BIT            (0 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_BOUND_BIT                  (1 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_DESTROY_QUEUED_BIT         (2 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_LISTENING_BIT              (3 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_DETACHED_BIT               (4 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_FREE_QUEUED_BIT            (5 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_FREED_BIT                  (6 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_DATA_READY_RESTORED_BIT    (7 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_RELEASE_ENTERED_BIT        (8 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_DESTROY_DONE_BIT           (9 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_TCP_FORCE_DONE_BIT         (10 + STCP_FLAG_SOCKET_START_FROM)
#define STCP_FLAG_SOCKET_IN_DATA_READY_BIT          (11 + STCP_FLAG_SOCKET_START_FROM)

/* Future bits start from 32+ */
#define STCP_FLAG_SESSION_START_FROM                32
#define STCP_FLAG_SESSION_CREATION_ERROR_BIT        (0 + STCP_FLAG_SESSION_START_FROM)



#define REASON_DESTROY_FROM_INTERRUPT               100
#define REASON_DESTROY_FROM_OK_CONTEXT              101
#define REASON_TCP_NOT_ESTABLISHED                  102
#define REASON_RECV_BEFORE_SESSION                  103
#define REASON_PUMP_ACCEPT_DONE                     104
#define REASON_PUMP_CONNECT_DONE                    105
#define REASON_TCP_ESTABLISHED                      106

inline int stcp_state_is_listening_socket(struct sock *sk);
inline struct stcp_sock *stcp_get_st_ref_from_sk(struct sock *sk);
inline void stcp_state_put_st(struct stcp_sock *st);

inline int stcp_state_try_acquire_free(struct stcp_sock *st);
inline int stcp_wait_for_flag_or_timeout(struct stcp_sock *st,
                                        unsigned long the_bit,
                                        unsigned int timeout_ms,
                                        bool nonblock);

int stcp_state_wait_for_handshake_or_timeout(struct stcp_sock *st,
                                       unsigned int bit_complete,
                                       unsigned int timeout_ms,
                                       bool nonblock);

// Handshake helppereitä                                              
inline void stcp_state_hanshake_reset(struct stcp_sock *st);
inline bool stcp_state_hanshake_is_complete(struct stcp_sock *st);
inline bool stcp_state_hanshake_is_failed(struct stcp_sock *st);
inline void stcp_state_hanshake_mark_complete(struct stcp_sock *st);
inline void stcp_state_hanshake_mark_failed(struct stcp_sock *st, int err);

inline bool stcp_state_is_nonblock_sock(struct sock *sk, int msg_flags);
inline int stcp_state_wait_tcp_established(struct sock *sk,
                                           unsigned int timeout_ms,
                                           bool nonblock);

inline int stcp_state_wait_connection_established(struct sock *sk,
                                                  unsigned int tcp_wait_timeout_ms,
                                                  unsigned int stcp_wait_timeout_ms,
                                                  int msg_flags);

// RCU makrot
#define RCU_ENTER()  rcu_read_lock()
#define RCU_EXIT()   rcu_read_unlock()

#define RCU_DEREF(p)       rcu_dereference(p)
#define RCU_ASSIGN(p, v)   rcu_assign_pointer((p), (v))
#define RCU_INIT(p, v)     RCU_INIT_POINTER((p), (v))

#define RCU_VALIDATE_OR_NULL(st_io)                                       \
    do {                                                                  \
        if ((st_io) && READ_ONCE((st_io)->magic) != STCP_MAGIC_ALIVE)     \
            (st_io) = NULL;                                               \
    } while (0)

#define RCU_ENTER_AND_GET_FROM_SK(st_out, sk_in)                          \
    do {                                                                  \
        RCU_ENTER();                                                      \
        (st_out) = (struct stcp_sock *)RCU_DEREF((sk_in)->sk_user_data);  \
    } while (0)

#define RCU_ENTER_GET_VALIDATE_FROM_SK(st_out, sk_in)                     \
    do {                                                                  \
        RCU_ENTER();                                                      \
        (st_out) = (struct stcp_sock *)RCU_DEREF((sk_in)->sk_user_data);  \
        RCU_VALIDATE_OR_NULL(st_out);                                     \
    } while (0)

#define RCU_EXIT_RETURN(x)        do { RCU_EXIT(); return (x); } while (0)
#define RCU_EXIT_RETURN_VOID()    do { RCU_EXIT(); return; } while (0)
#define RCU_EXIT_GOTO(label)      do { RCU_EXIT(); goto label; } while (0)

#define STCP_WITH_RCU_ST(st, sk)                                         \
    for (RCU_ENTER(),                                                    \
         (st) = (struct stcp_sock *)RCU_DEREF((sk)->sk_user_data),        \
         RCU_VALIDATE_OR_NULL(st);                                       \
         ;                                                               \
         RCU_EXIT(), (st) = NULL)                                        \
        if (1)

