#include <linux/types.h>

void *stcp_kernel_memcpy(
    void *dst,
    const void *src,
    size_t len
)
{
    unsigned char *d = dst;
    const unsigned char *s = src;

    while (len--)
        *d++ = *s++;

    return dst;
}

void *stcp_kernel_memset(
    void *dst,
    int value,
    size_t len
)
{
    unsigned char *d = dst;

    while (len--)
        *d++ = (unsigned char)value;

    return dst;
}
