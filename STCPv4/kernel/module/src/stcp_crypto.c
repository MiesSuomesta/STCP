// SPDX-License-Identifier: GPL-2.0

#include <crypto/chacha20poly1305.h>
#if __has_include(<crypto/aes-gcm.h>)
#include <crypto/aes-gcm.h>
#define STCP_NEW_AES_GCM 1
#else
#include <crypto/gcm.h>
#define STCP_NEW_AES_GCM 0
#endif
#include <crypto/curve25519.h>
#include <crypto/hash.h>

#include <linux/errno.h>
#include <linux/limits.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "stcp_crypto.h"
#include "stcp_aes256_gcm.h"


#define STCP_KDF_SHA256_LEN 32

static int stcp_kernel_sha256_3(const u8 *a, size_t a_len,
                                const u8 *b, size_t b_len,
                                const u8 *c, size_t c_len,
                                u8 out[STCP_KDF_SHA256_LEN])
{
	struct crypto_shash *tfm;
	struct shash_desc *desc;
	size_t desc_len;
	int rc;

	tfm = crypto_alloc_shash("sha256", 0, 0);
	if (IS_ERR(tfm))
		return PTR_ERR(tfm);

	desc_len = sizeof(*desc) + crypto_shash_descsize(tfm);
	desc = kzalloc(desc_len, GFP_KERNEL);
	if (!desc) {
		crypto_free_shash(tfm);
		return -ENOMEM;
	}

	desc->tfm = tfm;

	rc = crypto_shash_init(desc);
	if (!rc && a_len)
		rc = crypto_shash_update(desc, a, a_len);
	if (!rc && b_len)
		rc = crypto_shash_update(desc, b, b_len);
	if (!rc && c_len)
		rc = crypto_shash_update(desc, c, c_len);
	if (!rc)
		rc = crypto_shash_final(desc, out);

	memzero_explicit(desc, desc_len);
	kfree(desc);
	crypto_free_shash(tfm);
	return rc;
}

static int stcp_kernel_hmac_sha256_2(const u8 *key, size_t key_len,
                                     const u8 *a, size_t a_len,
                                     const u8 *b, size_t b_len,
                                     u8 out[STCP_KDF_SHA256_LEN])
{
	struct crypto_shash *tfm;
	struct shash_desc *desc;
	size_t desc_len;
	int rc;

	tfm = crypto_alloc_shash("hmac(sha256)", 0, 0);
	if (IS_ERR(tfm))
		return PTR_ERR(tfm);

	rc = crypto_shash_setkey(tfm, key, key_len);
	if (rc) {
		crypto_free_shash(tfm);
		return rc;
	}

	desc_len = sizeof(*desc) + crypto_shash_descsize(tfm);
	desc = kzalloc(desc_len, GFP_KERNEL);
	if (!desc) {
		crypto_free_shash(tfm);
		return -ENOMEM;
	}

	desc->tfm = tfm;

	rc = crypto_shash_init(desc);
	if (!rc && a_len)
		rc = crypto_shash_update(desc, a, a_len);
	if (!rc && b_len)
		rc = crypto_shash_update(desc, b, b_len);
	if (!rc)
		rc = crypto_shash_final(desc, out);

	memzero_explicit(desc, desc_len);
	kfree(desc);
	crypto_free_shash(tfm);
	return rc;
}

