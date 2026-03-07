
#include <zephyr/random/random.h>
#include <zephyr/kernel.h>
#include <string.h>

#include "uECC.h"
#include "stcp/debug.h"
#include "stcp/stcp_kernel_crypto.h"
LOG_MODULE_REGISTER(stcp_kernel_crypto_module, LOG_LEVEL_INF);

/* uECC RNG hook Zephyrille */
static int zephyr_rng(uint8_t *dest, unsigned size)
{
    sys_rand_get(dest, size);
    return 1; /* uECC odottaa non-zero = OK */
}

int stcp_crypto_generate_keypair(struct stcp_crypto_pubkey *out_pub,
                                 struct stcp_crypto_secret *out_priv)
{
    if (!out_pub || !out_priv) {
        return -1;
    }

    LDBG("sizeof pubkey %u\n", sizeof(struct stcp_crypto_pubkey));
    LDBG("sizeof secret %u\n", sizeof(struct stcp_crypto_secret));

    LDBG("Getting curve...");
    const struct uECC_Curve_t *curve = uECC_secp256r1();
    LDBG("Curve got...");

    /* uECC odottaa: pubkey = 64 tavua (x||y), privkey = 32 tavua */
    uint8_t pub_raw[STCP_CRYPTO_KEY_PUB_LEN];
    uint8_t priv_raw[STCP_CRYPTO_KEY_PRIV_LEN];
    LDBG("Allocated stack....");

    uECC_set_rng(zephyr_rng);
    LDBG("Random set ...");

    if (!uECC_make_key(pub_raw, priv_raw, curve)) {
        LDBG("Make key failed!");
        return -2;
    }

    LDBG("Keys made....");

    /* Kopioi x ja y omiin rakenteisiin */
    memcpy(out_pub->x, pub_raw, STCP_CRYPTO_KEY_PUB_XY_LEN);
    LDBG("Key pub X");
    memcpy(out_pub->y, pub_raw + STCP_CRYPTO_KEY_PUB_XY_LEN, STCP_CRYPTO_KEY_PUB_XY_LEN);
    LDBG("Key pub Y");
    
    /* Private key */
    LDBG("Check point");
    memcpy(out_priv->data, priv_raw, STCP_CRYPTO_KEY_PRIV_LEN);
    LDBG("Check the FINAL point");
    LDBG("Ret 0");
    return 0;
}

int stcp_crypto_compute_shared(const struct stcp_crypto_secret *priv,
                               const struct stcp_crypto_pubkey *peer_pub,
                               struct stcp_crypto_secret *out_shared)
{
    if (!priv || !peer_pub || !out_shared) {
        return -1;
    }

    const struct uECC_Curve_t *curve = uECC_secp256r1();

    uint8_t peer_pub_raw[STCP_CRYPTO_KEY_PUB_LEN];
    uint8_t shared_raw[STCP_CRYPTO_KEY_SHARED_LEN];

    /* Rakenna uECC:n odottama pubkey = x||y */
    memcpy(peer_pub_raw, peer_pub->x, STCP_CRYPTO_KEY_PUB_XY_LEN);
    memcpy(peer_pub_raw + STCP_CRYPTO_KEY_PUB_XY_LEN,
           peer_pub->y,
           STCP_CRYPTO_KEY_PUB_XY_LEN);

    if (!uECC_shared_secret(peer_pub_raw,
                            (const uint8_t *)priv->data,
                            shared_raw,
                            curve)) {
        printk("stcp_crypto: uECC_shared_secret failed\n");
        return -2;
    }

    memcpy(out_shared->data, shared_raw, STCP_CRYPTO_KEY_SHARED_LEN);
    return 0;
}
