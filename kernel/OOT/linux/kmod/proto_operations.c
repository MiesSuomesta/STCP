
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
#include <stcp/handshake_worker.h>
#include <stcp/stcp_tcp_low_level_operations.h>


/* ----------------------------------------- */
/*  SENDMSG (STCP encrypted send)            */
/* ----------------------------------------- */

int stcp_rust_glue_proto_op_sendmsg(struct sock *sk,
                                    struct msghdr *msg,
                                    size_t len)
{
    ssize_t ret;
    struct stcp_sock *st;
    int completed = 0;


    SDBG("HOOK sendmsg ENTER sk=%px proto=%u user_data=%px state=%d",
        sk, sk->sk_protocol, sk->sk_user_data, sk->sk_state);

    st = stcp_struct_get_st_from_sk(sk);
    if (st) {
        completed = (st->status & STCP_STATUS_HANDSHAKE_COMPLETE) > 0;
        SDBG("SendMSG: STCP handshake complete?: %d", completed);
    }

    SDBG("SendMSG[%px//%px]: At st check ..", sk, st);
    if (!st || !st->session) {
        /* älä tapa yhteyttä EINVAL:illa, vaan fallback TCP:lle */
        return orginal_tcp_sendmsg(sk, msg, len);
    }

    SDBG("SendMSG[%px//%px]: At handshake check, completed = %d ..", sk, st, completed);
    if (!completed) {
        SDBG("SendMSG[%px//%px]: HS Not done..", sk, st);
        return -EAGAIN; /* tai -ENOTCONN */
    }

    /* ✅ tärkein rivi: jos transportia ei ole, käytä tätä sockia */
    struct sock *tx_sk = st->sk ? st->sk : sk;
    SDBG("SendMSG[%px//%px]: TX Socket: %px", sk, st, tx_sk);

    /* debug varmistus */
    SDBG("SENDMSG: st=%px session=%px sk=%px tx_sk=%px state=%d",
         st, st->session, sk, tx_sk, tx_sk->sk_state);

    /* kopioi payload */
    char *kbuf = kzalloc(len, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    ret = copy_from_iter(kbuf, len, &msg->msg_iter);
    if (ret < 0) {
        kfree(kbuf);
        return ret;
    }

    /* vie Rustiin käyttäen tx_sk:ta */
    ret = rust_exported_session_sendmsg(st->session, (void *)tx_sk, kbuf, len);
    SDBG("SendMSG[%px//%px]: handled via rust, ret: %d", sk, st, (int)ret);

    kfree(kbuf);
    return ret;
}

/* ----------------------------------------- */
/*  RECVMSG (STCP encrypted recv)            */
/* ----------------------------------------- */

int stcp_rust_glue_proto_op_recvmsg(
    struct sock *sk,
    struct msghdr *msg,
    size_t len,
    int flags,
    int *recv_len)
{
    ssize_t ret;

    struct stcp_sock *st = stcp_struct_get_st_from_sk(sk);
    int completed = 0;

    SDBG("HOOK recvmsg ENTER sk=%px proto=%u user_data=%px state=%d",
        sk, sk->sk_protocol, sk->sk_user_data, sk->sk_state);
    if (st) {
        completed = (st->status & STCP_STATUS_HANDSHAKE_COMPLETE) > 0;
        SDBG("RecvMSG: STCP handshake complete?: %d", completed);
    }

    if (!completed) {
        if (sk->sk_socket && (sk->sk_socket->file->f_flags & O_NONBLOCK))
            return -EAGAIN;

        /* blocking: odota max 10s */
        if (!wait_for_completion_interruptible_timeout(&st->hs_done, 10*HZ))
            return -ETIMEDOUT;

        if (st->hs_result < 0)
            return st->hs_result;
    }

    if (!st || !st->session || !completed) {
        SDBG("RecvMSG: Calling orginal");
        return orginal_tcp_recvmsg(sk, msg, len, flags, recv_len);
    }

    if (!completed) {
        SDBG("RecvMSG: Handshake not done!");
        return -EAGAIN; /* tai -ENOTCONN */
    }

    if (!st->sk) {
        SDBG("No sk");
        return -ENOTCONN;
    }

    // Tulevalle datalle bufferi
    // TODO: Ota oma alloc funkkari käyttöön
    char *kbuf = kzalloc(len, GFP_KERNEL);
    
    if (!kbuf)
        return -ENOMEM;
        
    /* Delegate encrypted recv to Rust (BLOKING) */
    SDBG("RecvMSG: receiving via rust: (%px // %px), buffer: %px (%d bytes)",
            st->session, (void *)st->sk, kbuf, (int)len);

    ret = rust_exported_session_recvmsg(st->session, (void *)st->sk, kbuf, len, /* bloking */ 0);

    SDBG("[%px//%px] RecvMSG: received ret: %d",
            st->session, (void *)st->sk, (int)ret);

    if (ret > 0) {
        int cpret = memcpy_to_msg(msg, kbuf, ret);
        if (cpret < 0) {
            SDBG("[%px//%px] Error while copying to user buffer, ret %d",
                st->session, (void *)st->sk, cpret);
        } else {
            SDBG("[%px//%px] Copied to user buffer, %d bytes",
                st->session, (void *)st->sk, cpret);
        }
    } else {
        if (ret < 0 ) {
            SDBG("[%px//%px] Got error from recvmesg: %d",
                st->session, (void *)st->sk, (int)ret);
        } else {
            SDBG("[%px//%px] Peer closed, recvmesg ret: %d",
                st->session, st->sk, (int)ret);
        }
    }

    kfree(kbuf);

    SDBG("RecvMSG: final return: %d", (int)ret);
    return ret;
}

/* ----------------------------------------- */
/*  Destroy (STCP destroy)                   */
/* ----------------------------------------- */

void stcp_rust_glue_proto_op_destroy(struct sock *sock)
{
    struct stcp_sock *st;
    SDBG("@ Destroy of sock: %px", sock);
    /* Yritetään hakea STCP-per-socket struktia */
    // Ohittaa chekit .. loggaa vain..
    st = stcp_struct_get_st_from_sk_for_destroy(sock);
    int valid = st != NULL;

    u32 magic = 0;
    
    if (st) {
        magic = READ_ONCE(st->magic);
    }

    if(magic == STCP_MAGIC_DEAD)  { valid = 0; }

    if(magic != STCP_MAGIC_ALIVE) { valid = 0; }

    if (!valid) {
        SDBG("Not valid STCP sock in %px, this is dead socket.", sock);
    } else {

        SDBG("@ Destroy of stcp_sock: %px", st);

        SDBG("Destroying work queue...");
        destroy_the_work_queue(st);

        SDBG("Detaching session: %px", st);
        stcp_detach_from_sock(st);

        /* Vapauta Rust-session */
        if (st->session) {
            SDBG("@ Destroy of session: %px", st);
            rust_exported_session_destroy(st->session);
            st->session = NULL;
        }

        /* Vapauta STCP-struct */
        SDBG("@ Freeing stcp_sock structure...");
        stcp_struct_free_st(st);
    }

    // Kerneli hoitaa loput, vain omat jutut tässä siistata pois.
}

