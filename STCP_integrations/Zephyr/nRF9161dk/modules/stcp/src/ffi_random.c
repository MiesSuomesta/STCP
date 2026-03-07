#include <zephyr/random/random.h>

void stcp_random_get(uint8_t *buf, size_t len)
{
    sys_rand_get(buf, len);
}
