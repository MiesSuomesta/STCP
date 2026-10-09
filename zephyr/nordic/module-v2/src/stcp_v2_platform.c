#include <errno.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>
#include <psa/crypto.h>
#include <stcp/stcp_v2_internal.h>
#include <stcp/stcp_rust_ffi.h>

#include <stcp/stcp_x25519_soft.h>

LOG_MODULE_REGISTER(stcp_rust_platform, CONFIG_STCP_V2_LOG_LEVEL);

/* Silent Rust-owned stage recorder; no UART/logging in crypto context. */
extern void stcp_rust_crypto_diag_stage_set(size_t stage);


static atomic_t crypto_ready;
static psa_status_t crypto_init_status = PSA_ERROR_BAD_STATE;

/*
 * X25519 reset diagnostic.
 *
 * Kept in .noinit so a warm reset can reveal the last stage reached even
 * when the UART/debug transport disappears before the final log is emitted.
 * No logging is done while advancing the stage in the shared-secret path.
 */
#define STCP_X25519_RESET_MAGIC 0x58323535U /* "X255" */

struct stcp_x25519_reset_diag {
    uint32_t magic;
    uint32_t stage;
    uint32_t cycle;
    uintptr_t thread;
    uintptr_t shared;
    uintptr_t secret;
    uintptr_t peer;
};

__noinit static struct stcp_x25519_reset_diag stcp_x25519_reset_diag;

static inline void stcp_x25519_reset_stage(uint32_t stage,
                                           const uint8_t *shared,
                                           const uint8_t *secret,
                                           const uint8_t *peer)
{
    stcp_x25519_reset_diag.magic = STCP_X25519_RESET_MAGIC;
    stcp_x25519_reset_diag.stage = stage;
    stcp_x25519_reset_diag.cycle = k_cycle_get_32();
    stcp_x25519_reset_diag.thread = (uintptr_t)k_current_get();
    stcp_x25519_reset_diag.shared = (uintptr_t)shared;
    stcp_x25519_reset_diag.secret = (uintptr_t)secret;
    stcp_x25519_reset_diag.peer = (uintptr_t)peer;
}

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

    if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
        LOG_INF("STCP crypto init: status=%d (%s)", (int)status,
                psa_status_name(status));
    }
    return psa_to_errno(status);
}

