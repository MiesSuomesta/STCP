// kmod/stcp_proto.c
// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/types.h>   // bbool
#include <linux/net.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <net/protocol.h>
#include <linux/kernel.h>  // container_of
#include <net/sock.h>      // struct sock
#include <linux/net.h>     // struct socket
#include <net/inet_connection_sock.h>

// STCP kmoduuli include 
#include <stcp/kmod.h>
#include <stcp/sock_glue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lauri Jakku / Paxsudos IT <lauri.jakku@paxsudos.fi>");
MODULE_DESCRIPTION("STCP: SecureTCP, drop in replacement for TCP socket");

int stcp_safe_mode = 1;
int stcp_debug_mode = 1;

module_param_named(safe_mode, stcp_safe_mode, int, 0644);
MODULE_PARM_DESC(safe_mode,
   "If non-zero, refuse everything except stcp_release to debug accept lockups.");

module_param_named(debug_mode, stcp_debug_mode, int, 0644);
MODULE_PARM_DESC(debug_mode,
   "If non-zero, enables the debugging of STCP protocol.");

int stcp_proto_register(void);
void stcp_proto_unregister(void);


/* ---- proto_ops: vain osoittimet glue-kerroksen funktioihin ---- */
static const struct proto_ops stcp_stream_ops = {
    .family     = AF_INET,
    .owner      = THIS_MODULE,
    .release    = stcp_release,
    .bind       = stcp_bind,
    .connect    = stcp_connect,
    .socketpair = sock_no_socketpair,
    .getname    = stcp_getname,
    .poll       = stcp_poll,
    .ioctl      = stcp_ioctl,
    .listen     = stcp_listen,
    .shutdown   = stcp_shutdown,
    .setsockopt = stcp_setsockopt,
    .getsockopt = stcp_getsockopt,
    .sendmsg    = stcp_sendmsg,
    .recvmsg    = stcp_recvmsg,
/* oikea accept-osoitin kerneliversion mukaan */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
    .accept     = stcp_accept_glue,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0)
    .accept     = stcp_accept_glue,
#else
    .accept     = stcp_accept_glue,
#endif

};



static int stcp_sk_init(struct sock *sk)
{
	struct stcp_sock *st = stcp_from_sk(sk);

	pr_debug("stcp: SK init at A");

    st->parent = NULL;
    st->inner  = NULL;
    st->rust   = NULL;
    st->magic  = 0x53544350u; /* 'STCP' */


	pr_debug("stcp: SK init at B");
    spin_lock_init(&st->lock);
	spin_lock_init(&st->inner_lock);
    INIT_LIST_HEAD(&st->accept_q);
    init_waitqueue_head(&st->waitq);

	BUILD_BUG_ON(sizeof(struct stcp_sock) < sizeof(struct sock));

	pr_debug("stcp: SK init OK");
	return 0;
}

static void stcp_sk_destroy(struct sock *sk)
{
	struct stcp_sock *st = stcp_from_sk(sk);
    STCP_CHECK_ST_VOID(st);
    STCP_CHECK_INNER_VOID(st);

	if (!sk)
		return;

	st = (struct stcp_sock *)sk;
 
	stcp_inner_destroy(st);
}

static struct proto stcp_prot = {
	.name       = "stcp",
	.owner      = THIS_MODULE,
	.obj_size   = sizeof(struct stcp_sock),
	.useroffset = 0,
	.usersize   = 0,
	.init       = stcp_sk_init,
	.destroy    = stcp_sk_destroy,
	.close      = stcp_close,
};

/* ---- Protosw: liitä AF_INET:iin IPPROTO_STCP ---- */
static struct inet_protosw stcp_inet_protosw = {
	.type       = SOCK_STREAM,
	.protocol   = IPPROTO_STCP,
	.prot       = &stcp_prot,
	.ops        = &stcp_stream_ops,
	.flags      = INET_PROTOSW_ICSK,
};

/* ---- julkiset rekisteröinnit modpostia varten ---- */

#ifndef STCP_BUILD_DATE
#define STCP_BUILD_DATE "unknown"
#endif
#ifndef STCP_GIT_SHA
#define STCP_GIT_SHA "unknown"
#endif

static void stcp_kernel_banner(void)
{
    pr_debug(".----<[STCP by Paxsudos IT]>------------------------------------------------------------>\n");
    pr_debug("|  ✅ STCP Initialised, Protocol number %d\n", IPPROTO_STCP);
    pr_debug("|  🕓 Build at %s (%s)\n", STCP_BUILD_DATE, STCP_GIT_SHA);
    pr_debug("'----------------------------------------------------------------------'\n");
}


int stcp_proto_register(void)
{

	pr_info("stcp: proto_register start (obj_size=%u, name=%s)\n",
		stcp_prot.obj_size, stcp_prot.name);
	pr_debug("stcp: registering proto: %px", &stcp_prot);
	proto_register(&stcp_prot, 1 /* alloc_slab */);
	pr_debug("stcp: registering inet_protosw: %px (prot=%px ops=%px type=%d proto=%d)\n",
		 &stcp_inet_protosw,
		 stcp_inet_protosw.prot,
		 stcp_inet_protosw.ops,
		 stcp_inet_protosw.type,
		 stcp_inet_protosw.protocol);

	if (!stcp_inet_protosw.prot || !stcp_inet_protosw.ops) {
		pr_err("stcp: BUG: inet_protosw.prot/ops is NULL\n");
		proto_unregister(&stcp_prot);
		return -EINVAL;
	}
	pr_debug("stcp: attempting to register AF_INET...");
	inet_register_protosw(&stcp_inet_protosw);
	pr_info("stcp: AF_INET protosw registered (IPPROTO_STCP=%d)\n", IPPROTO_STCP);
	stcp_kernel_banner();
	return 0;
}
EXPORT_SYMBOL_GPL(stcp_proto_register);

void stcp_proto_unregister(void)
{
	pr_debug("stcp: unregisttering inet sw");
	inet_unregister_protosw(&stcp_inet_protosw);
	pr_debug("stcp: unregisttering proto sw");
	proto_unregister(&stcp_prot);
	pr_info("stcp: unregistered\n");
}
EXPORT_SYMBOL_GPL(stcp_proto_unregister);

static int __init stcp_c_init(void)
{
    pr_info("stcp_c: module_init -> registering IPPROTO_STCP=253\n");
    return stcp_proto_register();
}

static void __exit stcp_c_exit(void)
{
    pr_info("stcp_c: module_exit -> unregister\n");
    stcp_proto_unregister();
}

module_init(stcp_c_init);
module_exit(stcp_c_exit);
