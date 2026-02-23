#pragma once

#include <stdint.h>

#define ECC_MAX_DIGITS  32   /* 256 bit */

#define SCTP_CRYPTO_CURVE_ID_INT    1 /* Placeholder */
#define STCP_CRYPTO_KEY_PRIV_LEN    ECC_MAX_DIGITS
#define STCP_CRYPTO_KEY_PUB_LEN    (ECC_MAX_DIGITS * 2)
#define STCP_CRYPTO_KEY_PUB_XY_LEN  ECC_MAX_DIGITS
#define STCP_CRYPTO_KEY_SHARED_LEN  ECC_MAX_DIGITS

typedef uint8_t stcp_crypto_array_item_t;

struct stcp_crypto_pubkey {
    stcp_crypto_array_item_t x[STCP_CRYPTO_KEY_PUB_XY_LEN];
    stcp_crypto_array_item_t y[STCP_CRYPTO_KEY_PUB_XY_LEN];
};

struct stcp_crypto_secret {
    stcp_crypto_array_item_t data[STCP_CRYPTO_KEY_SHARED_LEN];
};

struct stcp_ecdh_private {
    stcp_crypto_array_item_t data[STCP_CRYPTO_KEY_PRIV_LEN];
};

int stcp_crypto_generate_keypair(struct stcp_crypto_pubkey *out_pub,
                                 struct stcp_crypto_secret *out_priv);

int stcp_crypto_compute_shared(const struct stcp_crypto_secret *priv,
                               const struct stcp_crypto_pubkey *peer_pub,
                               struct stcp_crypto_secret *out_shared);
