#include <stcp/stcp_buffer_pool.h>
#include <errno.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>
#include <psa/crypto.h>
#include <stcp/stcp_internal.h>
#include <stcp/stcp_rust_ffi.h>
#include <stcp/stcp_debug.h>

#include <stcp/stcp_x25519_soft.h>
#include "stcp_chachapoly_soft.h"

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
        STCP_CRYPTO_INF("STCP crypto init: status=%d (%s)", (int)status,
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

/* Diagnostic access to Zephyr's system k_malloc heap. Snapshot under its
 * lock; release before logging to avoid logger/allocator lock inversion. */
extern struct k_heap _system_heap;
void stcp_memory_snapshot(const char *stage, size_t requested)
{
#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
    if (requested == 0) {
        LOG_ERR("STCP_HEAP alloc_loops=%u", (unsigned)CONFIG_SYS_HEAP_ALLOC_LOOPS);
        stcp_buffer_pool_report();
    }
    struct sys_memory_stats stats;
    k_spinlock_key_t key = k_spin_lock(&_system_heap.lock);
    int rc = sys_heap_runtime_stats_get(&_system_heap.heap, &stats);
    k_spin_unlock(&_system_heap.lock, key);
    if (rc == 0) {
        LOG_ERR("STCP_HEAP stage=%s requested=%u used=%u free=%u peak=%u",
                stage, (unsigned)requested, (unsigned)stats.allocated_bytes,
                (unsigned)stats.free_bytes, (unsigned)stats.max_allocated_bytes);
    } else {
        LOG_ERR("STCP_HEAP stage=%s stats_rc=%d", stage, rc);
    }
#else
    LOG_ERR("STCP_HEAP stats disabled");
#endif
}

/* One diagnostic dump per boot. Direct printk is required while the
 * allocator lock is held; CONFIG_LOG_PRINTK must be disabled. */
#if defined(CONFIG_SYS_HEAP_INFO) && defined(CONFIG_SYS_HEAP_VALIDATE) && !defined(CONFIG_LOG_PRINTK)
static atomic_t heap_dump_done;
static void stcp_heap_failure_dump(size_t requested)
{
    if (!atomic_cas(&heap_dump_done, 0, 1)) return;
    printk("STCP_HEAP_DUMP_BEGIN requested=%u alloc_loops=%u\n",
           (unsigned)requested, (unsigned)CONFIG_SYS_HEAP_ALLOC_LOOPS);
    k_spinlock_key_t key = k_spin_lock(&_system_heap.lock);
    bool valid = sys_heap_validate(&_system_heap.heap);
    if (valid) sys_heap_print_info(&_system_heap.heap, true);
    k_spin_unlock(&_system_heap.lock, key);
    printk("STCP_HEAP_DUMP_END valid=%u\n", (unsigned)valid);
}
#else
static void stcp_heap_failure_dump(size_t requested)
{
    LOG_ERR("STCP_HEAP_DUMP disabled requested=%u", (unsigned)requested);
}
#endif

void *stcp_rust_kernel_alloc(size_t size)
{
    /* Small Rust metadata/control buffers retain the system allocator.
     * Frame-sized buffers never fall back to the fragmented heap. */
    void *ptr = size > 512U ? stcp_buffer_alloc(size) : k_malloc(size ? size : 1U);
    if (ptr == NULL) {
        stcp_memory_snapshot("alloc-failed", size);
        stcp_buffer_pool_report();
        if (size <= 512U) stcp_heap_failure_dump(size);
        LOG_ERR("STCP_MEM alloc_failed bytes=%u", (unsigned)size);
    }
    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_ALLOC)) {
        STCP_CRYPTO_DBG("rust alloc size=%u ptr=%p", (unsigned)size, ptr);
    }
    return ptr;
}

void stcp_rust_kernel_free(void *ptr)
{
    if (ptr != NULL) {
        if (!stcp_buffer_free(ptr)) k_free(ptr);
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
        /* Latch the wake until recv() confirms that the application buffer is
         * empty.  A binary semaphore alone can be consumed by another waiter,
         * which used to lose POLLIN between Rust RX and the MQTT worker. */
        ctx->poll_read_wakeup = true;
        k_sem_give(&ctx->rust_event);
    }
}

