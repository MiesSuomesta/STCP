// kmod/stcp_proto.c
// SPDX-License-Identifier: GPL-2.0

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

#include <stcp/lifespan.h>   // stcp_ctx_get/put/live_count jos sulla on tämä
#include <stcp/stcp_misc.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_proto_ops.h>
#include <stcp/settings.h>
#include <stcp/stcp_protocol.h>
#include <stcp/tcp_callbacks.h>

#ifndef STCP_BUILD_DATE
#define STCP_BUILD_DATE "nodate"
#endif

#ifndef STCP_GIT_SHA
#define STCP_GIT_SHA "nogit"
#endif

#ifndef STCP_VERSION
#define STCP_VERSION "0.0.1"
#endif 


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lauri Jakku / Paxsudos IT <lauri.jakku@paxsudos.fi>");
MODULE_DESCRIPTION("STCP: SecureTCP, drop in replacement for TCP socket");

MODULE_VERSION(STCP_VERSION "+" STCP_GIT_SHA "." STCP_BUILD_DATE);
MODULE_ALIAS_NETPROTO(IPPROTO_STCP);
MODULE_SOFTDEP("pre: tcp ecdh");
MODULE_INFO(company, "Paxsudos IT");
MODULE_INFO(product, "STCP Module for linux");
MODULE_INFO(support, "info@paxsudos.fi");

#define SHOW_BUILD_CONFIG 0

static void stcp_kernel_banner(void)
{
    pr_emerg(".----<[STCP by Paxsudos IT]>------------------------------------------------------------>\n");
    pr_emerg("|  ✅ STCP Initialised (Version %s), Protocol number %d\n", STCP_VERSION, IPPROTO_STCP);
    pr_emerg("|  🕓 Build at %s (%s)\n", STCP_BUILD_DATE, STCP_GIT_SHA);

#if SHOW_BUILD_CONFIG
#  if ENABLE_RATELIMIT_PRINTK
    pr_emerg("|  ✅ STCP RateLimit ON\n");
#  else
    pr_emerg("|  ✅ STCP RateLimit OFF\n");
#  endif
#endif

#if SHOW_BUILD_CONFIG
    pr_emerg("|     Config: ");
    pr_emerg("|        USE_OWN_SEND_MSG                : %s", onoff(USE_OWN_SEND_MSG));
    pr_emerg("|        USE_OWN_RECV_MSG                : %s", onoff(USE_OWN_RECV_MSG));
    pr_emerg("|        USE_OWN_DESTROY                 : %s", onoff(USE_OWN_DESTROY));
    pr_emerg("|        FORCE_TCP_PROTO                 : %s", onoff(FORCE_TCP_PROTO));
    pr_emerg("|        USE_OWN_PROT_OPTS               : %s", onoff(USE_OWN_PROT_OPTS));
    pr_emerg("|        USE_OWN_SOCK_OPTS               : %s", onoff(USE_OWN_SOCK_OPTS));
    pr_emerg("|");
    pr_emerg("|     Callback config: ");
    pr_emerg("|        USE_OWN_BIND                    : %s", onoff(USE_OWN_BIND));
    pr_emerg("|        USE_OWN_LISTEN                  : %s", onoff(USE_OWN_LISTEN));
    pr_emerg("|        USE_OWN_ACCEPT                  : %s", onoff(USE_OWN_ACCEPT));
    pr_emerg("|        USE_OWN_CONNECT                 : %s", onoff(USE_OWN_CONNECT));
    pr_emerg("|        USE_OWN_RELEASE                 : %s", onoff(USE_OWN_RELEASE));
    pr_emerg("|");
    pr_emerg("|     Timeout config: ");
    pr_emerg("|        Wait for TCP Established        : %d ms", STCP_WAIT_FOR_TCP_ESTABLISHED_MSEC);
    pr_emerg("|        Wait for HS Complete            : %d ms", STCP_WAIT_FOR_HANDSHAKE_TO_COMPLETE_MSEC);
    pr_emerg("|");
    pr_emerg("|     Misc config: ");
    pr_emerg("|        MAX Pumps                       : %d", STCP_HANDSHAKE_STATUS_MAX_PUMPS);
    pr_emerg("|        WANR ON Magic failure           : %s", onoff(WARN_ON_MAGIC_FAILURE));
    pr_emerg("|        Wait for Connection Establish   : %s", onoff(STCP_WAIT_FOR_CONNECTION_ESTABLISHED_AT_IO));
    pr_emerg("|        Socket IO: Bypass everything    : %s", onoff(STCP_SOCKET_BYPASS_ALL_IO));

    
#endif

    pr_emerg("'----------------------------------------------------------------------'\n");
}

static atomic_t RUST_INIT_DONE = ATOMIC_INIT(0);


inline int is_rust_init_done(void) {
    int ret = atomic_read(&RUST_INIT_DONE);
    SDBG("INIT: Is RUST init done: %d", ret);
    return ret;
}

inline void set_rust_init_done(int v) {
    SDBG("INIT: Set RUST init done: %d", v);
    atomic_set(&RUST_INIT_DONE, v);
}

