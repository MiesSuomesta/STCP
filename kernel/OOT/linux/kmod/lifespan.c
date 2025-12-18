
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>

#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>

#include <stcp/debug.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>

#define GOT_LIFE_SPAN_CHECKS 4
#include <stcp/lifespan.h>
                
int stcp_end_of_life_for_sk(void *skvp, int err) {

    struct sock *sk = (struct sock *)skvp;

    if (!sk)
        return -EINVAL;

    struct stcp_sock *st = NULL;

    st = sk->sk_user_data;
    if (st) {
        stcp_struct_free_st(st);
        sk->sk_user_data = NULL;
    }
    return 0;
}

/*
 * Kuinka monta STCP-kontekstia / socketia on elossa.
 * Tämä ei sinällään estä moduulin unloadia, mutta antaa
 * debug-infon jos jokin jää roikkumaan.
 *
 */

#if GOT_LIFE_SPAN_CHECKS
static atomic_t stcp_sockets_alive = ATOMIC_INIT(0);
#endif

void stcp_exported_rust_sockets_alive_get(void)
{
#if GOT_LIFE_SPAN_CHECKS
#if GOT_LIFE_SPAN_CHECKS > 1
    SDBG("Get from:");
    // dump_stack();
#endif
    atomic_inc(&stcp_sockets_alive);
    SDBG("stcp: rust context get: Alive is now %u", 
        (u32)atomic_read(&stcp_sockets_alive));
#else
    SDBG("stcp: lifespan disabled"); 
#endif
}

void stcp_exported_rust_sockets_alive_put(void)
{
#if GOT_LIFE_SPAN_CHECKS
#if GOT_LIFE_SPAN_CHECKS > 1
    SDBG("Put from:");
    // dump_stack();
#endif

    unsigned int tmp = atomic_read(&stcp_sockets_alive);
    if (tmp > 0) {
        atomic_dec(&stcp_sockets_alive);
    } else {
        pr_err("stcp: rust context put: undeflow, not decrementing!");
    }
    SDBG("stcp: rust context put: Alive is now %u",
            (u32)atomic_read(&stcp_sockets_alive));
#else
    SDBG("stcp: lifespan disabled"); 
#endif
}

int stcp_exported_rust_ctx_alive_count(void)
{
#if GOT_LIFE_SPAN_CHECKS
    unsigned int ret = (u32)atomic_read(&stcp_sockets_alive);

    SDBG("stcp: Alive Rust contexts now: %u", ret);
    return ret;
#else
    SDBG("stcp: lifespan disabled"); 
    return 0;
#endif
}

