#include <linux/random.h>
#include <linux/types.h>

#include <stcp/ffi.h>

void stcp_kgetrandom(void *buf, size_t len)
{
    get_random_bytes(buf, len);
}
EXPORT_SYMBOL_GPL(stcp_kgetrandom);
