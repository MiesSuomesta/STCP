#ifndef STCP_H
#define STCP_H

#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define STCP_REASON_NEXT_STEP 3

#define SESSION_MAGIC 1398031184

#define STCP_HEADER_SIZE 16

#define STCP_MAX_FRAME (1024 * 1024)

#define STCP_TAG_BYTES 1398031184

#define STCP_VERSION 1

#define STCP_ECDH_SHARED_LEN 32

#define STCP_ECDH_PUB_XY_LEN 32

#define STCP_ECDH_PUB_LEN 64

#define STCP_MAX_TCP_PAYLOAD_SIZE 65495

typedef struct StcpEcdhPubKey {
  uint8_t x[STCP_ECDH_PUB_XY_LEN];
  uint8_t y[STCP_ECDH_PUB_XY_LEN];
} StcpEcdhPubKey;

typedef struct StcpEcdhSecret {
  uint8_t data[32];
} StcpEcdhSecret;

int32_t stcp_rust_alive(void);

#if defined(STCP_LINUX)
int stcp_random_get(uint8_t *buf, uintptr_t len);
#endif

extern int32_t stcp_end_of_life_for_sk(void *skvp, int err);

extern void *stcp_rust_kernel_alloc(uintptr_t size);

extern void stcp_rust_kernel_free(void *ptr);

extern void stcp_bug_null_ctx(void *sk);

extern void stcp_random_get(uint8_t *buf, uintptr_t len);

extern void stcp_sleep_ms(uint32_t ms);

extern void *stcp_misc_ecdh_public_key_new(void);

extern void *stcp_misc_ecdh_private_key_new(void);

extern void *stcp_misc_ecdh_shared_key_new(void);

extern int32_t stcp_misc_ecdh_public_key_size(void);

extern int32_t stcp_misc_ecdh_private_key_size(void);

extern int32_t stcp_misc_ecdh_shared_key_size(void);

extern void stcp_misc_ecdh_key_free(void *key);

extern int32_t stcp_crypto_generate_keypair(struct StcpEcdhPubKey *out_pub,
                                            struct StcpEcdhSecret *out_priv);

extern int32_t stcp_crypto_compute_shared(const struct StcpEcdhSecret *priv_key,
                                          const struct StcpEcdhPubKey *peer_pub,
                                          struct StcpEcdhSecret *out_shared);

extern int32_t stcp_is_debug_enabled(void);

extern void stcp_rust_log(int32_t level, const uint8_t *buf, uintptr_t len);

extern int32_t stcp_exported_rust_ctx_alive_count(void);

extern void stcp_random_get(uint8_t *buf, uintptr_t len);

int32_t rust_session_create(void **out_sess, void *transport);

int32_t rust_session_is_valid(void *sess);

int32_t rust_session_destroy(void *sess);

int32_t rust_session_reset_everything_now(void *sess);

int32_t rust_session_handshake_lte(void *sess_vp, void *transport);

int rust_exported_session_handshake_done(void *sess_void_ptr);

int rust_exported_session_create(void **out, void *transport);

intptr_t rust_exported_session_sendmsg(const void *sess_void_ptr,
                                       const void *transport_void_ptr,
                                       const void *buf,
                                       uintptr_t len);

intptr_t rust_exported_session_sendmsg_iovec(const void *sess_void_ptr,
                                             const void *transport_void_ptr,
                                             void *msg_void_ptr,
                                             int32_t flags,
                                             bool encrypted);

intptr_t rust_exported_session_recvmsg(void *sess_void_ptr,
                                       const void *payload_ptr,
                                       uintptr_t payload_len,
                                       void *out_buf_void_ptr,
                                       uintptr_t out_maxlen);

intptr_t rust_exported_session_destroy(void *sess_void_ptr);

extern intptr_t stcp_tcp_send(void *sock, const uint8_t *buf, uintptr_t len);

extern intptr_t stcp_tcp_send_iovec(void *sock, void *msg_vp, int flags);

extern intptr_t stcp_tcp_recv(void *sock,
                              uint8_t *buf,
                              uintptr_t len,
                              int32_t non_blocking,
                              uint32_t flags,
                              int *recv_len);

void stcp_module_rust_enter(void);

void stcp_module_rust_exit(void);

int32_t stcp_crypto_selftest(void);

#endif  /* STCP_H */
