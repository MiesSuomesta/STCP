// SPDX-License-Identifier: GPL-2.0

#include <crypto/chacha20poly1305.h>
#include <crypto/curve25519.h>

#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>

#include "stcp_crypto.h"

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

int stcp_kernel_chacha_encrypt(
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

int stcp_kernel_chacha_decrypt(
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

int stcp_kernel_chacha_decrypt_in_place(
	const u8 key[STCP_AEAD_KEY_LEN],
	u64 nonce,
	const u8 *associated_data,
	size_t associated_data_len,
	u8 *ciphertext_and_tag,
	size_t ciphertext_and_tag_len
)
{
	size_t plaintext_len;

	if (!key || !ciphertext_and_tag)
		return -EINVAL;
	if (associated_data_len && !associated_data)
		return -EINVAL;
	if (ciphertext_and_tag_len < STCP_AEAD_TAG_LEN)
		return -EBADMSG;

	plaintext_len = ciphertext_and_tag_len - STCP_AEAD_TAG_LEN;

	/* The kernel ChaCha20-Poly1305 helper supports dst == src.  Keeping
	 * ciphertext and plaintext in the same owned frame buffer removes one
	 * multi-megabyte allocation and one full payload copy per RX frame. */
	if (!chacha20poly1305_decrypt(
		ciphertext_and_tag,
		ciphertext_and_tag,
		ciphertext_and_tag_len,
		associated_data,
		associated_data_len,
		nonce,
		key)) {
		if (plaintext_len)
			memzero_explicit(ciphertext_and_tag, plaintext_len);
		return -EBADMSG;
	}

	return 0;
}
