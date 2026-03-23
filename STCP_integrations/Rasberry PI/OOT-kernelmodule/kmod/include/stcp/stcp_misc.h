#pragma once 

#include <linux/slab.h>
#include <linux/types.h>
#include <stcp/rust_alloc.h>
#include <stcp/stcp_misc.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <stcp/proto_layer.h>

#define STCP_ECDH_PUB_LEN       64
#define STCP_ECDH_PUB_XY_LEN    32
#define STCP_ECDH_PRIVATE_LEN   32
#define STCP_ECDH_SHARED_LEN    32

struct stcp_ecdh_public_key {
    u8 x[STCP_ECDH_PUB_XY_LEN];
    u8 y[STCP_ECDH_PUB_XY_LEN];
};

struct stcp_ecdh_shared_key {
    u8     data[STCP_ECDH_SHARED_LEN];
    size_t len;
};

struct stcp_ecdh_private_key {
    u8     data[STCP_ECDH_PRIVATE_LEN];
    size_t len;
};

void *stcp_misc_ecdh_public_key_new(void);
void *stcp_misc_ecdh_private_key_new(void);
void *stcp_misc_ecdh_shared_key_new(void);

int stcp_misc_ecdh_private_key_size(void);
int stcp_misc_ecdh_public_key_size(void);
int stcp_misc_ecdh_shared_key_size(void);


void stcp_misc_ecdh_key_free(void *pFreePtr);

struct stcp_sock; 

int stcp_proto_setup(void);
int is_stcp_magic_ok(struct stcp_sock* st);
int is_handshake_in_progress(struct sock *sk);

void stcp_struct_session_destroy_request(struct stcp_sock *st, int reason);
void stcp_struct_destroy_workfn(struct work_struct *work);


inline int stcp_state_is_handshake_complete(struct stcp_sock *st);
inline void stcp_hs_mark_complete(struct stcp_sock *st);
inline void stcp_state_restore_data_ready_hook(struct stcp_sock *st);
inline int stcp_state_try_start_handshake(struct stcp_sock *st, int server_side);
