#pragma once

#include <linux/types.h>

#define STCP_CURVE25519_KEY_LEN 32
#define STCP_AEAD_KEY_LEN       32
#define STCP_AEAD_TAG_LEN       16

int stcp_kernel_x25519_keypair(
	u8 private_key[STCP_CURVE25519_KEY_LEN],
	u8 public_key[STCP_CURVE25519_KEY_LEN]
);

int stcp_kernel_x25519_shared(
	u8 shared_key[STCP_CURVE25519_KEY_LEN],
	const u8 private_key[STCP_CURVE25519_KEY_LEN],
	const u8 peer_public_key[STCP_CURVE25519_KEY_LEN]
);

int stcp_kernel_chacha_encrypt(
	const u8 key[STCP_AEAD_KEY_LEN],
	u64 nonce,
	const u8 *associated_data,
	size_t associated_data_len,
	const u8 *plaintext,
	size_t plaintext_len,
	u8 *ciphertext_and_tag,
	size_t ciphertext_capacity
);

int stcp_kernel_chacha_decrypt(
	const u8 key[STCP_AEAD_KEY_LEN],
	u64 nonce,
	const u8 *associated_data,
	size_t associated_data_len,
	const u8 *ciphertext_and_tag,
	size_t ciphertext_and_tag_len,
	u8 *plaintext,
	size_t plaintext_capacity
);
