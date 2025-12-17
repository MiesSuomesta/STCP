// stcp_misc.c

#include <linux/slab.h>
#include <linux/types.h>
#include <stcp/rust_alloc.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <net/tcp.h>

#include <stcp/debug.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/stcp_proto_ops.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_misc.h>
#include <stcp/settings.h>


#define ALLOC_ZERO(mVar) \
    mVar = stcp_rust_kernel_alloc(sizeof(*mVar))

void stcp_misc_ecdh_key_free(void *pFreePtr)
{
    if (pFreePtr) {
        kfree(pFreePtr);
    }
}

void *stcp_misc_ecdh_public_key_new(void)
{
    struct stcp_ecdh_public_key *pk;

    ALLOC_ZERO(pk);

    return pk;
}

void *stcp_misc_ecdh_shared_key_new(void)
{
    struct stcp_ecdh_shared_key *sk;

    ALLOC_ZERO(sk);

    return sk;
}

void *stcp_misc_ecdh_private_key_new(void)
{
    struct stcp_ecdh_private_key *pk;

    ALLOC_ZERO(pk);

    return pk;
}

int stcp_misc_ecdh_private_key_size(void) { return sizeof(struct stcp_ecdh_private_key); }
int stcp_misc_ecdh_public_key_size(void)  { return sizeof(struct stcp_ecdh_public_key); }
int stcp_misc_ecdh_shared_key_size(void)  { return sizeof(struct stcp_ecdh_shared_key); }

void stcp_misc_force_tcp_protocol(struct sock *sk, const char *where)
{
#if ENABLE_PROTO_FORCING
    if (!sk) return;
    if (sk->sk_protocol != IPPROTO_TCP) {
        int old = sk->sk_protocol;
        sk->sk_protocol = IPPROTO_TCP;
        SDBG("%s: forced sk_protocol %d -> %d", where, old, sk->sk_protocol);
    }
#else
    SDBG("Bypassed forcing protocol.");
#endif
}

inline int stcp_swap_proto(struct sock *sk, int newp)
{
#if ENABLE_PROTO_SWAPPING 
    int old = READ_ONCE(sk->sk_protocol);
    WRITE_ONCE(sk->sk_protocol, newp);
    return old;
#else
    SDBG("Bypassed swapping protocol.");
#endif
    return sk->sk_protocol;
}

int is_handshake_in_progress(struct sock *sk) {
    struct stcp_sock* st = stcp_struct_get_st_from_sk(sk);
    if (!st) {
        return -EINVAL;
    }

    int isPending  = (st->status & STCP_STATUS_HANDSHAKE_PENDING)  >  0;
    int isStarted  = (st->status & STCP_STATUS_HANDSHAKE_STARTED)  >  0;
    int isComplete = (st->status & STCP_STATUS_HANDSHAKE_COMPLETE) >  0;
    int isFailed   = (st->status & STCP_STATUS_HANDSHAKE_FAILED)   >  0;

    SDBG("HS/in progress: Pending: %d, Started: %d, Completed: %d, Failed: %d",
        isPending, isStarted, isComplete, isFailed);

    if (isStarted) {
        if (!isComplete) {
            // In progress
            return 1;
        }
    }
    return 0;
}
