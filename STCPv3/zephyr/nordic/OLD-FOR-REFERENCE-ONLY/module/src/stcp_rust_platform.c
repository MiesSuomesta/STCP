#include <errno.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>
#include <psa/crypto.h>
#include <stcp/stcp_internal.h>
#include <stcp/stcp_rust_ffi.h>

#include <stcp/stcp_x25519_soft.h>

LOG_MODULE_REGISTER(stcp_rust_platform, CONFIG_STCP_LOG_LEVEL);

static atomic_t crypto_ready;
static psa_status_t crypto_init_status = PSA_ERROR_BAD_STATE;

static const char *psa_status_name(psa_status_t status)
{
    switch (status) {
    case PSA_SUCCESS: return "PSA_SUCCESS";
    case PSA_ERROR_NOT_SUPPORTED: return "PSA_ERROR_NOT_SUPPORTED";
    case PSA_ERROR_INVALID_ARGUMENT: return "PSA_ERROR_INVALID_ARGUMENT";
    case PSA_ERROR_BAD_STATE: return "PSA_ERROR_BAD_STATE";
    case PSA_ERROR_INSUFFICIENT_MEMORY: return "PSA_ERROR_INSUFFICIENT_MEMORY";
    case PSA_ERROR_BUFFER_TOO_SMALL: return "PSA_ERROR_BUFFER_TOO_SMALL";
    case PSA_ERROR_NOT_PERMITTED: return "PSA_ERROR_NOT_PERMITTED";
    case PSA_ERROR_INVALID_SIGNATURE: return "PSA_ERROR_INVALID_SIGNATURE";
    default: return "PSA_ERROR_OTHER";
    }
}

static int psa_to_errno(psa_status_t status)
{
    switch (status) {
    case PSA_SUCCESS: return 0;
    case PSA_ERROR_INVALID_ARGUMENT: return -EINVAL;
    case PSA_ERROR_NOT_SUPPORTED: return -ENOTSUP;
    case PSA_ERROR_NOT_PERMITTED: return -EACCES;
    case PSA_ERROR_BAD_STATE: return -EIO;
    case PSA_ERROR_INSUFFICIENT_MEMORY: return -ENOMEM;
    case PSA_ERROR_BUFFER_TOO_SMALL: return -ENOSPC;
    case PSA_ERROR_INVALID_SIGNATURE: return -EBADMSG;
    default: return -EIO;
    }
}

static int stcp_crypto_ensure_ready(void)
{
    psa_status_t status;

    if (atomic_get(&crypto_ready) != 0) {
        return crypto_init_status == PSA_SUCCESS ? 0 : psa_to_errno(crypto_init_status);
    }

    status = psa_crypto_init();
    crypto_init_status = status;
    atomic_set(&crypto_ready, 1);

    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_CRYPTO)) {
        LOG_INF("STCP crypto init: status=%d (%s)", (int)status,
                psa_status_name(status));
    }
    return psa_to_errno(status);
}

static int stcp_crypto_init_hook(void)
{
    int rc = stcp_crypto_ensure_ready();
    if (rc != 0) {
        LOG_ERR("STCP crypto backend unavailable at boot: rc=%d psa=%d (%s)",
                rc, (int)crypto_init_status, psa_status_name(crypto_init_status));
    }
    return 0; /* Keep boot alive; the socket path reports the exact error. */
}
SYS_INIT(stcp_crypto_init_hook, APPLICATION, 85);

void *stcp_rust_kernel_alloc(size_t size)
{
    void *ptr = k_malloc(size ? size : 1U);
    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_ALLOC)) {
        LOG_DBG("rust alloc size=%u ptr=%p", (unsigned)size, ptr);
    }
    return ptr;
}

void stcp_rust_kernel_free(void *ptr)
{
    if (ptr != NULL) {
        k_free(ptr);
    }
}

__attribute__((noreturn)) void stcp_kernel_panic(void)
{
    LOG_ERR("Rust STCP core panic");
    k_panic();
    CODE_UNREACHABLE;
}

void stcp_kernel_wake_accept(void *owner)
{
    struct stcp_ctx *ctx = owner;
    if (ctx != NULL) {
        k_sem_give(&ctx->rust_event);
    }
}

void stcp_kernel_wake_recv(void *owner)
{
    struct stcp_ctx *ctx = owner;
    if (ctx != NULL) {
        k_sem_give(&ctx->rust_event);
    }
}

