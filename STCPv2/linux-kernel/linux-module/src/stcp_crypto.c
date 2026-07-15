// SPDX-License-Identifier: GPL-2.0

#include <crypto/chacha20poly1305.h>
#include <crypto/curve25519.h>

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>

#include "stcp_crypto.h"

/*
 * Kernel ChaCha20-Poly1305 appends a 16-byte Poly1305 tag directly
 * after the encrypted payload.
 */
#define STCP_CHACHA_TAG_LEN CHACHA20POLY1305_AUTHTAG_SIZE

int stcp_kernel_x25519_keypair(
	u8 private_key[STCP_CURVE25519_KEY_LEN],
	u8 public_key[STCP_CURVE25519_KEY_LEN]
)
{
	if (!private_key || !public_key)
		return -EINVAL;

	/*
	 * Generates 32 random bytes and applies the Curve25519 clamping
	 * required by RFC 7748.
	 */
	curve25519_generate_secret(private_key);

	if (!curve25519_generate_public(
		    public_key,
		    private_key
	    )) {
		memzero_explicit(
			private_key,
			STCP_CURVE25519_KEY_LEN
		);

		memzero_explicit(
			public_key,
			STCP_CURVE25519_KEY_LEN
		);

		return -EKEYREJECTED;
	}

	return 0;
}

int stcp_kernel_x25519_shared(
	const u8 private_key[STCP_CURVE25519_KEY_LEN],
	const u8 peer_public_key[STCP_CURVE25519_KEY_LEN],
	u8 shared_key[STCP_CURVE25519_KEY_LEN]
)
{
	if (!private_key ||
	    !peer_public_key ||
	    !shared_key)
		return -EINVAL;

	if (!curve25519(
		    shared_key,
		    private_key,
		    peer_public_key
	    )) {
		/*
		 * curve25519() returns false for an invalid peer public key
		 * which produces the all-zero shared secret.
		 */
		memzero_explicit(
			shared_key,
			STCP_CURVE25519_KEY_LEN
		);

		return -EKEYREJECTED;
	}

	return 0;
}

int stcp_kernel_chacha_encrypt(
	const u8 key[STCP_AEAD_KEY_LEN],
	u64 nonce_counter,
	const u8 *associated_data,
	size_t associated_data_len,
	const u8 *plaintext,
	size_t plaintext_len,
	u8 *ciphertext_and_tag,
	size_t ciphertext_capacity
)
{
	size_t required_len;

	if (!key || !ciphertext_and_tag)
		return -EINVAL;

	if (!plaintext && plaintext_len != 0)
		return -EINVAL;

	if (!associated_data && associated_data_len != 0)
		return -EINVAL;

	if (plaintext_len > SIZE_MAX - STCP_CHACHA_TAG_LEN)
		return -EOVERFLOW;

	required_len =
		plaintext_len +
		STCP_CHACHA_TAG_LEN;

	if (ciphertext_capacity < required_len)
		return -ENOSPC;

	/*
	 * Output layout:
	 *
	 *   ciphertext_and_tag[0 .. plaintext_len]
	 *   ciphertext_and_tag[plaintext_len .. +16] = Poly1305 tag
	 *
	 * The associated data is authenticated but not copied into the
	 * destination buffer.
	 */
	chacha20poly1305_encrypt(
		ciphertext_and_tag,
		plaintext,
		plaintext_len,
		associated_data,
		associated_data_len,
		nonce_counter,
		key
	);

	return 0;
}

int stcp_kernel_chacha_decrypt(
	const u8 key[STCP_AEAD_KEY_LEN],
	u64 nonce_counter,
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

	if (!associated_data && associated_data_len != 0)
		return -EINVAL;

	if (ciphertext_and_tag_len < STCP_CHACHA_TAG_LEN)
		return -EBADMSG;

	plaintext_len =
		ciphertext_and_tag_len -
		STCP_CHACHA_TAG_LEN;

	if (plaintext_capacity < plaintext_len)
		return -ENOSPC;

	/*
	 * Returns false when the Poly1305 authentication tag is invalid.
	 * The caller must discard the plaintext buffer in that case.
	 */
	if (!chacha20poly1305_decrypt(
		    plaintext,
		    ciphertext_and_tag,
		    ciphertext_and_tag_len,
		    associated_data,
		    associated_data_len,
		    nonce_counter,
		    key
	    )) {
		if (plaintext_len) {
			memzero_explicit(
				plaintext,
				plaintext_len
			);
		}

		return -EBADMSG;
	}

	return 0;
}
