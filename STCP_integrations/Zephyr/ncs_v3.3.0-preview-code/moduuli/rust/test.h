#ifndef STCP_H
#define STCP_H

#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define EPERM 1

#define ENOENT 2

#define ESRCH 3

#define EINTR 4

#define EIO 5

#define ENXIO 6

#define E2BIG 7

#define ENOEXEC 8

#define EBADF 9

#define ECHILD 10

#define EAGAIN 11

#define ENOMEM 12

#define EACCES 13

#define EFAULT 14

#define EBUSY 16

#define EEXIST 17

#define EXDEV 18

#define ENODEV 19

#define ENOTDIR 20

#define EISDIR 21

#define EINVAL 22

#define ENFILE 23

#define EMFILE 24

#define ENOTTY 25

#define ETXTBSY 26

#define EFBIG 27

#define ENOSPC 28

#define ESPIPE 29

#define EROFS 30

#define EMLINK 31

#define EPIPE 32

#define EDOM 33

#define ERANGE 34

#define EMSGSIZE 90

#define EPROTO 71

#define ENOTCONN 107

#define ECONNRESET 104

#define ECONNREFUSED 111

#define ECONNABORTED 103

#define ETIMEDOUT 116

#define EINPROGRESS 115

#define EHOSTUNREACH 113

#define ENETUNREACH 128

#define STCP_REASON_NEXT_STEP 3

#define STCP_TAG_BYTES 1398031184

#define STCP_VERSION 1

#define STCP_ECDH_SHARED_LEN 32

#define STCP_ECDH_PUB_XY_LEN 32

#define STCP_ECDH_PUB_LEN 64

#define STCP_MAX_TCP_PAYLOAD_SIZE 65495

typedef struct ProtoSession ProtoSession;

typedef struct StcpEcdhPubKey {
  uint8_t x[STCP_ECDH_PUB_XY_LEN];
  uint8_t y[STCP_ECDH_PUB_XY_LEN];
} StcpEcdhPubKey;

typedef struct StcpEcdhSecret {
  uint8_t data[32];
} StcpEcdhSecret;

typedef struct kernel_socket {
  int32_t fd;
  void *kctx;
  void *resolved_host;
} kernel_socket;

int32_t stcp_rust_alive(void);

#if defined(STCP_LINUX)
int stcp_random_get(uint8_t *buf, uintptr_t len);
#endif

extern int32_t stcp_end_of_life_for_sk(void *skvp, int err);

extern void *stcp_rust_kernel_alloc(uintptr_t size);

extern void stcp_rust_kernel_free(void *ptr);

extern int stcp_tcp_raw_send(void *sk, const uint8_t *buf, uintptr_t len);

extern int stcp_tcp_raw_recv(void *sk, uint8_t *buf, uintptr_t len, int flags, int *recv_len);

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

extern int32_t stcp_exported_rust_ctx_alive_count(void);

extern void stcp_random_get(uint8_t *buf, uintptr_t len);

extern void stcp_rust_log(int level, const uint8_t *buf, uintptr_t len);

int32_t rust_session_create(void **out_sess, struct kernel_socket *transport);

int32_t rust_session_destroy(void *sess);

int32_t rust_session_reset_everything_now(void *sess);

int32_t rust_session_client_handshake_lte(struct ProtoSession *sess,
                                          struct kernel_socket *transport);

int32_t rust_session_server_handshake_lte(struct ProtoSession *sess,
                                          struct kernel_socket *transport);

int32_t rust_exported_session_handshake_pump(struct ProtoSession *sess,
                                             struct kernel_socket *transport,
                                             int32_t reason);

int rust_exported_session_handshake_done(void *sess_void_ptr);

int rust_exported_data_client_ready_worker(void *sess_void_ptr, void *transport_void_ptr);

int rust_exported_data_server_ready_worker(void *sess_void_ptr, void *transport_void_ptr);

int rust_exported_session_create(void **out, void *transport);

int rust_exported_session_client_handshake(void *sess_void_ptr);

int rust_exported_session_server_handshake(void *sess_void_ptr);

intptr_t rust_exported_session_sendmsg(const void *sess_void_ptr,
                                       const void *transport_void_ptr,
                                       const void *buf,
                                       uintptr_t len);

intptr_t rust_exported_session_recvmsg(void *sess_void_ptr,
                                       void *transport_void_ptr,
                                       void *out_buf_void_ptr,
                                       uintptr_t out_maxlen,
                                       int32_t non_blocking);

intptr_t rust_exported_session_destroy(void *sess_void_ptr);

extern intptr_t stcp_tcp_send(struct kernel_socket *sock, const uint8_t *buf, uintptr_t len);

extern intptr_t stcp_tcp_recv(struct kernel_socket *sock,
                              uint8_t *buf,
                              uintptr_t len,
                              int32_t non_blocking,
                              uint32_t flags,
                              int *recv_len);

void stcp_module_rust_enter(void);

void stcp_module_rust_exit(void);

#endif  /* STCP_H */
