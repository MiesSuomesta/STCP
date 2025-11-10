

#include <stcp/structures.h>   // stcp_sock, stcp_from_socket
#include <stcp/helpers.h>       // stcp_from_* helppereitä
#include <linux/spinlock.h>

void *stcp_rust_blob_get(struct socket *sock)
{
    struct stcp_sock *st = stcp_from_socket(sock);
    if (!st)
        return NULL;
    return st->rust;
}

void stcp_rust_blob_set(struct socket *sock, void *ctx)
{
    struct stcp_sock *st = stcp_from_socket(sock);
    if (!st)
        return;
    st->rust = ctx;
}

void stcp_rust_blob_lock(struct socket *sock)
{
    struct stcp_sock *st = stcp_from_socket(sock);
    if (!st)
        return;
    spin_lock(&st->rust_lock);
}

void stcp_rust_blob_unlock(struct socket *sock)
{
    struct stcp_sock *st = stcp_from_socket(sock);
    if (!st)
        return;
    spin_unlock(&st->rust_lock);
}
