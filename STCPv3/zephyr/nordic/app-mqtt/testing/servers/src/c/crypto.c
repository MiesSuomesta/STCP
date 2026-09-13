#include <stdint.h>
#include <string.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/sha.h>

struct stcp_crypto_pubkey {
    uint8_t x[32];
    uint8_t y[32];
};

struct stcp_crypto_secret {
    uint8_t data[32];
};

int stcp_crypto_generate_keypair(
        struct stcp_crypto_pubkey *out_pub,
        struct stcp_crypto_secret *out_priv)
{
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!key) return -1;

    if (!EC_KEY_generate_key(key)) return -1;

    const BIGNUM *priv = EC_KEY_get0_private_key(key);
    const EC_POINT *pub = EC_KEY_get0_public_key(key);
    const EC_GROUP *group = EC_KEY_get0_group(key);

    BN_bn2binpad(priv, out_priv->data, 32);

    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();

    EC_POINT_get_affine_coordinates(group, pub, x, y, NULL);

    BN_bn2binpad(x, out_pub->x, 32);
    BN_bn2binpad(y, out_pub->y, 32);

    BN_free(x);
    BN_free(y);
    EC_KEY_free(key);

    return 0;
}

int stcp_crypto_compute_shared(
        const struct stcp_crypto_secret *priv,
        const struct stcp_crypto_pubkey *peer_pub,
        struct stcp_crypto_secret *out_shared)
{
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);

    BIGNUM *priv_bn = BN_bin2bn(priv->data, 32, NULL);
    EC_KEY_set_private_key(key, priv_bn);

    const EC_GROUP *group = EC_KEY_get0_group(key);

    EC_POINT *point = EC_POINT_new(group);

    BIGNUM *x = BN_bin2bn(peer_pub->x, 32, NULL);
    BIGNUM *y = BN_bin2bn(peer_pub->y, 32, NULL);

    EC_POINT_set_affine_coordinates(group, point, x, y, NULL);

    uint8_t secret[32];

    ECDH_compute_key(secret, 32, point, key, NULL);

    memcpy(out_shared->data, secret, 32);

    BN_free(x);
    BN_free(y);
    BN_free(priv_bn);
    EC_POINT_free(point);
    EC_KEY_free(key);

    return 0;
}
