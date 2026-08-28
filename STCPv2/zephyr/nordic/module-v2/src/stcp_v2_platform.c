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

    if (IS_ENABLED(CONFIG_STCP_V2_TRACE_CRYPTO)) {
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
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t key_id = 0;
    size_t shared_len = 0;
    psa_status_t status;
    int rc;

    if (shared == NULL || secret == NULL || peer == NULL) {
        return -EINVAL;
    }

    rc = stcp_crypto_ensure_ready();
    if (rc != 0) {
        return rc;
    }

    /*
     * nRF Connect SDK's PSA X25519 path supports imported Montgomery-255
     * private keys for ECDH even on configurations where psa_generate_key()
     * for that key type returns PSA_ERROR_NOT_SUPPORTED.
     *
     * Keep keypair generation on the already-working RFC7748 software path,
     * but move the expensive shared-secret scalar multiplication out of the
     * stcp-v2-rx stack into the PSA backend.
     */
    psa_set_key_type(&attr,
                     PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_MONTGOMERY));
    psa_set_key_bits(&attr, 255);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDH);

    LOG_INF("X25519 shared-secret start: backend=PSA imported-key");

    LOG_ERR("X25519DIAG IMPORT ENTER");

    status = psa_import_key(&attr, secret, 32U, &key_id);

    LOG_ERR("X25519DIAG IMPORT RETURN status=%d key_id=%u",
            (int)status, (unsigned int)key_id);

    if (status != PSA_SUCCESS) {
        LOG_ERR("X25519 psa_import_key failed: status=%d (%s)",
                (int)status, psa_status_name(status));
        memset(shared, 0, 32U);
        psa_reset_key_attributes(&attr);
        return psa_to_errno(status);
    }

    LOG_ERR("X25519DIAG AGREEMENT ENTER key_id=%u",
            (unsigned int)key_id);

    status = psa_raw_key_agreement(PSA_ALG_ECDH,
                                   key_id,
                                   peer, 32U,
                                   shared, 32U,
                                   &shared_len);

    LOG_ERR("X25519DIAG AGREEMENT RETURN status=%d len=%u",
            (int)status, (unsigned int)shared_len);

    (void)psa_destroy_key(key_id);
    psa_reset_key_attributes(&attr);

    if (status != PSA_SUCCESS || shared_len != 32U) {
        LOG_ERR("X25519 psa_raw_key_agreement failed: status=%d len=%u (%s)",
                (int)status,
                (unsigned int)shared_len,
                psa_status_name(status));
        memset(shared, 0, 32U);
        return status == PSA_SUCCESS ? -EIO : psa_to_errno(status);
    }

    if (stcp_x25519_soft_is_all_zero(shared)) {
        LOG_ERR("X25519 PSA shared secret is all zero (low-order peer key)");
        memset(shared, 0, 32U);
        return -EKEYREJECTED;
    }

    LOG_INF("X25519 shared-secret complete: backend=PSA imported-key");
    return 0;
}


/*
 * RFC 8439 software ChaCha20-Poly1305 fallback.
 *
 * This path is used only when the PSA backend explicitly reports
 * PSA_ERROR_NOT_SUPPORTED for ChaCha20-Poly1305.  It is allocation-free:
 * Poly1305 is streamed over AAD/ciphertext, so large benchmark frames do not
 * consume a second payload-sized heap buffer.
 */