void stcp_kernel_debug_event(uint32_t event, uintptr_t ctx,
                             uintptr_t arg0, uintptr_t arg1)
{
    if (event == 920) {
        printk("STCP_SEND_STAGE_FAIL stage=%u errno=%u ctx=%p\n",
               (unsigned)arg0, (unsigned)arg1, (void *)ctx);
        return;
    }
    if (event >= 900 && event <= 904) {
        LOG_ERR("STCP_MEM event=%u ctx=%p a=%u b=%u", event,
                (void *)ctx, (unsigned)arg0, (unsigned)arg1);
        return;
    }
    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_EVENTS)) {
        STCP_CRYPTO_DBG("rust event=%u ctx=%p arg0=%u arg1=%u", event,
                (void *)ctx, (unsigned)arg0, (unsigned)arg1);
    }
}


static void log_crypto_stack(const char *where)
{
#if defined(CONFIG_THREAD_STACK_INFO)
    size_t unused = 0;
    int rc = k_thread_stack_space_get(k_current_get(), &unused);

    if (rc == 0) {
        STCP_CRYPTO_INF("crypto stack %s: unused=%zu", where, unused);
    } else {
        STCP_CRYPTO_WRN("crypto stack %s: measurement failed rc=%d", where, rc);
    }
#else
    ARG_UNUSED(where);
#endif
}

static void nonce_to_bytes(uint64_t nonce, uint8_t out[12])
{
    memset(out, 0, 12);
    sys_put_be64(nonce, &out[4]);
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

    STCP_CRYPTO_INF("X25519 keypair start: backend=software-rfc7748 rng=PSA");
    status = psa_generate_random(secret, 32);
    STCP_CRYPTO_INF("X25519 software psa_generate_random: status=%d (%s)",
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
        STCP_CRYPTO_HEXDUMP(public_key, 32, "X25519 software public key");
    }
    STCP_CRYPTO_INF("X25519 backend selected: software-rfc7748");
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

    /* Keep the critical X25519 path free of deferred logging and heavy
     * formatter arguments. These fixed markers are intentionally tiny. */
    STCP_CRYPTO_PRINTK("X25-P1\n");
    rc = stcp_x25519_soft(shared, secret, peer);
    STCP_CRYPTO_PRINTK("X25-P2\n");
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
    STCP_CRYPTO_PRINTK("X25-P3\n");
    return 0;
}

#define STCP_PSA_AEAD_CHUNK_SIZE 1024U

static psa_status_t stcp_psa_chacha_encrypt_stream(
    psa_key_id_t key_id,
    const uint8_t nonce_bytes[12],
    const uint8_t *aad, size_t aad_len,
    const uint8_t *plain, size_t plain_len,
    uint8_t *out, size_t out_len,
    size_t *written)
{
    psa_aead_operation_t operation = PSA_AEAD_OPERATION_INIT;
    uint8_t tag[16];
    size_t in_off = 0;
    size_t out_off = 0;
    size_t produced = 0;
    size_t tag_len = 0;
    psa_status_t status;

    *written = 0;
    STCP_CRYPTO_PRINTK("CH-STR-E1\n");

    status = psa_aead_encrypt_setup(&operation, key_id,
                                    PSA_ALG_CHACHA20_POLY1305);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-ES %d\n", (int)status);
        goto out_abort;
    }

    status = psa_aead_set_lengths(&operation, aad_len, plain_len);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-EL %d\n", (int)status);
        goto out_abort;
    }

    status = psa_aead_set_nonce(&operation, nonce_bytes, 12);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-EN %d\n", (int)status);
        goto out_abort;
    }

    if (aad_len != 0U) {
        status = psa_aead_update_ad(&operation, aad, aad_len);
        if (status != PSA_SUCCESS) {
            STCP_CRYPTO_PRINTK("CH-STR-EA %d\n", (int)status);
            goto out_abort;
        }
    }

    STCP_CRYPTO_PRINTK("CH-STR-E2\n");
    while (in_off < plain_len) {
        size_t part = plain_len - in_off;

        if (part > STCP_PSA_AEAD_CHUNK_SIZE) {
            part = STCP_PSA_AEAD_CHUNK_SIZE;
        }
        if (out_off >= out_len) {
            status = PSA_ERROR_BUFFER_TOO_SMALL;
            goto out_abort;
        }

        produced = 0;
        status = psa_aead_update(&operation,
                                 plain + in_off, part,
                                 out + out_off, out_len - out_off,
                                 &produced);
        if (status != PSA_SUCCESS) {
            STCP_CRYPTO_PRINTK("CH-STR-EU %d I=%u O=%u\n", (int)status,
                   (unsigned)in_off, (unsigned)out_off);
            goto out_abort;
        }
        in_off += part;
        out_off += produced;
    }

    produced = 0;
    status = psa_aead_finish(&operation,
                             out + out_off, out_len - out_off,
                             &produced,
                             tag, sizeof(tag), &tag_len);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-EF %d\n", (int)status);
        goto out_abort;
    }
    out_off += produced;

    if (tag_len != sizeof(tag) || out_len - out_off < tag_len) {
        status = PSA_ERROR_BUFFER_TOO_SMALL;
        goto out_abort;
    }
    memcpy(out + out_off, tag, tag_len);
    out_off += tag_len;
    *written = out_off;
    memset(tag, 0, sizeof(tag));
    STCP_CRYPTO_PRINTK("CH-STR-E3 %u\n", (unsigned)out_off);
    return PSA_SUCCESS;