int stcp_kernel_derive_session_keys(
	const u8 shared[STCP_CURVE25519_KEY_LEN],
	const u8 client_public_key[STCP_CURVE25519_KEY_LEN],
	const u8 server_public_key[STCP_CURVE25519_KEY_LEN],
	u8 client_to_server[STCP_AEAD_KEY_LEN],
	u8 server_to_client[STCP_AEAD_KEY_LEN]
)
{
	static const u8 domain[] = "STCPv4-AES256-GCM-HKDF-SHA256";
	static const u8 client_label[] = "STCPv4 AES256-GCM client to server key";
	static const u8 server_label[] = "STCPv4 AES256-GCM server to client key";
	static const u8 counter = 1;
	u8 salt[STCP_KDF_SHA256_LEN];
	u8 prk[STCP_KDF_SHA256_LEN];
	int rc;

	if (!shared || !client_public_key || !server_public_key ||
	    !client_to_server || !server_to_client)
		return -EINVAL;

	rc = stcp_kernel_sha256_3(domain, sizeof(domain) - 1,
				  client_public_key, STCP_CURVE25519_KEY_LEN,
				  server_public_key, STCP_CURVE25519_KEY_LEN,
				  salt);
	if (rc)
		goto out;

	rc = stcp_kernel_hmac_sha256_2(salt, sizeof(salt),
				       shared, STCP_CURVE25519_KEY_LEN,
				       NULL, 0, prk);
	if (rc)
		goto out;

	rc = stcp_kernel_hmac_sha256_2(prk, sizeof(prk),
				       client_label, sizeof(client_label) - 1,
				       &counter, sizeof(counter),
				       client_to_server);
	if (rc)
		goto out;

	rc = stcp_kernel_hmac_sha256_2(prk, sizeof(prk),
				       server_label, sizeof(server_label) - 1,
				       &counter, sizeof(counter),
				       server_to_client);
	if (rc)
		goto out;

	if (!memcmp(client_to_server, server_to_client, STCP_AEAD_KEY_LEN)) {
		rc = -EKEYREJECTED;
		goto out;
	}

out:
	if (rc) {
		memzero_explicit(client_to_server, STCP_AEAD_KEY_LEN);
		memzero_explicit(server_to_client, STCP_AEAD_KEY_LEN);
	}
	memzero_explicit(salt, sizeof(salt));
	memzero_explicit(prk, sizeof(prk));
	return rc;
}

int stcp_kernel_x25519_keypair(
	u8 private_key[STCP_CURVE25519_KEY_LEN],
	u8 public_key[STCP_CURVE25519_KEY_LEN]
)
{
	if (!private_key || !public_key)
		return -EINVAL;

	curve25519_generate_secret(private_key);

	if (!curve25519_generate_public(public_key, private_key)) {
		memzero_explicit(private_key, STCP_CURVE25519_KEY_LEN);
		memzero_explicit(public_key, STCP_CURVE25519_KEY_LEN);
		return -EKEYREJECTED;
	}

	return 0;
}

int stcp_kernel_x25519_shared(
	u8 shared_key[STCP_CURVE25519_KEY_LEN],
	const u8 private_key[STCP_CURVE25519_KEY_LEN],
	const u8 peer_public_key[STCP_CURVE25519_KEY_LEN]
)
{
	if (!shared_key || !private_key || !peer_public_key)
		return -EINVAL;

	if (!curve25519(shared_key, private_key, peer_public_key)) {
		memzero_explicit(shared_key, STCP_CURVE25519_KEY_LEN);
		return -EKEYREJECTED;
	}

	return 0;
}

int stcp_p2p_noise_chacha_encrypt(
	const u8 key[STCP_AEAD_KEY_LEN],
	u64 nonce,
	const u8 *associated_data,
	size_t associated_data_len,
	const u8 *plaintext,
	size_t plaintext_len,
	u8 *ciphertext_and_tag,
	size_t ciphertext_capacity
)
{
	if (!key || !ciphertext_and_tag)
		return -EINVAL;
	if (plaintext_len && !plaintext)
		return -EINVAL;
	if (associated_data_len && !associated_data)
		return -EINVAL;
	if (plaintext_len > SIZE_MAX - STCP_AEAD_TAG_LEN)
		return -EOVERFLOW;
	if (ciphertext_capacity < plaintext_len + STCP_AEAD_TAG_LEN)
		return -ENOSPC;

	chacha20poly1305_encrypt(
		ciphertext_and_tag,
		plaintext,
		plaintext_len,
		associated_data,
		associated_data_len,
		nonce,
		key
	);
	return 0;
}

int stcp_p2p_noise_chacha_decrypt(
	const u8 key[STCP_AEAD_KEY_LEN],
	u64 nonce,
	const u8 *associated_data,
	size_t associated_data_len,
	const u8 *ciphertext_and_tag,
	size_t ciphertext_and_tag_len,
	u8 *plaintext,
	size_t plaintext_capacity
)
{
	size_t plaintext_len;

	if (!key || !ciphertext_and_tag || !plaintext)
		return -EINVAL;
	if (associated_data_len && !associated_data)
		return -EINVAL;
	if (ciphertext_and_tag_len < STCP_AEAD_TAG_LEN)
		return -EBADMSG;

	plaintext_len = ciphertext_and_tag_len - STCP_AEAD_TAG_LEN;
	if (plaintext_capacity < plaintext_len)
		return -ENOSPC;

	if (!chacha20poly1305_decrypt(
			plaintext,
			ciphertext_and_tag,
			ciphertext_and_tag_len,
			associated_data,
			associated_data_len,
			nonce,
			key)) {
		if (plaintext_len)
			memzero_explicit(plaintext, plaintext_len);
		return -EBADMSG;
	}

	return 0;
}