void stcp_kernel_debug_event(uint32_t event, uintptr_t ctx,
                             uintptr_t arg0, uintptr_t arg1)
{
    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_EVENTS)) {
        LOG_DBG("rust event=%u ctx=%p arg0=%u arg1=%u", event,
                (void *)ctx, (unsigned)arg0, (unsigned)arg1);
    }
}

static void nonce_to_bytes(uint64_t nonce, uint8_t out[12])
{
    memset(out, 0, 12);
    sys_put_le64(nonce, &out[4]);
}

static void x25519_clamp(uint8_t secret[32])
{
    secret[0] &= 248U;
    secret[31] &= 127U;
    secret[31] |= 64U;
}

int stcp_kernel_x25519_keypair(uint8_t *secret, uint8_t *public_key)
{
    psa_status_t status;
    int rc;

    if (secret == NULL || public_key == NULL) {
        return -EINVAL;
    }
    rc = stcp_crypto_ensure_ready();
    if (rc != 0) {
        LOG_ERR("X25519 keypair aborted: crypto init rc=%d", rc);
        return rc;
    }

    if (!IS_ENABLED(CONFIG_STCP_RUST_X25519_SOFTWARE)) {
        LOG_ERR("X25519 software backend disabled and PSA X25519 is unavailable");
        return -ENOTSUP;
    }

    LOG_INF("X25519 keypair start: backend=software-rfc7748 rng=PSA");
    status = psa_generate_random(secret, 32);
    LOG_INF("X25519 software psa_generate_random: status=%d (%s)",
            (int)status, psa_status_name(status));
    if (status != PSA_SUCCESS) {
        memset(secret, 0, 32);
        return psa_to_errno(status);
    }

    x25519_clamp(secret);
    rc = stcp_x25519_soft_public(public_key, secret);
    if (rc != 0) {
        LOG_ERR("X25519 software public-key calculation failed: rc=%d", rc);
        memset(secret, 0, 32);
        memset(public_key, 0, 32);
        return rc;
    }
    if (stcp_x25519_soft_is_all_zero(public_key)) {
        LOG_ERR("X25519 software public key is all zero");
        memset(secret, 0, 32);
        memset(public_key, 0, 32);
        return -EIO;
    }
    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_CRYPTO)) {
        LOG_HEXDUMP_INF(public_key, 32, "X25519 software public key");
    }
    LOG_INF("X25519 backend selected: software-rfc7748");
    return 0;
}

int stcp_kernel_x25519_shared(uint8_t *shared, const uint8_t *secret,
                              const uint8_t *peer)
{
    int rc;

    if (shared == NULL || secret == NULL || peer == NULL) {
        return -EINVAL;
    }
    if (!IS_ENABLED(CONFIG_STCP_RUST_X25519_SOFTWARE)) {
        return -ENOTSUP;
    }

    LOG_INF("X25519 shared-secret start: backend=software-rfc7748");
    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_CRYPTO)) {
        LOG_HEXDUMP_INF(peer, 32, "X25519 peer public key");
    }
    rc = stcp_x25519_soft(shared, secret, peer);
    if (rc != 0) {
        LOG_ERR("X25519 software shared-secret calculation failed: rc=%d", rc);
        memset(shared, 0, 32);
        return rc;
    }
    if (stcp_x25519_soft_is_all_zero(shared)) {
        LOG_ERR("X25519 software shared secret is all zero (low-order peer key)");
        memset(shared, 0, 32);
        return -EKEYREJECTED;
    }
    LOG_INF("X25519 shared-secret complete: backend=software-rfc7748");
    return 0;
}

int stcp_kernel_chacha_encrypt(const uint8_t *key, uint64_t nonce,
                               const uint8_t *aad, size_t aad_len,
                               const uint8_t *plain, size_t plain_len,
                               uint8_t *out, size_t out_len)
{
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id = 0;
    uint8_t nonce_bytes[12];
    size_t written = 0;
    psa_status_t status;
    int rc = stcp_crypto_ensure_ready();
    if (rc != 0) return rc;
    if (!key || !out || out_len < plain_len + 16) return -EINVAL;
    nonce_to_bytes(nonce, nonce_bytes);
    psa_set_key_type(&attr, PSA_KEY_TYPE_CHACHA20);
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_ENCRYPT);
    psa_set_key_algorithm(&attr, PSA_ALG_CHACHA20_POLY1305);
    status = psa_import_key(&attr, key, 32, &key_id);
    if (status == PSA_SUCCESS) {
        status = psa_aead_encrypt(key_id, PSA_ALG_CHACHA20_POLY1305,
                                  nonce_bytes, sizeof(nonce_bytes),
                                  aad, aad_len, plain, plain_len,
                                  out, out_len, &written);
    }
    if (key_id != 0) (void)psa_destroy_key(key_id);
    psa_reset_key_attributes(&attr);
    if (status != PSA_SUCCESS || written != plain_len + 16) {
        LOG_ERR("ChaCha20-Poly1305 encrypt failed: status=%d (%s) written=%u expected=%u",
                (int)status, psa_status_name(status), (unsigned)written,
                (unsigned)(plain_len + 16));
        return status == PSA_SUCCESS ? -EIO : psa_to_errno(status);
    }
    return 0;
}