out_abort:
    (void)psa_aead_abort(&operation);
    memset(tag, 0, sizeof(tag));
    return status;
}

static psa_status_t stcp_psa_chacha_decrypt_stream(
    psa_key_id_t key_id,
    const uint8_t nonce_bytes[12],
    const uint8_t *aad, size_t aad_len,
    const uint8_t *cipher, size_t cipher_len,
    uint8_t *out, size_t out_len,
    size_t *written)
{
    psa_aead_operation_t operation = PSA_AEAD_OPERATION_INIT;
    const size_t tag_len = 16U;
    const size_t body_len = cipher_len - tag_len;
    const uint8_t *tag = cipher + body_len;
    size_t in_off = 0;
    size_t out_off = 0;
    size_t produced = 0;
    psa_status_t status;

    *written = 0;
    STCP_CRYPTO_PRINTK("CH-STR-D1\n");

    status = psa_aead_decrypt_setup(&operation, key_id,
                                    PSA_ALG_CHACHA20_POLY1305);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-DS %d\n", (int)status);
        goto out_abort;
    }

    status = psa_aead_set_lengths(&operation, aad_len, body_len);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-DL %d\n", (int)status);
        goto out_abort;
    }

    status = psa_aead_set_nonce(&operation, nonce_bytes, 12);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-DN %d\n", (int)status);
        goto out_abort;
    }

    if (aad_len != 0U) {
        status = psa_aead_update_ad(&operation, aad, aad_len);
        if (status != PSA_SUCCESS) {
            STCP_CRYPTO_PRINTK("CH-STR-DA %d\n", (int)status);
            goto out_abort;
        }
    }

    STCP_CRYPTO_PRINTK("CH-STR-D2\n");
    while (in_off < body_len) {
        size_t part = body_len - in_off;

        if (part > STCP_PSA_AEAD_CHUNK_SIZE) {
            part = STCP_PSA_AEAD_CHUNK_SIZE;
        }
        if (out_off >= out_len && body_len != 0U) {
            status = PSA_ERROR_BUFFER_TOO_SMALL;
            goto out_abort;
        }

        produced = 0;
        status = psa_aead_update(&operation,
                                 cipher + in_off, part,
                                 out + out_off, out_len - out_off,
                                 &produced);
        if (status != PSA_SUCCESS) {
            STCP_CRYPTO_PRINTK("CH-STR-DU %d I=%u O=%u\n", (int)status,
                   (unsigned)in_off, (unsigned)out_off);
            goto out_abort;
        }
        in_off += part;
        out_off += produced;
    }

    produced = 0;
    status = psa_aead_verify(&operation,
                             out + out_off, out_len - out_off,
                             &produced,
                             tag, tag_len);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-STR-DV %d\n", (int)status);
        goto out_abort;
    }
    out_off += produced;
    *written = out_off;
    STCP_CRYPTO_PRINTK("CH-STR-D3 %u\n", (unsigned)out_off);
    return PSA_SUCCESS;

