#include <stcp/stcp_buffer_pool.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stcp_gcm_diag, LOG_LEVEL_ERR);
/* STCP_GCM_DIAGNOSTIC_FIX: failure-only logs; never log keys or plaintext. */
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <psa/crypto.h>

#define STCP_GCM_TAG_LEN 16U

static void stcp_gcm_nonce(uint64_t nonce, uint8_t iv[12])
{
    memset(iv, 0, 4);
    for (unsigned i = 0; i < 8; i++)
        iv[4 + i] = (uint8_t)(nonce >> (56 - 8 * i));
}

static int stcp_gcm_error(psa_status_t status)
{
    if (status == PSA_ERROR_INVALID_SIGNATURE)
        return -EBADMSG;
    if (status == PSA_ERROR_NOT_SUPPORTED)
        return -ENOTSUP;
    if (status == PSA_ERROR_INSUFFICIENT_MEMORY)
        return -ENOMEM;
    return -EIO;
}

static int stcp_gcm_key_import(const uint8_t *key,
                               psa_key_usage_t usage,
                               psa_key_id_t *id)
{
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_type(&attr, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, usage);
    psa_set_key_algorithm(&attr, PSA_ALG_GCM);

    psa_status_t status = psa_import_key(&attr, key, 32, id);
    psa_reset_key_attributes(&attr);

    if (status != PSA_SUCCESS)
        LOG_ERR("STCP_GCM_IMPORT_FAIL status=%d usage=%u",
                (int)status, (unsigned)usage);
    return status == PSA_SUCCESS ? 0 : stcp_gcm_error(status);
}

int stcp_kernel_aes256_gcm_encrypt(
    const uint8_t *key, uint64_t nonce,
    const uint8_t *aad, size_t aad_len,
    const uint8_t *plain, size_t plain_len,
    uint8_t *out, size_t out_len)
{
    if (!key || !out || (plain_len && !plain) ||
        (aad_len && !aad))
        return -EINVAL;

    if (plain_len > SIZE_MAX - STCP_GCM_TAG_LEN ||
        out_len < plain_len + STCP_GCM_TAG_LEN)
        return -ENOSPC;

    uint8_t iv[12];
    psa_key_id_t id = 0;
    size_t written = 0;

    stcp_gcm_nonce(nonce, iv);

    int rc = stcp_gcm_key_import(key, PSA_KEY_USAGE_ENCRYPT, &id);
    if (rc)
        return rc;

    psa_status_t status = psa_aead_encrypt(
        id, PSA_ALG_GCM,
        iv, sizeof(iv),
        aad, aad_len,
        plain, plain_len,
        out, out_len,
        &written);

    (void)psa_destroy_key(id);

    if (status != PSA_SUCCESS || written != plain_len + STCP_GCM_TAG_LEN) {
        LOG_ERR("STCP_GCM_ENCRYPT_FAIL status=%d plain=%zu aad=%zu out=%zu written=%zu",
                (int)status, plain_len, aad_len, out_len, written);
        memset(out, 0, plain_len + STCP_GCM_TAG_LEN);
        return status == PSA_SUCCESS ? -EIO : stcp_gcm_error(status);
    }

    return 0;
}

int stcp_kernel_aes256_gcm_decrypt(
    const uint8_t *key, uint64_t nonce,
    const uint8_t *aad, size_t aad_len,
    const uint8_t *cipher, size_t cipher_len,
    uint8_t *out, size_t out_len)
{
    if (!key || !cipher || !out || (aad_len && !aad))
        return -EINVAL;

    if (cipher_len < STCP_GCM_TAG_LEN)
        return -EBADMSG;

    size_t plain_len = cipher_len - STCP_GCM_TAG_LEN;

    if (out_len < plain_len)
        return -ENOSPC;

    uint8_t iv[12];
    psa_key_id_t id = 0;
    size_t written = 0;

    stcp_gcm_nonce(nonce, iv);

    int rc = stcp_gcm_key_import(key, PSA_KEY_USAGE_DECRYPT, &id);
    if (rc)
        return rc;

    psa_status_t status = psa_aead_decrypt(
        id, PSA_ALG_GCM,
        iv, sizeof(iv),
        aad, aad_len,
        cipher, cipher_len,
        out, out_len,
        &written);

    (void)psa_destroy_key(id);

    if (status != PSA_SUCCESS || written != plain_len) {
        memset(out, 0, plain_len);
        return status == PSA_SUCCESS ? -EIO : stcp_gcm_error(status);
    }

    return 0;
}

int stcp_kernel_aes256_gcm_decrypt_in_place(
    const uint8_t *key, uint64_t nonce,
    const uint8_t *aad, size_t aad_len,
    uint8_t *cipher, size_t cipher_len)
{
    if (!cipher || cipher_len < STCP_GCM_TAG_LEN)
        return -EINVAL;

    size_t plain_len = cipher_len - STCP_GCM_TAG_LEN;

    uint8_t *tmp = stcp_buffer_alloc(plain_len ? plain_len : 1);
    if (!tmp)
        return -ENOMEM;

    int rc = stcp_kernel_aes256_gcm_decrypt(
        key, nonce, aad, aad_len,
        cipher, cipher_len, tmp, plain_len);

    if (rc == 0)
        memcpy(cipher, tmp, plain_len);
    else
        memset(cipher, 0, plain_len);

    memset(tmp, 0, plain_len);
    (void)stcp_buffer_free(tmp);

    return rc;
}
