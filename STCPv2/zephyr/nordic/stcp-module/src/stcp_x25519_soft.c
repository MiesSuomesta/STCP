/*
 * Compact RFC 7748 X25519 implementation for Zephyr targets whose PSA
 * backend does not expose Montgomery/X25519 operations.
 *
 * The field arithmetic is derived from the public-domain TweetNaCl
 * Curve25519 implementation. It operates in constant-time with respect to
 * the secret scalar and has no heap or platform dependencies.
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <stcp/stcp_x25519_soft.h>

typedef int64_t gf[16];

static const gf gf_121665 = {0xdb41, 1};

static void car25519(gf o)
{
    int i;
    int64_t c;

    for (i = 0; i < 16; ++i) {
        o[i] += (int64_t)1 << 16;
        c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
        o[i] -= c << 16;
    }
}

static void sel25519(gf p, gf q, int b)
{
    int i;
    int64_t t;
    const int64_t mask = -(int64_t)b;

    for (i = 0; i < 16; ++i) {
        t = mask & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

static void pack25519(uint8_t out[32], const gf n)
{
    int i, j;
    gf m, t;

    for (i = 0; i < 16; ++i) {
        t[i] = n[i];
    }
    car25519(t);
    car25519(t);
    car25519(t);

    for (j = 0; j < 2; ++j) {
        m[0] = t[0] - 0xffed;
        for (i = 1; i < 15; ++i) {
            m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xffff;
        }
        m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
        {
            const int b = (int)((m[15] >> 16) & 1);
            m[14] &= 0xffff;
            sel25519(t, m, 1 - b);
        }
    }

    for (i = 0; i < 16; ++i) {
        out[2 * i] = (uint8_t)(t[i] & 0xff);
        out[2 * i + 1] = (uint8_t)(t[i] >> 8);
    }
}

static void unpack25519(gf out, const uint8_t in[32])
{
    int i;

    for (i = 0; i < 16; ++i) {
        out[i] = (int64_t)in[2 * i] + ((int64_t)in[2 * i + 1] << 8);
    }
    out[15] &= 0x7fff;
}

static void add(gf out, const gf a, const gf b)
{
    int i;
    for (i = 0; i < 16; ++i) {
        out[i] = a[i] + b[i];
    }
}

static void sub(gf out, const gf a, const gf b)
{
    int i;
    for (i = 0; i < 16; ++i) {
        out[i] = a[i] - b[i];
    }
}

static void mul(gf out, const gf a, const gf b)
{
    int i, j;
    int64_t t[31] = {0};

    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j) {
            t[i + j] += a[i] * b[j];
        }
    }
    for (i = 0; i < 15; ++i) {
        t[i] += 38 * t[i + 16];
    }
    for (i = 0; i < 16; ++i) {
        out[i] = t[i];
    }
    car25519(out);
    car25519(out);
}

static void square(gf out, const gf a)
{
    mul(out, a, a);
}

static void inv25519(gf out, const gf in)
{
    int i;
    gf c;

    for (i = 0; i < 16; ++i) {
        c[i] = in[i];
    }
    for (i = 253; i >= 0; --i) {
        square(c, c);
        if (i != 2 && i != 4) {
            mul(c, c, in);
        }
    }
    for (i = 0; i < 16; ++i) {
        out[i] = c[i];
    }
}

int stcp_x25519_soft(uint8_t out[32], const uint8_t scalar[32],
                     const uint8_t point[32])
{
    uint8_t z[32];
    int i;
    gf x, a, b, c, d, e, f;

    if (out == NULL || scalar == NULL || point == NULL) {
        return -EINVAL;
    }

    memcpy(z, scalar, sizeof(z));
    z[31] = (uint8_t)((z[31] & 127U) | 64U);
    z[0] &= 248U;
    unpack25519(x, point);

    for (i = 0; i < 16; ++i) {
        b[i] = x[i];
        d[i] = a[i] = c[i] = 0;
    }
    a[0] = d[0] = 1;

    for (i = 254; i >= 0; --i) {
        const int r = (z[i >> 3] >> (i & 7)) & 1;
        sel25519(a, b, r);
        sel25519(c, d, r);
        add(e, a, c);
        sub(a, a, c);
        add(c, b, d);
        sub(b, b, d);
        square(d, e);
        square(f, a);
        mul(a, c, a);
        mul(c, b, e);
        add(e, a, c);
        sub(a, a, c);
        square(b, a);
        sub(c, d, f);
        mul(a, c, gf_121665);
        add(a, a, d);
        mul(c, c, a);
        mul(a, d, f);
        mul(d, b, x);
        square(b, e);
        sel25519(a, b, r);
        sel25519(c, d, r);
    }

    inv25519(c, c);
    mul(a, a, c);
    pack25519(out, a);
    memset(z, 0, sizeof(z));
    return 0;
}

int stcp_x25519_soft_public(uint8_t public_key[32], const uint8_t secret[32])
{
    uint8_t basepoint[32] = {9};
    return stcp_x25519_soft(public_key, secret, basepoint);
}

bool stcp_x25519_soft_is_all_zero(const uint8_t value[32])
{
    uint8_t acc = 0;
    int i;

    for (i = 0; i < 32; ++i) {
        acc |= value[i];
    }
    return acc == 0;
}
