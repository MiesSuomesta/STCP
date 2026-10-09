cd /srv/stcp-project/STCP/version-to-use

mkdir -p /tmp/stcp-x25519-test/include/zephyr/logging
mkdir -p /tmp/stcp-x25519-test/include/zephyr/sys
mkdir -p /tmp/stcp-x25519-test/include/stcp

cat >/tmp/stcp-x25519-test/include/zephyr/kernel.h <<'EOF'
#pragma once
#include <stdlib.h>
#define k_malloc malloc
#define k_free free
EOF

cat >/tmp/stcp-x25519-test/include/zephyr/logging/log.h <<'EOF'
#pragma once
#define LOG_MODULE_REGISTER(...)
#define LOG_ERR(...)
EOF

cat >/tmp/stcp-x25519-test/include/zephyr/sys/util.h <<'EOF'
#pragma once
#define ARG_UNUSED(x) (void)(x)
EOF

cat >/tmp/stcp-x25519-test/include/zephyr/sys/printk.h <<'EOF'
#pragma once
EOF

cat >/tmp/stcp-x25519-test/include/stcp/stcp_debug.h <<'EOF'
#pragma once
#define STCP_CRYPTO_PRINTK(...)
EOF

cp zephyr/nordic/module-v2/include/stcp/stcp_x25519_soft.h \
   /tmp/stcp-x25519-test/include/stcp/

cat >/tmp/stcp-x25519-test/test.c <<'EOF'
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stcp/stcp_x25519_soft.h>

static int hex2bin(const char *hex, uint8_t out[32])
{
    for (int i = 0; i < 32; i++) {
        unsigned int x;
        if (sscanf(hex + i * 2, "%2x", &x) != 1)
            return -1;
        out[i] = (uint8_t)x;
    }
    return 0;
}

static int check(const char *name, const uint8_t actual[32],
                 const char *expected_hex)
{
    uint8_t expected[32];

    if (hex2bin(expected_hex, expected) != 0)
        return 1;

    if (memcmp(actual, expected, 32) != 0) {
        printf("%s FAIL\nactual: ", name);
        for (int i = 0; i < 32; i++)
            printf("%02x", actual[i]);
        printf("\n");
        return 1;
    }

    printf("%s PASS\n", name);
    return 0;
}

int main(void)
{
    uint8_t alice_secret[32], bob_secret[32];
    uint8_t alice_pub[32], bob_pub[32];
    uint8_t alice_shared[32], bob_shared[32];

    hex2bin("77076d0a7318a57d3c16c17251b26645"
            "df4c2f87ebc0992ab177fba51db92c2a", alice_secret);

    hex2bin("5dab087e624a8a4b79e17f8b83800ee6"
            "6f3bb1292618b6fd1c2f8b27ff88e0eb", bob_secret);

    if (stcp_x25519_soft_public(alice_pub, alice_secret) ||
        stcp_x25519_soft_public(bob_pub, bob_secret) ||
        stcp_x25519_soft(alice_shared, alice_secret, bob_pub) ||
        stcp_x25519_soft(bob_shared, bob_secret, alice_pub)) {
        puts("X25519 calculation FAIL");
        return 1;
    }

    int failed = 0;

    failed |= check("Alice public", alice_pub,
        "8520f0098930a754748b7ddcb43ef75a"
        "0a6d4d4d7f7d6d4d5d6d4d4d7f7d6d4d");

    failed |= check("Bob public", bob_pub,
        "de9edb7d7b7dc1b4d35b61c2ece43537"
        "3f8343c85b78674dadfc7e146f882b4f");

    failed |= check("Shared secret", alice_shared,
        "4a5d9d5ba4ce2de1728e3bf480350f25"
        "e07e21c947d19e3376f09b3c1e161742");

    failed |= check("Alice/Bob agreement",
        alice_shared,
        "4a5d9d5ba4ce2de1728e3bf480350f25"
        "e07e21c947d19e3376f09b3c1e161742");

    if (memcmp(alice_shared, bob_shared, 32) != 0) {
        puts("Alice/Bob symmetry FAIL");
        failed = 1;
    } else {
        puts("Alice/Bob symmetry PASS");
    }

    return failed;
}
EOF