int           (*orginal_tcp_sendmsg)(struct sock *sk, struct msghdr *msg, size_t len);
int           (*orginal_tcp_recvmsg)(struct sock *sk, struct msghdr *msg, size_t len, int flags, int *recv_len);
int           (*orginal_tcp_connect)(struct sock *sk, struct sockaddr_unsized *uaddr, int addr_len);
struct sock * (*orginal_tcp_accept) (struct sock *sk, struct proto_accept_arg *arg);
int           (*orginal_tcp_bind)   (struct sock *sk, struct sockaddr *uaddr, int addr_len);
void          (*orginal_tcp_destroy) (struct sock *sk);

#ifndef STCP_LOG
#define STCP_LOG(fmt, ...) \
    SDBG("stcp[proto]: " fmt "\n", ##__VA_ARGS__)
#endif


/* ----------- inet_protosw entry STCP:lle ------------- */
/*
 * Käytetään suoraan tcp_prot:ia – eli STCP on tässä vaiheessa
 * “TCP wrapper” joka käyttää samaa protoa, mutta eri proto_opsia.
 */
extern struct proto_ops stcp_stream_ops;
#if USE_OWN_PROT_OPTS
extern struct proto stcp_prot;
#endif 

struct inet_protosw stcp_inet_protosw = {
    .type     = SOCK_STREAM,
    .protocol = IPPROTO_STCP,     // 253

#if USE_OWN_PROT_OPTS
    .prot     = &stcp_prot,  
#else 
    .prot     = &tcp_prot,   
#endif

#if USE_OWN_SOCK_OPTS
    .ops      = &stcp_stream_ops, // socket-ops: bind/listen/connect/accept/release
#else
    .ops      = &inet_stream_ops,
#endif
    .flags    = INET_PROTOSW_ICSK,
};

/* ----------- Protosw rekisteröinti ------------- */
extern void stcp_rust_test_log(void);


static int stcp_proto_register(void)
{
    /* 
       Tehdään dynaaminen alustus: Koska
       kääntäjä ei annan käyttää tcp_prot.* 
       juttuja initissä.
    */
    // Defaultti: TCP Proto operaatiot.

    // tämä on vaarallinen?

    /* Rekisteröidään uusi protosw AF_INETiin (IPPROTO_STCP) */
    STCP_LOG("Doing stcp setup for socket operations...");
    stcp_socket_ops_setup(&stcp_stream_ops);

#if USE_OWN_SOCK_OPTS
    STCP_LOG("INIT: STCP socket ops: bind=%px listen=%px connect=%px accept=%px release=%px owner=%px\n",
        stcp_stream_ops.bind, stcp_stream_ops.listen, stcp_stream_ops.connect,
        stcp_stream_ops.accept, stcp_stream_ops.release, stcp_stream_ops.owner);
#endif


    int reg = stcp_proto_setup();
    if (reg< 0) {
        STCP_LOG("Protocol setup failed! (%d)", reg);
        return reg;
    }
    
    STCP_LOG("Registering stcp...");
    inet_register_protosw(&stcp_inet_protosw);


    STCP_LOG("INIT: STCP Registered: protosw prot=%px ops=%px proto=%d\n",
        stcp_inet_protosw.prot, stcp_inet_protosw.ops, stcp_inet_protosw.protocol);

#if USE_OWN_PROT_OPTS
    STCP_LOG("INIT: stcp_prot=%px name=%s owner=%px sendmsg=%px recvmsg=%px\n",
        &stcp_prot, stcp_prot.name, stcp_prot.owner,
        stcp_prot.sendmsg, stcp_prot.recvmsg);
#endif
        
    STCP_LOG("stcp_proto_register: registered AF_INET, proto=%d (IPPROTO_STCP)",
             IPPROTO_STCP);
    return 0;
}

static void stcp_proto_unregister(void)
{
    STCP_LOG("stcp_proto_unregister: unregistering inet_protosw");
    inet_unregister_protosw(&stcp_inet_protosw);
}

/* ----------- module_init / module_exit ------------- */

static int __init stcp_init(void)
{
    int ret;

    SDBG("stcp_c: module_init\n");
    
    ret = stcp_proto_register();
    if (ret) {
        pr_err("stcp: stcp_rust_call_init failed: %d\n", ret);
        stcp_proto_unregister();
        return ret;
    }

    SDBG("stcp: calling module_rust_enter\n");
    stcp_module_rust_enter();
    set_rust_init_done(1);

    stcp_kernel_banner();
    return 0;
}

static void __exit stcp_exit(void)
{
    u32 live;

    SDBG("stcp_c: module_exit -> unregister protosw\n");
    stcp_proto_unregister();   // estää uudet socketit

    live = stcp_exported_rust_ctx_alive_count();
    if (live > 0) {
        pr_err("stcp: %u sockets still live at module exit! Refuse cleanup.\n", live);
        WARN_ON(1);
        return; // ei estä unloadia, mutta vähentää tuhon mahdollisuutta
    }

    stcp_module_rust_exit();
    set_rust_init_done(0);
}

module_init(stcp_init);
module_exit(stcp_exit);

