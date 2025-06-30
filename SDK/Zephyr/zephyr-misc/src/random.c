
#include <zephyr/random/random.h>
#include "stcp_random.h"

uint8_t zephyr_rand_byte(void) {
    return sys_rand32_get() & 0xFF;
}