out_abort:
    (void)psa_aead_abort(&operation);
    return status;
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
    psa_status_t status = PSA_ERROR_GENERIC_ERROR;
    int rc = stcp_crypto_ensure_ready();
    if (rc != 0) return rc;
    if (!key || (!plain && plain_len != 0U) || (aad_len != 0U && !aad) || !out || plain_len > SIZE_MAX - 16U || out_len < plain_len + 16U)
        return -EINVAL;
    nonce_to_bytes(nonce, nonce_bytes);
    psa_set_key_type(&attr, PSA_KEY_TYPE_CHACHA20);
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attr, PSA_ALG_CHACHA20_POLY1305);
    STCP_CRYPTO_PRINTK("CH-ATTR T=%u B=%u U=%u A=%u\n",
           (unsigned)psa_get_key_type(&attr),
           (unsigned)psa_get_key_bits(&attr),
           (unsigned)psa_get_key_usage_flags(&attr),
           (unsigned)psa_get_key_algorithm(&attr));
    STCP_CRYPTO_PRINTK("CH-ENC1\n");
    status = psa_import_key(&attr, key, 32, &key_id);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-ENCI %d\n", (int)status);
    } else {
        STCP_CRYPTO_PRINTK("CH-ENC2\n");
        STCP_CRYPTO_PRINTK("CH-LEN P=%u A=%u N=%u O=%u E=%u C=%u\n",
               (unsigned)plain_len, (unsigned)aad_len,
               (unsigned)sizeof(nonce_bytes), (unsigned)out_len,
               (unsigned)(plain_len + 16U),
               (unsigned)STCP_PSA_AEAD_CHUNK_SIZE);
        status = stcp_psa_chacha_encrypt_stream(
            key_id, nonce_bytes, aad, aad_len,
            plain, plain_len, out, out_len, &written);
        if (status != PSA_SUCCESS) {
            STCP_CRYPTO_PRINTK("CH-ENCE %d W=%u\n", (int)status, (unsigned)written);
        } else {
            STCP_CRYPTO_PRINTK("CH-ENC3 %u\n", (unsigned)written);
        }
    }
    if (key_id != 0) (void)psa_destroy_key(key_id);
    psa_reset_key_attributes(&attr);
    if (status == PSA_ERROR_NOT_SUPPORTED) {
        /* PSA streaming unsupported: use the same RFC 8439 fallback as v2.
         * PSA operation was aborted and the key destroyed above. */
        LOG_INF("ChaCha20-Poly1305 stream unsupported; using RFC8439 software fallback");
        return stcp_soft_chachapoly_encrypt(key, nonce_bytes, aad, aad_len,
                                            plain, plain_len, out);
    }
    if (status != PSA_SUCCESS || written != plain_len + 16U) {
        LOG_ERR("ChaCha20-Poly1305 stream encrypt failed: status=%d (%s) written=%u expected=%u",
                (int)status, psa_status_name(status), (unsigned)written,
                (unsigned)(plain_len + 16U));
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
    psa_status_t status = PSA_ERROR_GENERIC_ERROR;
    int rc = stcp_crypto_ensure_ready();
    if (rc != 0) return rc;
    if (!key || !cipher || !out || (aad_len != 0U && !aad) || cipher_len < 16U || out_len < cipher_len - 16U)
        return -EINVAL;
    nonce_to_bytes(nonce, nonce_bytes);
    psa_set_key_type(&attr, PSA_KEY_TYPE_CHACHA20);
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attr, PSA_ALG_CHACHA20_POLY1305);
    STCP_CRYPTO_PRINTK("CH-DATTR T=%u B=%u U=%u A=%u\n",
           (unsigned)psa_get_key_type(&attr),
           (unsigned)psa_get_key_bits(&attr),
           (unsigned)psa_get_key_usage_flags(&attr),
           (unsigned)psa_get_key_algorithm(&attr));
    STCP_CRYPTO_PRINTK("CH-DEC1\n");
    status = psa_import_key(&attr, key, 32, &key_id);
    if (status != PSA_SUCCESS) {
        STCP_CRYPTO_PRINTK("CH-DECI %d\n", (int)status);
    } else {
        STCP_CRYPTO_PRINTK("CH-DEC2\n");
        STCP_CRYPTO_PRINTK("CH-DLEN C=%u A=%u N=%u O=%u E=%u K=%u\n",
               (unsigned)cipher_len, (unsigned)aad_len,
               (unsigned)sizeof(nonce_bytes), (unsigned)out_len,
               (unsigned)(cipher_len - 16U),
               (unsigned)STCP_PSA_AEAD_CHUNK_SIZE);
        status = stcp_psa_chacha_decrypt_stream(
            key_id, nonce_bytes, aad, aad_len,
            cipher, cipher_len, out, out_len, &written);
        if (status != PSA_SUCCESS) {
            STCP_CRYPTO_PRINTK("CH-DECE %d W=%u\n", (int)status, (unsigned)written);
        } else {
            STCP_CRYPTO_PRINTK("CH-DEC3 %u\n", (unsigned)written);
        }
    }
    if (key_id != 0) (void)psa_destroy_key(key_id);
    psa_reset_key_attributes(&attr);
    if (status == PSA_ERROR_NOT_SUPPORTED) {
        /* Never expose partial plaintext from a failed PSA attempt. */
        memset(out, 0, cipher_len - 16U);
        LOG_INF("ChaCha20-Poly1305 stream unsupported; using RFC8439 software fallback");
        return stcp_soft_chachapoly_decrypt(key, nonce_bytes, aad, aad_len,
                                            cipher, cipher_len, out);
    }
    if (status != PSA_SUCCESS || written != cipher_len - 16U) {
        memset(out, 0, cipher_len - 16U);
        LOG_ERR("ChaCha20-Poly1305 stream decrypt failed: status=%d (%s) written=%u expected=%u",
                (int)status, psa_status_name(status), (unsigned)written,
                (unsigned)(cipher_len - 16U));
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
    tmp = stcp_buffer_alloc(cipher_len - 16);
    if (!tmp) return -ENOMEM;
    rc = stcp_kernel_chacha_decrypt(key, nonce, aad, aad_len,
                                    cipher, cipher_len, tmp, cipher_len - 16);
    if (rc == 0) memcpy(cipher, tmp, cipher_len - 16);
    memset(tmp, 0, cipher_len - 16);
    (void)stcp_buffer_free(tmp);
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
    size_t done = 0;

    if (!ctx || ctx->carrier_fd < 0)
        return -ENOTCONN;

    if (!data && len)
        return -EINVAL;

    if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_WIRE)) {
        STCP_TX_HEXDUMP(data,
                        MIN(len, (size_t)CONFIG_STCP_RUST_HEXDUMP_BYTES),
                        "Rust STCP TX");
    }

    /* UDP_ATOMIC_SEND_FIX: never split one frame across datagrams. */
    if (ctx->socket_type == SOCK_DGRAM) {
        ssize_t rc;
        int saved_errno;
        do {
            errno = 0;
            rc = zsock_send(ctx->carrier_fd, data, len, flags);
            saved_errno = errno;
        } while (rc < 0 && saved_errno == EINTR);
        if (rc < 0) {
            LOG_ERR("STCP_UDP_TX_FAIL fd=%d len=%zu rc=%d errno=%d",
                    ctx->carrier_fd, len, (int)rc, saved_errno);
            return -(saved_errno ? saved_errno : EIO);
        }
        if ((size_t)rc != len) {
            LOG_ERR("STCP_UDP_TX_SHORT fd=%d len=%zu rc=%d",
                    ctx->carrier_fd, len, (int)rc);
            return -EIO;
        }
        return rc;
    }

    while (done < len) {
        ssize_t rc = zsock_send(ctx->carrier_fd,
                                data + done,
                                len - done,
                                flags);
        int saved_errno = rc < 0 ? errno : 0;

        if (rc > 0) {
            done += (size_t)rc;
            continue;
        }

        if (rc == 0)
            return -EPIPE;

        if (saved_errno == EINTR)
            continue;

        LOG_ERR("STCP_CARRIER_TX_FAIL fd=%d progress=%zu/%zu errno=%d",
                ctx->carrier_fd, done, len, saved_errno);

        return -saved_errno;
    }

    return (ssize_t)done;
}

// STCP_ZEPHYR_LOCK_WAIT_FIX
/* All shared Rust-core callers run in thread context. k_yield() is not
 * sufficient: a priority-5 waiter must allow a priority-7 owner to run.
 * Sleep only on contention; keep the uncontended atomic fast path intact.
 */
void stcp_rust_lock_wait(void)
{
    __ASSERT_NO_MSG(!k_is_in_isr());
    k_sleep(K_MSEC(1));
}
