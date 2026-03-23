// kmod/stcp_kernel_ecdh.c
// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <crypto/ecdh.h>
#include <crypto/internal/ecc.h>

#include <linux/kernel.h>

#include <stcp/debug.h>
#include <stcp/stcp_kernel_crypto.h>

// 256bit  
#define STCP_ECC_CURVE_ID           ECC_CURVE_NIST_P256

// Montako byteä vaadittu bittisyys vie (256/64 == 4 Byteä)
#define STCP_ECC_NDIGITS            ECC_CURVE_NIST_P256_DIGITS

// Montako BYTEä vaaditaan talletukseen (4 << 3) == 32
#define STCP_ECC_NBYTES             (STCP_ECC_NDIGITS << ECC_DIGITS_TO_BYTES_SHIFT)

// X ja Y vektorit (ECC_BYTEä per vektori)
#define STCP_ECC_PUBLIC_KEY_SIZE_IN_BYTES      (STCP_ECC_NBYTES * 2)

// Vain data vektori
#define STCP_ECC_PRIVATE_KEY_SIZE_IN_BYTES     STCP_ECC_NBYTES
#define STCP_ECC_SHARED_KEY_SIZE_IN_BYTES      STCP_ECC_NBYTES

// Taulukoiden allokointiin 
#define STCP_ECC_PRIVATE_DWORDS        STCP_ECC_NDIGITS        // 4
#define STCP_ECC_PUBLIC_DWORDS        (STCP_ECC_NDIGITS * 2)   // 8
#define STCP_ECC_SHARED_DWORDS         STCP_ECC_NDIGITS    

/*
 * STCP internal: Convert ECC digits[] → big endian bytes[]
 * Equivalent to upstream ecc_digits_to_bytes(), for kernels where it doesn't exist.
 * 
 * Done with AI.
 */
static void stcp_ecc_digits_to_bytes(const u64 *src_digits,
                                     unsigned int ndigits,
                                     u8 *dst,
                                     unsigned int nbytes)
{
    int i;
    u64 v;

    /* digits[] = little-endian limbs
     * output   = big-endian byte array
     */

    for (i = ndigits - 1; i >= 0; i--) {
        v = src_digits[i];
        *dst++ = (v >> 56) & 0xff;
        *dst++ = (v >> 48) & 0xff;
        *dst++ = (v >> 40) & 0xff;
        *dst++ = (v >> 32) & 0xff;
        *dst++ = (v >> 24) & 0xff;
        *dst++ = (v >> 16) & 0xff;
        *dst++ = (v >>  8) & 0xff;
        *dst++ = (v >>  0) & 0xff;
    }
}

/*
 * Luo ECC-avainpari:
 *  - generoi 32 tavun random privaatin
 *  - asettaa sen KPP:lle secretiksi
 *  - generoi public-keyn (64 tavua: X||Y)
 *  - tallettaa privaatin ABI:in (stcp_crypto_secret)
 */
int stcp_crypto_generate_keypair(struct stcp_crypto_pubkey *out_pub,
                                 struct stcp_crypto_secret *out_priv)
{
    u64 private_key[ STCP_ECC_PRIVATE_DWORDS ]  = { 0 };
    u64 public_key [ STCP_ECC_PUBLIC_DWORDS  ]  = { 0 };
    int err;

    if (!out_pub || !out_priv) {
        SDBG("Null arg! %px and %px", out_pub, out_priv);
        return -22;
    }

    /* 1) Generoi privaatin avaimen */
    SDBG("Generating private key: %d, %d, %px", STCP_ECC_CURVE_ID, STCP_ECC_NDIGITS, private_key);
    err = ecc_gen_privkey(STCP_ECC_CURVE_ID, STCP_ECC_NDIGITS, private_key);
    SDBG("Generated private key, ret: %d", err);
    if (err) {
        goto out;
    }

    /* 2) Generoi julkinen avain */
    SDBG("Generating public key....");
    err = ecc_make_pub_key(STCP_ECC_CURVE_ID, STCP_ECC_NDIGITS, private_key, public_key);
    SDBG("Generated public key, ret: %d", err);
    if (err) {
        goto out;
    }
    SDBG("Crypto: Preparing X....");
    stcp_ecc_digits_to_bytes(public_key,
                             STCP_ECC_NDIGITS,        // X's bytes
                             out_pub->x,
                             STCP_ECC_NBYTES);

    SDBG("Crypto: Preparing Y....");
    stcp_ecc_digits_to_bytes(public_key + STCP_ECC_NDIGITS,
                             STCP_ECC_NDIGITS,        // Y's bytes
                             out_pub->y,
                             STCP_ECC_NBYTES);

    SDBG("Crypto: keypair done!");
out:
    memzero_explicit(private_key, sizeof(private_key));
    memzero_explicit(public_key,  sizeof(public_key));
    return err;
}

/*
 * Laske jaettu salaisuus:
 *  - otetaan oma priv-key (priv->data)
 *  - asetaan se KPP:lle secretiksi
 *  - annetaan peerin public-key (X||Y) inputtina
 *  - luetaan ulos 32 tavun shared secret
 */
int stcp_crypto_compute_shared(const struct stcp_crypto_secret *private_key,
                               const struct stcp_crypto_pubkey *peer_public_key,
                               struct stcp_crypto_secret *out_shared_key)
{
    int ret = 0;
    
    u64 stack_private_key   [ STCP_ECC_PRIVATE_DWORDS ] = { 0 };
    u64 stack_public_key    [ STCP_ECC_PUBLIC_DWORDS  ] = { 0 };
    u64 stack_shared_secret [ STCP_ECC_SHARED_DWORDS  ] = { 0 };

    if (!private_key || !peer_public_key || !out_shared_key) {
        SDBG("No valid param: %px, %px, %px", private_key, peer_public_key,out_shared_key);
        return -EINVAL;
    }

    // Prepare stack arrays
    SDBG("Crypto: Initialising private key....");
    ecc_digits_from_bytes(
                        private_key->data,
                        STCP_ECC_NBYTES,
                        stack_private_key,
                        STCP_ECC_NDIGITS
                    );

    SDBG("Crypto: Loading X....");
    ecc_digits_from_bytes(peer_public_key->x,
                          STCP_ECC_NBYTES,        // X bytes
                          stack_public_key,
                          STCP_ECC_NDIGITS);

    SDBG("Crypto: Loading Y....");
    ecc_digits_from_bytes(peer_public_key->y,
                          STCP_ECC_NBYTES,        // Y bytes
                          stack_public_key + STCP_ECC_NBYTES,
                          STCP_ECC_NDIGITS);

    SDBG("Crypto: Computing shared key....");

    ret = crypto_ecdh_shared_secret(
        STCP_ECC_CURVE_ID, 
        STCP_ECC_NDIGITS, 
        stack_private_key, 
        stack_public_key,
        stack_shared_secret);

    SDBG("Crypto: Return from compute shared: %d", ret);
    if (ret < 0) {
        SDBG("Crypto: Error while calculating shared key");
        goto out;
    }

    /* Tallenna private key ABI-rakenteeseen. */
    SDBG("Crypto: Preparing shared key to final place..");
    stcp_ecc_digits_to_bytes(stack_shared_secret,
                             STCP_ECC_NDIGITS,
                             out_shared_key->data,
                             STCP_ECC_NBYTES);

    SDBG("Crypto: Made shared key (%d bytes)", STCP_ECC_NBYTES);
    out_shared_key->len = STCP_ECC_NBYTES;

out:
    return ret;
}