int stcp_kernel_chacha_decrypt(const uint8_t *key, uint64_t nonce,
                               const uint8_t *aad, size_t aad_len,
                               const uint8_t *cipher, size_t cipher_len,
                               uint8_t *out, size_t out_len)
{
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id = 0;
    uint8_t nonce_bytes[12];
    size_t written = 0;
    psa_status_t status;
    int rc = stcp_crypto_ensure_ready();
    if (rc != 0) return rc;
    if (!key || !cipher || !out || cipher_len < 16 || out_len < cipher_len - 16) return -EINVAL;
    nonce_to_bytes(nonce, nonce_bytes);
    psa_set_key_type(&attr, PSA_KEY_TYPE_CHACHA20);
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attr, PSA_ALG_CHACHA20_POLY1305);
    status = psa_import_key(&attr, key, 32, &key_id);
    if (status == PSA_SUCCESS) {
        status = psa_aead_decrypt(key_id, PSA_ALG_CHACHA20_POLY1305,
                                  nonce_bytes, sizeof(nonce_bytes),
                                  aad, aad_len, cipher, cipher_len,
                                  out, out_len, &written);
    }
    if (key_id != 0) (void)psa_destroy_key(key_id);
    psa_reset_key_attributes(&attr);
    if (status != PSA_SUCCESS || written != cipher_len - 16) {
        LOG_ERR("ChaCha20-Poly1305 decrypt failed: status=%d (%s) written=%u expected=%u",
                (int)status, psa_status_name(status), (unsigned)written,
                (unsigned)(cipher_len - 16));
        return status == PSA_SUCCESS ? -EIO : psa_to_errno(status);
    }
    return 0;
}

int stcp_kernel_chacha_decrypt_in_place(const uint8_t *key, uint64_t nonce,
                                        const uint8_t *aad, size_t aad_len,
                                        uint8_t *cipher, size_t cipher_len)
{
    uint8_t *tmp;
    int rc;
    if (!cipher || cipher_len < 16) return -EINVAL;
    tmp = k_malloc(cipher_len - 16);
    if (!tmp) return -ENOMEM;
    rc = stcp_kernel_chacha_decrypt(key, nonce, aad, aad_len,
                                    cipher, cipher_len, tmp, cipher_len - 16);
    if (rc == 0) memcpy(cipher, tmp, cipher_len - 16);
    memset(tmp, 0, cipher_len - 16);
    k_free(tmp);
    return rc;
}

bool stcp_carrier_needs_reliability(const void *carrier)
{
    const struct stcp_ctx *ctx = carrier;
    return ctx != NULL && ctx->socket_type == SOCK_DGRAM;
}

void *stcp_carrier_create_udp_child(void *listener, void *child_rust_ctx,
                                    uint32_t peer_addr, uint16_t peer_port)
{
    ARG_UNUSED(listener); ARG_UNUSED(child_rust_ctx);
    ARG_UNUSED(peer_addr); ARG_UNUSED(peer_port);
    return NULL;
}

void stcp_carrier_destroy(void *carrier)
{
    ARG_UNUSED(carrier);
}

ssize_t stcp_carrier_send(void *carrier, const uint8_t *data,
                          size_t len, int flags)
{
    struct stcp_ctx *ctx = carrier;
    ssize_t rc;
    if (!ctx || ctx->carrier_fd < 0) return -ENOTCONN;
    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_WIRE)) {
        LOG_HEXDUMP_DBG(data, MIN(len, (size_t)CONFIG_STCP_RUST_HEXDUMP_BYTES),
                        "Rust STCP TX");
    }
    rc = zsock_send(ctx->carrier_fd, data, len, flags);
    if (rc < 0) return -errno;
    return rc;
}