/* STCP transport uses AES-256-GCM.  The 8-byte wire nonce is extended to
 * the 12-byte GCM nonce with a fixed zero prefix; direction-separated keys
 * make (key, nonce) pairs unique across the two traffic directions. */
#if STCP_NEW_AES_GCM
#define stcp_gcm_key aes_gcm_key
#define stcp_gcm_prepare aes_gcm_preparekey
#else
#define stcp_gcm_key aesgcm_ctx
#define stcp_gcm_prepare aesgcm_expandkey
#endif

static void stcp_aes_gcm_nonce(u8 iv[12], u64 nonce)
{
	unsigned int i;

	memset(iv, 0, 4);
	for (i = 0; i < 8; i++)
		iv[4 + i] = (u8)(nonce >> (56 - 8 * i));
}

int stcp_kernel_aes256_gcm_encrypt(
	const u8 key[STCP_AEAD_KEY_LEN], u64 nonce,
	const u8 *aad, size_t aad_len, const u8 *plain, size_t plain_len,
	u8 *out, size_t out_len)
{
	struct stcp_gcm_key prepared;
	u8 iv[12];
	int rc;

	if (!key || !out || (plain_len && !plain) || (aad_len && !aad))
		return -EINVAL;
	if (plain_len > SIZE_MAX - STCP_AEAD_TAG_LEN)
		return -EOVERFLOW;
	if (out_len < plain_len + STCP_AEAD_TAG_LEN)
		return -ENOSPC;
#if !STCP_NEW_AES_GCM
	if (plain_len > INT_MAX || aad_len > INT_MAX)
		return -EOVERFLOW;
#endif
	rc = stcp_gcm_prepare(&prepared, key, 32, STCP_AEAD_TAG_LEN);
	if (rc)
		return rc;
	stcp_aes_gcm_nonce(iv, nonce);
#if STCP_NEW_AES_GCM
	aes_gcm_encrypt(out, plain, plain_len, out + plain_len,
			 aad, aad_len, iv, &prepared);
#else
	aesgcm_encrypt(&prepared, out, plain, plain_len, aad, aad_len,
			iv, out + plain_len);
#endif
	memzero_explicit(&prepared, sizeof(prepared));
	return 0;
}

int stcp_kernel_aes256_gcm_decrypt(
	const u8 key[STCP_AEAD_KEY_LEN], u64 nonce,
	const u8 *aad, size_t aad_len, const u8 *cipher, size_t cipher_len,
	u8 *out, size_t out_len)
{
	struct stcp_gcm_key prepared;
	size_t plain_len;
	u8 iv[12];
	int rc;

	if (!key || !cipher || !out || (aad_len && !aad))
		return -EINVAL;
	if (cipher_len < STCP_AEAD_TAG_LEN)
		return -EBADMSG;
	plain_len = cipher_len - STCP_AEAD_TAG_LEN;
	if (out_len < plain_len)
		return -ENOSPC;
#if !STCP_NEW_AES_GCM
	if (plain_len > INT_MAX || aad_len > INT_MAX)
		return -EOVERFLOW;
#endif
	rc = stcp_gcm_prepare(&prepared, key, 32, STCP_AEAD_TAG_LEN);
	if (rc)
		return rc;
	stcp_aes_gcm_nonce(iv, nonce);
#if STCP_NEW_AES_GCM
	rc = aes_gcm_decrypt(out, cipher, plain_len, cipher + plain_len,
			     aad, aad_len, iv, &prepared);
#else
	rc = aesgcm_decrypt(&prepared, out, cipher, plain_len, aad, aad_len,
			    iv, cipher + plain_len) ? 0 : -EBADMSG;
#endif
	memzero_explicit(&prepared, sizeof(prepared));
	if (rc && plain_len)
		memzero_explicit(out, plain_len);
	return rc;
}

int stcp_kernel_aes256_gcm_decrypt_in_place(
	const u8 key[STCP_AEAD_KEY_LEN], u64 nonce,
	const u8 *aad, size_t aad_len, u8 *cipher, size_t cipher_len)
{
	if (cipher_len < STCP_AEAD_TAG_LEN)
		return -EBADMSG;
	return stcp_kernel_aes256_gcm_decrypt(key, nonce, aad, aad_len,
			cipher, cipher_len, cipher,
			cipher_len - STCP_AEAD_TAG_LEN);
}
