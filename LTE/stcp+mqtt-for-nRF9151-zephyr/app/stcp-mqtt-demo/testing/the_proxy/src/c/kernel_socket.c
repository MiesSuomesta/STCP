#include <stdio.h>
#include <stdlib.h>
#include "kernel_socket.h"

void * stcp_rust_kernel_socket_create(int fd)
{
    struct kernel_socket *p = malloc(sizeof(struct kernel_socket));
    printf("ALLOC kernel_socket %p fd=%d\n", p, fd);
    if (!p) return NULL;

    p->fd = fd;
    p->kctx = NULL;

    return p;
}

void stcp_rust_kernel_socket_dump(void *p) {
    if (p) {
        struct kernel_socket *ks = p;
        printf("kernel_socket=%p fd=%d", ks, ks->fd);
    }
}


int stcp_rust_api_transport_get_fd(void *p) {
    if (p) {
        struct kernel_socket *ks = p;
        printf("kernel_socket=%p fd=%d", ks, ks->fd);
        return ks->fd;
    }
    return -1;
}

void stcp_rust_kernel_socket_destroy(void *p)
{
    printf("FREE kernel_socket %p\n", p);
    free(p);
}