static uint32_t stcp_soft_load32_le(const uint8_t *p)
{
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void stcp_soft_store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t stcp_soft_rotl32(uint32_t v, unsigned int n)
{
    return (v << n) | (v >> (32U - n));
}

static void stcp_soft_chacha_qr(uint32_t *a, uint32_t *b,
                                uint32_t *c, uint32_t *d)
{
    *a += *b; *d ^= *a; *d = stcp_soft_rotl32(*d, 16);
    *c += *d; *b ^= *c; *b = stcp_soft_rotl32(*b, 12);
    *a += *b; *d ^= *a; *d = stcp_soft_rotl32(*d, 8);
    *c += *d; *b ^= *c; *b = stcp_soft_rotl32(*b, 7);
}

static void stcp_soft_chacha20_block(const uint8_t key[32], uint32_t counter,
                                     const uint8_t nonce[12], uint8_t out[64])
{
    static const uint8_t sigma[16] = "expand 32-byte k";
    uint32_t x[16];
    uint32_t w[16];
    int i;

    x[0] = stcp_soft_load32_le(sigma);
    x[1] = stcp_soft_load32_le(sigma + 4);
    x[2] = stcp_soft_load32_le(sigma + 8);
    x[3] = stcp_soft_load32_le(sigma + 12);
    for (i = 0; i < 8; i++) {
        x[4 + i] = stcp_soft_load32_le(key + 4 * i);
    }
    x[12] = counter;
    x[13] = stcp_soft_load32_le(nonce);
    x[14] = stcp_soft_load32_le(nonce + 4);
    x[15] = stcp_soft_load32_le(nonce + 8);

    memcpy(w, x, sizeof(x));

    for (i = 0; i < 10; i++) {
        stcp_soft_chacha_qr(&w[0], &w[4], &w[8], &w[12]);
        stcp_soft_chacha_qr(&w[1], &w[5], &w[9], &w[13]);
        stcp_soft_chacha_qr(&w[2], &w[6], &w[10], &w[14]);
        stcp_soft_chacha_qr(&w[3], &w[7], &w[11], &w[15]);

        stcp_soft_chacha_qr(&w[0], &w[5], &w[10], &w[15]);
        stcp_soft_chacha_qr(&w[1], &w[6], &w[11], &w[12]);
        stcp_soft_chacha_qr(&w[2], &w[7], &w[8], &w[13]);
        stcp_soft_chacha_qr(&w[3], &w[4], &w[9], &w[14]);
    }

    for (i = 0; i < 16; i++) {
        stcp_soft_store32_le(out + 4 * i, w[i] + x[i]);
    }

    memset(x, 0, sizeof(x));
    memset(w, 0, sizeof(w));
}

static void stcp_soft_chacha20_xor(const uint8_t key[32],
                                   const uint8_t nonce[12],
                                   uint32_t counter,
                                   const uint8_t *in,
                                   uint8_t *out,
                                   size_t len)
{
    uint8_t block[64];

    while (len != 0U) {
        size_t n;
        size_t i;

        stcp_soft_chacha20_block(key, counter++, nonce, block);
        n = len < sizeof(block) ? len : sizeof(block);

        for (i = 0; i < n; i++) {
            out[i] = in[i] ^ block[i];
        }

        in += n;
        out += n;
        len -= n;
    }

    memset(block, 0, sizeof(block));
}

struct stcp_soft_poly1305 {
    uint32_t r0, r1, r2, r3, r4;
    uint32_t s1, s2, s3, s4;
    uint32_t h0, h1, h2, h3, h4;
    uint32_t pad0, pad1, pad2, pad3;
};

static void stcp_soft_poly1305_init(struct stcp_soft_poly1305 *st,
                                    const uint8_t key[32])
{
    st->r0 = stcp_soft_load32_le(key + 0) & 0x3ffffffU;
    st->r1 = (stcp_soft_load32_le(key + 3) >> 2) & 0x3ffff03U;
    st->r2 = (stcp_soft_load32_le(key + 6) >> 4) & 0x3ffc0ffU;
    st->r3 = (stcp_soft_load32_le(key + 9) >> 6) & 0x3f03fffU;
    st->r4 = (stcp_soft_load32_le(key + 12) >> 8) & 0x00fffffU;

    st->s1 = st->r1 * 5U;
    st->s2 = st->r2 * 5U;
    st->s3 = st->r3 * 5U;
    st->s4 = st->r4 * 5U;

    st->h0 = 0U;
    st->h1 = 0U;
    st->h2 = 0U;
    st->h3 = 0U;
    st->h4 = 0U;

    st->pad0 = stcp_soft_load32_le(key + 16);
    st->pad1 = stcp_soft_load32_le(key + 20);
    st->pad2 = stcp_soft_load32_le(key + 24);
    st->pad3 = stcp_soft_load32_le(key + 28);
}

static void stcp_soft_poly1305_block(struct stcp_soft_poly1305 *st,
                                     const uint8_t m[16])
{
    uint32_t t0 = stcp_soft_load32_le(m);
    uint32_t t1 = stcp_soft_load32_le(m + 4);
    uint32_t t2 = stcp_soft_load32_le(m + 8);
    uint32_t t3 = stcp_soft_load32_le(m + 12);
    uint32_t c;
    uint64_t d0, d1, d2, d3, d4;

    st->h0 += t0 & 0x3ffffffU;
    st->h1 += ((t0 >> 26) | (t1 << 6)) & 0x3ffffffU;
    st->h2 += ((t1 >> 20) | (t2 << 12)) & 0x3ffffffU;
    st->h3 += ((t2 >> 14) | (t3 << 18)) & 0x3ffffffU;
    st->h4 += (t3 >> 8) | (1U << 24);

    d0 = (uint64_t)st->h0 * st->r0 +
         (uint64_t)st->h1 * st->s4 +
         (uint64_t)st->h2 * st->s3 +
         (uint64_t)st->h3 * st->s2 +
         (uint64_t)st->h4 * st->s1;
    d1 = (uint64_t)st->h0 * st->r1 +
         (uint64_t)st->h1 * st->r0 +
         (uint64_t)st->h2 * st->s4 +
         (uint64_t)st->h3 * st->s3 +
         (uint64_t)st->h4 * st->s2;
    d2 = (uint64_t)st->h0 * st->r2 +
         (uint64_t)st->h1 * st->r1 +
         (uint64_t)st->h2 * st->r0 +
         (uint64_t)st->h3 * st->s4 +
         (uint64_t)st->h4 * st->s3;
    d3 = (uint64_t)st->h0 * st->r3 +
         (uint64_t)st->h1 * st->r2 +
         (uint64_t)st->h2 * st->r1 +
         (uint64_t)st->h3 * st->r0 +
         (uint64_t)st->h4 * st->s4;
    d4 = (uint64_t)st->h0 * st->r4 +
         (uint64_t)st->h1 * st->r3 +
         (uint64_t)st->h2 * st->r2 +
         (uint64_t)st->h3 * st->r1 +
         (uint64_t)st->h4 * st->r0;

    c = (uint32_t)(d0 >> 26);
    st->h0 = (uint32_t)d0 & 0x3ffffffU;
    d1 += c;
    c = (uint32_t)(d1 >> 26);
    st->h1 = (uint32_t)d1 & 0x3ffffffU;
    d2 += c;
    c = (uint32_t)(d2 >> 26);
    st->h2 = (uint32_t)d2 & 0x3ffffffU;
    d3 += c;
    c = (uint32_t)(d3 >> 26);
    st->h3 = (uint32_t)d3 & 0x3ffffffU;
    d4 += c;
    c = (uint32_t)(d4 >> 26);
    st->h4 = (uint32_t)d4 & 0x3ffffffU;

    st->h0 += c * 5U;
    c = st->h0 >> 26;
    st->h0 &= 0x3ffffffU;
    st->h1 += c;
}

static void stcp_soft_poly1305_update_padded(struct stcp_soft_poly1305 *st,
                                             const uint8_t *data,
                                             size_t len)
{
    uint8_t block[16];

    while (len >= 16U) {
        stcp_soft_poly1305_block(st, data);
        data += 16U;
        len -= 16U;
    }

    if (len != 0U) {
        memset(block, 0, sizeof(block));
        memcpy(block, data, len);
        stcp_soft_poly1305_block(st, block);
        memset(block, 0, sizeof(block));
    }
}

static void stcp_soft_poly1305_finish(struct stcp_soft_poly1305 *st,
                                      uint8_t mac[16])
{
    uint32_t c, g0, g1, g2, g3, g4, mask;
    uint64_t f0, f1, f2, f3;

    c = st->h1 >> 26;
    st->h1 &= 0x3ffffffU;
    st->h2 += c;
    c = st->h2 >> 26;
    st->h2 &= 0x3ffffffU;
    st->h3 += c;
    c = st->h3 >> 26;
    st->h3 &= 0x3ffffffU;
    st->h4 += c;
    c = st->h4 >> 26;
    st->h4 &= 0x3ffffffU;
    st->h0 += c * 5U;
    c = st->h0 >> 26;
    st->h0 &= 0x3ffffffU;
    st->h1 += c;

    g0 = st->h0 + 5U;
    c = g0 >> 26;
    g0 &= 0x3ffffffU;
    g1 = st->h1 + c;
    c = g1 >> 26;
    g1 &= 0x3ffffffU;
    g2 = st->h2 + c;
    c = g2 >> 26;
    g2 &= 0x3ffffffU;
    g3 = st->h3 + c;
    c = g3 >> 26;
    g3 &= 0x3ffffffU;
    g4 = st->h4 + c - (1U << 26);

    mask = (g4 >> 31) - 1U;
    g0 &= mask;
    g1 &= mask;
    g2 &= mask;
    g3 &= mask;
    g4 &= mask;
    mask = ~mask;

    st->h0 = (st->h0 & mask) | g0;
    st->h1 = (st->h1 & mask) | g1;
    st->h2 = (st->h2 & mask) | g2;
    st->h3 = (st->h3 & mask) | g3;
    st->h4 = (st->h4 & mask) | g4;

    f0 = (uint32_t)(st->h0 | (st->h1 << 26));
    f1 = (uint32_t)((st->h1 >> 6) | (st->h2 << 20));
    f2 = (uint32_t)((st->h2 >> 12) | (st->h3 << 14));
    f3 = (uint32_t)((st->h3 >> 18) | (st->h4 << 8));

    f0 += st->pad0;
    f1 += st->pad1 + (f0 >> 32);
    f0 &= 0xffffffffU;
    f2 += st->pad2 + (f1 >> 32);
    f1 &= 0xffffffffU;
    f3 += st->pad3 + (f2 >> 32);
    f2 &= 0xffffffffU;

    stcp_soft_store32_le(mac, (uint32_t)f0);
    stcp_soft_store32_le(mac + 4, (uint32_t)f1);
    stcp_soft_store32_le(mac + 8, (uint32_t)f2);
    stcp_soft_store32_le(mac + 12, (uint32_t)f3);

    memset(st, 0, sizeof(*st));
}

static void stcp_soft_store64_le(uint8_t *p, uint64_t v)
{
    unsigned int i;

    for (i = 0; i < 8U; i++) {
        p[i] = (uint8_t)v;
        v >>= 8;
    }
}

static void stcp_soft_chachapoly_tag(const uint8_t key[32],
                                     const uint8_t nonce[12],
                                     const uint8_t *aad,
                                     size_t aad_len,
                                     const uint8_t *cipher,
                                     size_t cipher_len,
                                     uint8_t tag[16])
{
    uint8_t block0[64];
    uint8_t lens[16];
    struct stcp_soft_poly1305 st;

    stcp_soft_chacha20_block(key, 0U, nonce, block0);
    stcp_soft_poly1305_init(&st, block0);

    stcp_soft_poly1305_update_padded(&st, aad, aad_len);
    stcp_soft_poly1305_update_padded(&st, cipher, cipher_len);

    stcp_soft_store64_le(lens, (uint64_t)aad_len);
    stcp_soft_store64_le(lens + 8, (uint64_t)cipher_len);
    stcp_soft_poly1305_block(&st, lens);

    stcp_soft_poly1305_finish(&st, tag);

    memset(block0, 0, sizeof(block0));
    memset(lens, 0, sizeof(lens));
}

static int stcp_soft_ct_equal16(const uint8_t a[16], const uint8_t b[16])
{
    uint8_t diff = 0U;
    unsigned int i;

    for (i = 0; i < 16U; i++) {
        diff |= (uint8_t)(a[i] ^ b[i]);
    }
    return diff == 0U;
}

static int stcp_soft_chachapoly_encrypt(const uint8_t key[32],
                                        const uint8_t nonce[12],
                                        const uint8_t *aad,
                                        size_t aad_len,
                                        const uint8_t *plain,
                                        size_t plain_len,
                                        uint8_t *out)
{
    uint8_t tag[16];

    stcp_soft_chacha20_xor(key, nonce, 1U, plain, out, plain_len);
    stcp_soft_chachapoly_tag(key, nonce, aad, aad_len, out, plain_len, tag);
    memcpy(out + plain_len, tag, sizeof(tag));
    memset(tag, 0, sizeof(tag));
    return 0;
}

static int stcp_soft_chachapoly_decrypt(const uint8_t key[32],
                                        const uint8_t nonce[12],
                                        const uint8_t *aad,
                                        size_t aad_len,
                                        const uint8_t *cipher,
                                        size_t cipher_len,
                                        uint8_t *out)
{
    size_t plain_len = cipher_len - 16U;
    uint8_t tag[16];

    stcp_soft_chachapoly_tag(key, nonce, aad, aad_len,
                            cipher, plain_len, tag);

    if (!stcp_soft_ct_equal16(tag, cipher + plain_len)) {
        memset(tag, 0, sizeof(tag));
        return -EBADMSG;
    }

    stcp_soft_chacha20_xor(key, nonce, 1U, cipher, out, plain_len);
    memset(tag, 0, sizeof(tag));
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
                        "STCPv2 wire TX");
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