static int stcp_crypto_init_hook(void)
{
    if (stcp_x25519_reset_diag.magic == STCP_X25519_RESET_MAGIC) {
        printk("X25519RESET PREV stage=%u cycle=%u tid=%p shared=%p secret=%p peer=%p\n",
               (unsigned int)stcp_x25519_reset_diag.stage,
               (unsigned int)stcp_x25519_reset_diag.cycle,
               (void *)stcp_x25519_reset_diag.thread,
               (void *)stcp_x25519_reset_diag.shared,
               (void *)stcp_x25519_reset_diag.secret,
               (void *)stcp_x25519_reset_diag.peer);
    } else {
        printk("X25519RESET PREV none\n");
    }

    memset(&stcp_x25519_reset_diag, 0, sizeof(stcp_x25519_reset_diag));

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
    if (IS_ENABLED(CONFIG_STCP_V2_TRACE_ALLOC)) {
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
    stcp_v2_signal((struct stcp_v2_socket *)owner);
}

void stcp_kernel_wake_recv(void *owner)
{
    stcp_v2_signal((struct stcp_v2_socket *)owner);
}

void stcp_kernel_debug_event(uint32_t event, uintptr_t ctx,
                             uintptr_t arg0, uintptr_t arg1)
{
    if (IS_ENABLED(CONFIG_STCP_V2_TRACE_EVENTS)) {
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


#define STCP_KDF_KEY_LEN 32U

static int stcp_psa_hash_sha256_3(const uint8_t *a, size_t a_len,
                                  const uint8_t *b, size_t b_len,
                                  const uint8_t *c, size_t c_len,
                                  uint8_t out[STCP_KDF_KEY_LEN])
{
    uint8_t *input;
    size_t total;
    size_t off = 0;
    size_t written = 0;
    psa_status_t status;

    if (a_len > SIZE_MAX - b_len || a_len + b_len > SIZE_MAX - c_len) {
        return -EOVERFLOW;
    }
    total = a_len + b_len + c_len;

    input = k_malloc(total ? total : 1U);
    if (input == NULL) {
        return -ENOMEM;
    }

    if (a_len) {
        memcpy(input + off, a, a_len);
        off += a_len;
    }
    if (b_len) {
        memcpy(input + off, b, b_len);
        off += b_len;
    }
    if (c_len) {
        memcpy(input + off, c, c_len);
    }

    status = psa_hash_compute(PSA_ALG_SHA_256,
                              input, total,
                              out, STCP_KDF_KEY_LEN,
                              &written);

    memset(input, 0, total);
    k_free(input);

    if (status != PSA_SUCCESS) {
        LOG_ERR("SHA256 KDF hash failed: status=%d (%s)",
                (int)status, psa_status_name(status));
        return psa_to_errno(status);
    }
    if (written != STCP_KDF_KEY_LEN) {
        return -EIO;
    }
    return 0;
}

static int stcp_psa_hmac_sha256_2(const uint8_t *key, size_t key_len,
                                  const uint8_t *a, size_t a_len,
                                  const uint8_t *b, size_t b_len,
                                  uint8_t out[STCP_KDF_KEY_LEN])
{
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id = 0;
    uint8_t *input;
    size_t total;
    size_t written = 0;
    psa_status_t status;

    if (a_len > SIZE_MAX - b_len) {
        return -EOVERFLOW;
    }
    total = a_len + b_len;

    input = k_malloc(total ? total : 1U);
    if (input == NULL) {
        return -ENOMEM;
    }

    if (a_len) {
        memcpy(input, a, a_len);
    }
    if (b_len) {
        memcpy(input + a_len, b, b_len);
    }

    psa_set_key_type(&attr, PSA_KEY_TYPE_HMAC);
    psa_set_key_bits(&attr, key_len * 8U);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_algorithm(&attr, PSA_ALG_HMAC(PSA_ALG_SHA_256));

    status = psa_import_key(&attr, key, key_len, &key_id);
    if (status == PSA_SUCCESS) {
        status = psa_mac_compute(key_id,
                                 PSA_ALG_HMAC(PSA_ALG_SHA_256),
                                 input, total,
                                 out, STCP_KDF_KEY_LEN,
                                 &written);
    }

    if (key_id != 0) {
        (void)psa_destroy_key(key_id);
    }
    psa_reset_key_attributes(&attr);

    memset(input, 0, total);
    k_free(input);

    if (status != PSA_SUCCESS) {
        LOG_ERR("HMAC-SHA256 KDF failed: status=%d (%s)",
                (int)status, psa_status_name(status));
        return psa_to_errno(status);
    }
    if (written != STCP_KDF_KEY_LEN) {
        return -EIO;
    }
    return 0;
}

int stcp_kernel_derive_session_keys(const uint8_t *shared,
                                    const uint8_t *client_pub,
                                    const uint8_t *server_pub,
                                    uint8_t *client_to_server,
                                    uint8_t *server_to_client)
{
    static const uint8_t domain[] = "STCPv2-HKDF-SHA256";
    static const uint8_t client_label[] = "STCPv2 client to server key";
    static const uint8_t server_label[] = "STCPv2 server to client key";
    static const uint8_t counter = 1U;
    uint8_t salt[STCP_KDF_KEY_LEN];
    uint8_t prk[STCP_KDF_KEY_LEN];
    int rc;

    if (shared == NULL || client_pub == NULL || server_pub == NULL ||
        client_to_server == NULL || server_to_client == NULL) {
        return -EINVAL;
    }

    rc = stcp_crypto_ensure_ready();
    if (rc != 0) {
        return rc;
    }

    rc = stcp_psa_hash_sha256_3(domain, sizeof(domain) - 1U,
                                client_pub, STCP_KDF_KEY_LEN,
                                server_pub, STCP_KDF_KEY_LEN,
                                salt);
    if (rc != 0) {
        goto out;
    }

    rc = stcp_psa_hmac_sha256_2(salt, sizeof(salt),
                                shared, STCP_KDF_KEY_LEN,
                                NULL, 0,
                                prk);
    if (rc != 0) {
        goto out;
    }

    rc = stcp_psa_hmac_sha256_2(prk, sizeof(prk),
                                client_label, sizeof(client_label) - 1U,
                                &counter, sizeof(counter),
                                client_to_server);
    if (rc != 0) {
        goto out;
    }

    rc = stcp_psa_hmac_sha256_2(prk, sizeof(prk),
                                server_label, sizeof(server_label) - 1U,
                                &counter, sizeof(counter),
                                server_to_client);
    if (rc != 0) {
        goto out;
    }

    if (memcmp(client_to_server, server_to_client, STCP_KDF_KEY_LEN) == 0) {
        rc = -EKEYREJECTED;
        goto out;
    }

    if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
        LOG_DBG("session KDF complete via PSA SHA256/HMAC");
    }

out:
    if (rc != 0) {
        memset(client_to_server, 0, STCP_KDF_KEY_LEN);
        memset(server_to_client, 0, STCP_KDF_KEY_LEN);
    }
    memset(salt, 0, sizeof(salt));
    memset(prk, 0, sizeof(prk));
    return rc;
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

    if (!IS_ENABLED(CONFIG_STCP_V2_X25519_SOFTWARE)) {
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
    if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
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

    /*
     * Keep the X25519 backend consistent with keypair generation.
     *
     * The nRF Connect SDK PSA imported Montgomery/X25519 private-key path
     * can block inside psa_import_key() on this target.  The RFC7748
     * software implementation is already used for public-key generation,
     * is allocation-free, and clamps a private copy of the scalar itself.
     */
    if (!IS_ENABLED(CONFIG_STCP_V2_X25519_SOFTWARE)) {
        LOG_ERR("X25519 software shared-secret backend disabled");
        memset(shared, 0, 32U);
        return -ENOTSUP;
    }

    /*
     * Deliberately silent diagnostic stages.  If the board resets or the
     * debug UART vanishes here, the next boot prints the last retained stage.
     *
     *  1 = returned from the start LOG_INF
     *  2 = immediately before stcp_x25519_soft()
     *  3 = stcp_x25519_soft() returned
     *  4 = shared secret passed the all-zero check
     */
    stcp_rust_crypto_diag_stage_set(50U);
    stcp_x25519_reset_stage(1U, shared, secret, peer);
    stcp_x25519_reset_stage(2U, shared, secret, peer);

    stcp_rust_crypto_diag_stage_set(60U);
    rc = stcp_x25519_soft(shared, secret, peer);
    stcp_rust_crypto_diag_stage_set(70U);

    stcp_x25519_reset_stage(3U, shared, secret, peer);

    if (rc != 0) {
        memset(shared, 0, 32U);
        return rc;
    }

    if (stcp_x25519_soft_is_all_zero(shared)) {
        memset(shared, 0, 32U);
        return -EKEYREJECTED;
    }

    stcp_x25519_reset_stage(4U, shared, secret, peer);
    stcp_rust_crypto_diag_stage_set(80U);

    return 0;
}

#include "stcp_chachapoly_soft.h"

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

    if (rc != 0) {
        return rc;
    }
    if (key == NULL || out == NULL || out_len < plain_len + 16U ||
        (plain_len != 0U && plain == NULL) ||
        (aad_len != 0U && aad == NULL)) {
        return -EINVAL;
    }

    nonce_to_bytes(nonce, nonce_bytes);

    psa_set_key_type(&attr, PSA_KEY_TYPE_CHACHA20);
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_ENCRYPT);
    psa_set_key_algorithm(&attr, PSA_ALG_CHACHA20_POLY1305);

    status = psa_import_key(&attr, key, 32U, &key_id);
    if (status == PSA_SUCCESS) {
        status = psa_aead_encrypt(key_id, PSA_ALG_CHACHA20_POLY1305,
                                  nonce_bytes, sizeof(nonce_bytes),
                                  aad, aad_len,
                                  plain, plain_len,
                                  out, out_len,
                                  &written);
    }

    if (key_id != 0) {
        (void)psa_destroy_key(key_id);
    }
    psa_reset_key_attributes(&attr);

    if (status == PSA_SUCCESS && written == plain_len + 16U) {
        if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
            LOG_DBG("ChaCha20-Poly1305 encrypt backend=PSA len=%u",
                    (unsigned int)plain_len);
        }
        return 0;
    }

    if (status == PSA_ERROR_NOT_SUPPORTED) {
        LOG_INF("ChaCha20-Poly1305 encrypt: PSA unsupported, using software RFC8439 fallback");
        rc = stcp_soft_chachapoly_encrypt(key, nonce_bytes,
                                          aad, aad_len,
                                          plain, plain_len,
                                          out);
        if (rc != 0) {
            LOG_ERR("ChaCha20-Poly1305 software encrypt failed: rc=%d", rc);
        } else if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
            LOG_DBG("ChaCha20-Poly1305 encrypt backend=software-rfc8439 len=%u",
                    (unsigned int)plain_len);
        }
        return rc;
    }

    LOG_ERR("ChaCha20-Poly1305 encrypt failed: status=%d (%s) written=%u expected=%u",
            (int)status, psa_status_name(status),
            (unsigned int)written,
            (unsigned int)(plain_len + 16U));

    return status == PSA_SUCCESS ? -EIO : psa_to_errno(status);
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

    if (rc != 0) {
        return rc;
    }
    if (key == NULL || cipher == NULL || out == NULL ||
        cipher_len < 16U || out_len < cipher_len - 16U ||
        (aad_len != 0U && aad == NULL)) {
        return -EINVAL;
    }

    nonce_to_bytes(nonce, nonce_bytes);

    psa_set_key_type(&attr, PSA_KEY_TYPE_CHACHA20);
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attr, PSA_ALG_CHACHA20_POLY1305);

    status = psa_import_key(&attr, key, 32U, &key_id);
    if (status == PSA_SUCCESS) {
        status = psa_aead_decrypt(key_id, PSA_ALG_CHACHA20_POLY1305,
                                  nonce_bytes, sizeof(nonce_bytes),
                                  aad, aad_len,
                                  cipher, cipher_len,
                                  out, out_len,
                                  &written);
    }

    if (key_id != 0) {
        (void)psa_destroy_key(key_id);
    }
    psa_reset_key_attributes(&attr);

    if (status == PSA_SUCCESS && written == cipher_len - 16U) {
        if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
            LOG_DBG("ChaCha20-Poly1305 decrypt backend=PSA len=%u",
                    (unsigned int)written);
        }
        return 0;
    }

    if (status == PSA_ERROR_NOT_SUPPORTED) {
        LOG_INF("ChaCha20-Poly1305 decrypt: PSA unsupported, using software RFC8439 fallback");
        rc = stcp_soft_chachapoly_decrypt(key, nonce_bytes,
                                          aad, aad_len,
                                          cipher, cipher_len,
                                          out);
        if (rc != 0) {
            LOG_ERR("ChaCha20-Poly1305 software decrypt failed: rc=%d", rc);
        } else if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
            LOG_DBG("ChaCha20-Poly1305 decrypt backend=software-rfc8439 len=%u",
                    (unsigned int)(cipher_len - 16U));
        }
        return rc;
    }

    LOG_ERR("ChaCha20-Poly1305 decrypt failed: status=%d (%s) written=%u expected=%u",
            (int)status, psa_status_name(status),
            (unsigned int)written,
            (unsigned int)(cipher_len - 16U));

    return status == PSA_SUCCESS ? -EIO : psa_to_errno(status);
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
    const struct stcp_v2_carrier *c = carrier;
    return c != NULL && c->socket_type == SOCK_DGRAM;
}

