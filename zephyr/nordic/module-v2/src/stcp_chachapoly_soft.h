#ifndef STCP_CHACHAPOLY_SOFT_H_
#define STCP_CHACHAPOLY_SOFT_H_

/* Shared RFC 8439 fallback, allocation-free. Include from one platform C file. */
/*
 * RFC 8439 software ChaCha20-Poly1305 fallback.
 *
 * This path is used only when the PSA backend explicitly reports
 * PSA_ERROR_NOT_SUPPORTED for ChaCha20-Poly1305.  It is allocation-free:
 * Poly1305 is streamed over AAD/ciphertext, so large benchmark frames do not
 * consume a second payload-sized heap buffer.
 */
static uint32_t stcp_soft_load32_le(const uint8_t *p)
{
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static void stcp_soft_store32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t stcp_soft_rotl32(uint32_t v, unsigned int n)
{
    return (v << n) | (v >> (32U - n));
}

static void stcp_soft_chacha_qr(uint32_t *a, uint32_t *b,
                                uint32_t *c, uint32_t *d)
{
    *a += *b; *d ^= *a; *d = stcp_soft_rotl32(*d, 16);
    *c += *d; *b ^= *c; *b = stcp_soft_rotl32(*b, 12);
    *a += *b; *d ^= *a; *d = stcp_soft_rotl32(*d, 8);
    *c += *d; *b ^= *c; *b = stcp_soft_rotl32(*b, 7);
}

static void stcp_soft_chacha20_block(const uint8_t key[32], uint32_t counter,
                                     const uint8_t nonce[12], uint8_t out[64])
{
    static const uint8_t sigma[16] = "expand 32-byte k";
    uint32_t x[16];
    uint32_t w[16];
    int i;

    x[0] = stcp_soft_load32_le(sigma);
    x[1] = stcp_soft_load32_le(sigma + 4);
    x[2] = stcp_soft_load32_le(sigma + 8);
    x[3] = stcp_soft_load32_le(sigma + 12);
    for (i = 0; i < 8; i++) {
        x[4 + i] = stcp_soft_load32_le(key + 4 * i);
    }
    x[12] = counter;
    x[13] = stcp_soft_load32_le(nonce);
    x[14] = stcp_soft_load32_le(nonce + 4);
    x[15] = stcp_soft_load32_le(nonce + 8);

    memcpy(w, x, sizeof(x));

    for (i = 0; i < 10; i++) {
        stcp_soft_chacha_qr(&w[0], &w[4], &w[8], &w[12]);
        stcp_soft_chacha_qr(&w[1], &w[5], &w[9], &w[13]);
        stcp_soft_chacha_qr(&w[2], &w[6], &w[10], &w[14]);
        stcp_soft_chacha_qr(&w[3], &w[7], &w[11], &w[15]);

        stcp_soft_chacha_qr(&w[0], &w[5], &w[10], &w[15]);
        stcp_soft_chacha_qr(&w[1], &w[6], &w[11], &w[12]);
        stcp_soft_chacha_qr(&w[2], &w[7], &w[8], &w[13]);
        stcp_soft_chacha_qr(&w[3], &w[4], &w[9], &w[14]);
    }

    for (i = 0; i < 16; i++) {
        stcp_soft_store32_le(out + 4 * i, w[i] + x[i]);
    }

    memset(x, 0, sizeof(x));
    memset(w, 0, sizeof(w));
}

static void stcp_soft_chacha20_xor(const uint8_t key[32],
                                   const uint8_t nonce[12],
                                   uint32_t counter,
                                   const uint8_t *in,
                                   uint8_t *out,
                                   size_t len)
{
    uint8_t block[64];

    while (len != 0U) {
        size_t n;
        size_t i;

        stcp_soft_chacha20_block(key, counter++, nonce, block);
        n = len < sizeof(block) ? len : sizeof(block);

        for (i = 0; i < n; i++) {
            out[i] = in[i] ^ block[i];
        }

        in += n;
        out += n;
        len -= n;
    }

    memset(block, 0, sizeof(block));
}

struct stcp_soft_poly1305 {
    uint32_t r0, r1, r2, r3, r4;
    uint32_t s1, s2, s3, s4;
    uint32_t h0, h1, h2, h3, h4;
    uint32_t pad0, pad1, pad2, pad3;
};

static void stcp_soft_poly1305_init(struct stcp_soft_poly1305 *st,
                                    const uint8_t key[32])
{
    st->r0 = stcp_soft_load32_le(key + 0) & 0x3ffffffU;
    st->r1 = (stcp_soft_load32_le(key + 3) >> 2) & 0x3ffff03U;
    st->r2 = (stcp_soft_load32_le(key + 6) >> 4) & 0x3ffc0ffU;
    st->r3 = (stcp_soft_load32_le(key + 9) >> 6) & 0x3f03fffU;
    st->r4 = (stcp_soft_load32_le(key + 12) >> 8) & 0x00fffffU;

    st->s1 = st->r1 * 5U;
    st->s2 = st->r2 * 5U;
    st->s3 = st->r3 * 5U;
    st->s4 = st->r4 * 5U;

    st->h0 = 0U;
    st->h1 = 0U;
    st->h2 = 0U;
    st->h3 = 0U;
    st->h4 = 0U;

    st->pad0 = stcp_soft_load32_le(key + 16);
    st->pad1 = stcp_soft_load32_le(key + 20);
    st->pad2 = stcp_soft_load32_le(key + 24);
    st->pad3 = stcp_soft_load32_le(key + 28);
}

static void stcp_soft_poly1305_block(struct stcp_soft_poly1305 *st,
                                     const uint8_t m[16])
{
    uint32_t t0 = stcp_soft_load32_le(m);
    uint32_t t1 = stcp_soft_load32_le(m + 4);
    uint32_t t2 = stcp_soft_load32_le(m + 8);
    uint32_t t3 = stcp_soft_load32_le(m + 12);
    uint32_t c;
    uint64_t d0, d1, d2, d3, d4;

    st->h0 += t0 & 0x3ffffffU;
    st->h1 += ((t0 >> 26) | (t1 << 6)) & 0x3ffffffU;
    st->h2 += ((t1 >> 20) | (t2 << 12)) & 0x3ffffffU;
    st->h3 += ((t2 >> 14) | (t3 << 18)) & 0x3ffffffU;
    st->h4 += (t3 >> 8) | (1U << 24);

    d0 = (uint64_t)st->h0 * st->r0 +
         (uint64_t)st->h1 * st->s4 +
         (uint64_t)st->h2 * st->s3 +
         (uint64_t)st->h3 * st->s2 +
         (uint64_t)st->h4 * st->s1;
    d1 = (uint64_t)st->h0 * st->r1 +
         (uint64_t)st->h1 * st->r0 +
         (uint64_t)st->h2 * st->s4 +
         (uint64_t)st->h3 * st->s3 +
         (uint64_t)st->h4 * st->s2;
    d2 = (uint64_t)st->h0 * st->r2 +
         (uint64_t)st->h1 * st->r1 +
         (uint64_t)st->h2 * st->r0 +
         (uint64_t)st->h3 * st->s4 +
         (uint64_t)st->h4 * st->s3;
    d3 = (uint64_t)st->h0 * st->r3 +
         (uint64_t)st->h1 * st->r2 +
         (uint64_t)st->h2 * st->r1 +
         (uint64_t)st->h3 * st->r0 +
         (uint64_t)st->h4 * st->s4;
    d4 = (uint64_t)st->h0 * st->r4 +
         (uint64_t)st->h1 * st->r3 +
         (uint64_t)st->h2 * st->r2 +
         (uint64_t)st->h3 * st->r1 +
         (uint64_t)st->h4 * st->r0;

    c = (uint32_t)(d0 >> 26);
    st->h0 = (uint32_t)d0 & 0x3ffffffU;
    d1 += c;
    c = (uint32_t)(d1 >> 26);
    st->h1 = (uint32_t)d1 & 0x3ffffffU;
    d2 += c;
    c = (uint32_t)(d2 >> 26);
    st->h2 = (uint32_t)d2 & 0x3ffffffU;
    d3 += c;
    c = (uint32_t)(d3 >> 26);
    st->h3 = (uint32_t)d3 & 0x3ffffffU;
    d4 += c;
    c = (uint32_t)(d4 >> 26);
    st->h4 = (uint32_t)d4 & 0x3ffffffU;

    st->h0 += c * 5U;
    c = st->h0 >> 26;
    st->h0 &= 0x3ffffffU;
    st->h1 += c;
}

static void stcp_soft_poly1305_update_padded(struct stcp_soft_poly1305 *st,
                                             const uint8_t *data,
                                             size_t len)
{
    uint8_t block[16];

    while (len >= 16U) {
        stcp_soft_poly1305_block(st, data);
        data += 16U;
        len -= 16U;
    }

    if (len != 0U) {
        memset(block, 0, sizeof(block));
        memcpy(block, data, len);
        stcp_soft_poly1305_block(st, block);
        memset(block, 0, sizeof(block));
    }
}

static void stcp_soft_poly1305_finish(struct stcp_soft_poly1305 *st,
                                      uint8_t mac[16])
{
    uint32_t c, g0, g1, g2, g3, g4, mask;
    uint64_t f0, f1, f2, f3;

    c = st->h1 >> 26;
    st->h1 &= 0x3ffffffU;
    st->h2 += c;
    c = st->h2 >> 26;
    st->h2 &= 0x3ffffffU;
    st->h3 += c;
    c = st->h3 >> 26;
    st->h3 &= 0x3ffffffU;
    st->h4 += c;
    c = st->h4 >> 26;
    st->h4 &= 0x3ffffffU;
    st->h0 += c * 5U;
    c = st->h0 >> 26;
    st->h0 &= 0x3ffffffU;
    st->h1 += c;

    g0 = st->h0 + 5U;
    c = g0 >> 26;
    g0 &= 0x3ffffffU;
    g1 = st->h1 + c;
    c = g1 >> 26;
    g1 &= 0x3ffffffU;
    g2 = st->h2 + c;
    c = g2 >> 26;
    g2 &= 0x3ffffffU;
    g3 = st->h3 + c;
    c = g3 >> 26;
    g3 &= 0x3ffffffU;
    g4 = st->h4 + c - (1U << 26);

    mask = (g4 >> 31) - 1U;
    g0 &= mask;
    g1 &= mask;
    g2 &= mask;
    g3 &= mask;
    g4 &= mask;
    mask = ~mask;

    st->h0 = (st->h0 & mask) | g0;
    st->h1 = (st->h1 & mask) | g1;
    st->h2 = (st->h2 & mask) | g2;
    st->h3 = (st->h3 & mask) | g3;
    st->h4 = (st->h4 & mask) | g4;

    f0 = (uint32_t)(st->h0 | (st->h1 << 26));
    f1 = (uint32_t)((st->h1 >> 6) | (st->h2 << 20));
    f2 = (uint32_t)((st->h2 >> 12) | (st->h3 << 14));
    f3 = (uint32_t)((st->h3 >> 18) | (st->h4 << 8));

    f0 += st->pad0;
    f1 += st->pad1 + (f0 >> 32);
    f0 &= 0xffffffffU;
    f2 += st->pad2 + (f1 >> 32);
    f1 &= 0xffffffffU;
    f3 += st->pad3 + (f2 >> 32);
    f2 &= 0xffffffffU;

    stcp_soft_store32_le(mac, (uint32_t)f0);
    stcp_soft_store32_le(mac + 4, (uint32_t)f1);
    stcp_soft_store32_le(mac + 8, (uint32_t)f2);
    stcp_soft_store32_le(mac + 12, (uint32_t)f3);

    memset(st, 0, sizeof(*st));
}

static void stcp_soft_store64_le(uint8_t *p, uint64_t v)
{
    unsigned int i;

    for (i = 0; i < 8U; i++) {
        p[i] = (uint8_t)v;
        v >>= 8;
    }
}

static void stcp_soft_chachapoly_tag(const uint8_t key[32],
                                     const uint8_t nonce[12],
                                     const uint8_t *aad,
                                     size_t aad_len,
                                     const uint8_t *cipher,
                                     size_t cipher_len,
                                     uint8_t tag[16])
{
    uint8_t block0[64];
    uint8_t lens[16];
    struct stcp_soft_poly1305 st;

    stcp_soft_chacha20_block(key, 0U, nonce, block0);
    stcp_soft_poly1305_init(&st, block0);

    stcp_soft_poly1305_update_padded(&st, aad, aad_len);
    stcp_soft_poly1305_update_padded(&st, cipher, cipher_len);

    stcp_soft_store64_le(lens, (uint64_t)aad_len);
    stcp_soft_store64_le(lens + 8, (uint64_t)cipher_len);
    stcp_soft_poly1305_block(&st, lens);

    stcp_soft_poly1305_finish(&st, tag);

    memset(block0, 0, sizeof(block0));
    memset(lens, 0, sizeof(lens));
}

static int stcp_soft_ct_equal16(const uint8_t a[16], const uint8_t b[16])
{
    uint8_t diff = 0U;
    unsigned int i;

    for (i = 0; i < 16U; i++) {
        diff |= (uint8_t)(a[i] ^ b[i]);
    }
    return diff == 0U;
}

static int stcp_soft_chachapoly_encrypt(const uint8_t key[32],
                                        const uint8_t nonce[12],
                                        const uint8_t *aad,
                                        size_t aad_len,
                                        const uint8_t *plain,
                                        size_t plain_len,
                                        uint8_t *out)
{
    uint8_t tag[16];

    stcp_soft_chacha20_xor(key, nonce, 1U, plain, out, plain_len);
    stcp_soft_chachapoly_tag(key, nonce, aad, aad_len, out, plain_len, tag);
    memcpy(out + plain_len, tag, sizeof(tag));
    memset(tag, 0, sizeof(tag));
    return 0;
}

static int stcp_soft_chachapoly_decrypt(const uint8_t key[32],
                                        const uint8_t nonce[12],
                                        const uint8_t *aad,
                                        size_t aad_len,
                                        const uint8_t *cipher,
                                        size_t cipher_len,
                                        uint8_t *out)
{
    size_t plain_len = cipher_len - 16U;
    uint8_t tag[16];

    stcp_soft_chachapoly_tag(key, nonce, aad, aad_len,
                            cipher, plain_len, tag);

    if (!stcp_soft_ct_equal16(tag, cipher + plain_len)) {
        memset(tag, 0, sizeof(tag));
        return -EBADMSG;
    }

    stcp_soft_chacha20_xor(key, nonce, 1U, cipher, out, plain_len);
    memset(tag, 0, sizeof(tag));
    return 0;
}

#endif /* STCP_CHACHAPOLY_SOFT_H_ */
