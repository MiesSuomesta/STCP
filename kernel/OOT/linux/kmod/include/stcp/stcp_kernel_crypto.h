#pragma once 

#include <crypto/internal/ecc.h>

#include <linux/types.h>

#define SCTP_CRYPTO_CURVE_ID_INT    ECC_CURVE_NIST_P256
#define STCP_CRYPTO_KEY_PRIV_LEN    ECC_MAX_DIGITS         /* 256-bit private key */
#define STCP_CRYPTO_KEY_PUB_LEN    (ECC_MAX_DIGITS*2)      /* X(32) || Y(32) */
#define STCP_CRYPTO_KEY_PUB_XY_LEN  ECC_MAX_DIGITS         /* X(32) || Y(32) */
#define STCP_CRYPTO_KEY_SHARED_LEN  ECC_MAX_DIGITS         /* 256-bit shared secret */

typedef u8 stcp_crypto_array_item_t;

struct stcp_crypto_pubkey {
    stcp_crypto_array_item_t x[STCP_CRYPTO_KEY_PUB_XY_LEN];
    stcp_crypto_array_item_t y[STCP_CRYPTO_KEY_PUB_XY_LEN];
};

struct stcp_crypto_secret {
    stcp_crypto_array_item_t data[STCP_CRYPTO_KEY_SHARED_LEN];
    u32 len;
};

struct stcp_ecdh_private {
    stcp_crypto_array_item_t data[STCP_CRYPTO_KEY_PRIV_LEN];
    u32 len;
};

int stcp_crypto_generate_keypair(struct stcp_crypto_pubkey *out_pub,
                               struct stcp_crypto_secret *out_priv);

int stcp_crypto_compute_shared(const struct stcp_crypto_secret *priv,
                             const struct stcp_crypto_pubkey *peer_pub,
                             struct stcp_crypto_secret *out_shared);