void *stcp_carrier_create_udp_child(void *listener, void *child_rust_ctx,
                                    uint32_t peer_addr, uint16_t peer_port)
{
    ARG_UNUSED(child_rust_ctx);
    return stcp_v2_carrier_udp_child((struct stcp_v2_carrier *)listener,
                                     peer_addr, peer_port);
}

void stcp_carrier_destroy(void *carrier)
{
    stcp_v2_carrier_free((struct stcp_v2_carrier *)carrier);
}

ssize_t stcp_carrier_send(void *carrier, const uint8_t *data,
                          size_t len, int flags)
{
    struct stcp_v2_carrier *c = (struct stcp_v2_carrier *)carrier;
    size_t done = 0;

    if (c == NULL || (data == NULL && len != 0U)) {
        return -EINVAL;
    }

    if (IS_ENABLED(CONFIG_STCP_V2_TRACE_WIRE)) {
        LOG_HEXDUMP_DBG(data, MIN(len, (size_t)CONFIG_STCP_V2_HEXDUMP_BYTES),
                        "STCP wire TX");
    }

    /*
     * Datagram writes are atomic.  Keep their existing one-call semantics:
     * STCP reliability needs to observe a datagram as one frame rather than
     * accidentally splitting it into multiple UDP packets.
     */
    if (c->socket_type == SOCK_DGRAM) {
        return stcp_v2_carrier_send_wire(c, data, len, flags);
    }

    /*
     * SOCK_STREAM may legally return a positive short write.  That is not an
     * I/O failure: the unwritten tail still belongs to the same STCP wire
     * frame and must be pushed before reporting success to the Rust core.
     *
     * This mirrors normal write_all()/send_all() semantics and prevents a
     * short native TCP write from being mapped to -EIO by the upper layer.
     */
    while (done < len) {
        ssize_t rc = stcp_v2_carrier_send_wire(c, data + done,
                                               len - done, flags);

        if (rc > 0) {
            if ((size_t)rc > len - done) {
                LOG_ERR("carrier TX invalid short-write result rc=%d remaining=%u",
                        (int)rc, (unsigned int)(len - done));
                return -EIO;
            }

            done += (size_t)rc;

            if (done < len) {
                LOG_WRN("carrier TX partial write rc=%d progress=%u/%u; continuing",
                        (int)rc, (unsigned int)done, (unsigned int)len);
            }
            continue;
        }

        if (rc == 0) {
            LOG_ERR("carrier TX zero write progress=%u/%u",
                    (unsigned int)done, (unsigned int)len);
            return -EPIPE;
        }

        /*
         * stcp_v2_carrier_send_wire() returns negative errno values.
         * EINTR is transient and the same bytes have not been consumed.
         */
        if (rc == -EINTR) {
            continue;
        }

        LOG_ERR("carrier TX failed rc=%d progress=%u/%u",
                (int)rc, (unsigned int)done, (unsigned int)len);
        return rc;
    }

    return (ssize_t)done;
}
