#include <stdio.h>
#include <stdlib.h>

// KERNEL SOCKET HACK
struct kernel_socket {
    int fd;
    void *kctx;
    void *resolved_host;
};

void * stcp_rust_kernel_socket_create(int fd)
{
    struct kernel_socket *p = malloc(sizeof(struct kernel_socket));
    printf("ALLOC kernel_socket %p fd=%d\n", p, fd);
    if (!p) return NULL;

    p->fd = fd;
    p->kctx = NULL;
    p->resolved_host = NULL;

    return p;
}

void stcp_rust_kernel_socket_destroy(void *p)
{
    printf("FREE kernel_socket %p\n", p);
    free(p);
}
