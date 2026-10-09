#ifndef STCP_X25519_SOFT_H
#define STCP_X25519_SOFT_H

#include <stdbool.h>
#include <stdint.h>

int stcp_x25519_soft(uint8_t out[32], const uint8_t scalar[32],
                     const uint8_t point[32]);
int stcp_x25519_soft_public(uint8_t public_key[32], const uint8_t secret[32]);
bool stcp_x25519_soft_is_all_zero(const uint8_t value[32]);

#endif
