/* SPDX-License-Identifier: GPL-2.0 */
#ifndef STCP_AES256_GCM_H
#define STCP_AES256_GCM_H

#include <linux/types.h>
#include "stcp_crypto.h"

int stcp_kernel_aes256_gcm_encrypt(
	const u8 key[STCP_AEAD_KEY_LEN], u64 nonce,
	const u8 *aad, size_t aad_len, const u8 *plain, size_t plain_len,
	u8 *out, size_t out_len);
int stcp_kernel_aes256_gcm_decrypt(
	const u8 key[STCP_AEAD_KEY_LEN], u64 nonce,
	const u8 *aad, size_t aad_len, const u8 *cipher, size_t cipher_len,
	u8 *out, size_t out_len);
int stcp_kernel_aes256_gcm_decrypt_in_place(
	const u8 key[STCP_AEAD_KEY_LEN], u64 nonce,
	const u8 *aad, size_t aad_len, u8 *cipher, size_t cipher_len);
#endif
