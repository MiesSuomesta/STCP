// kmod/stcp_rust_hooks_stubs.c
//
// Väliaikaiset stubit kaikille stcp_rust_* hook-funktioille,
// jotta moduuli linkittyy ja latautuu. Kaikki palauttaa -EOPNOTSUPP,
// kunnes Rust-puolen oikea toteutus on kytketty.

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/uio.h>
#include <linux/poll.h>

#define STCP_RUST_STUB_FN(name)                                      \
    int name(void)                                                   \
    {                                                                \
        pr_info("stcp_rust: stub %s() called (not implemented yet)\n", #name); \
        return -EOPNOTSUPP;                                          \
    }

// Jos haluat hieman tarkempia signatuureja, voit käyttää oikeita protoja,
// mutta linkityksen kannalta riittää, että symboli on olemassa.
// Varmin tapa ilman prototype-säätöä on käyttää varargsia:

#define STCP_RUST_STUB_VARARGS(name)                                 \
    int name(...)                                                    \
    {                                                                \
        pr_info("stcp_rust: stub %s() called (not implemented yet)\n", #name); \
        return -EOPNOTSUPP;                                          \
    }

// Socket operation -stubit:
STCP_RUST_STUB_VARARGS(stcp_rust_bind)
STCP_RUST_STUB_VARARGS(stcp_rust_listen)
STCP_RUST_STUB_VARARGS(stcp_rust_connect)
STCP_RUST_STUB_VARARGS(stcp_rust_accept)
STCP_RUST_STUB_VARARGS(stcp_rust_getname)
STCP_RUST_STUB_VARARGS(stcp_rust_ioctl)
STCP_RUST_STUB_VARARGS(stcp_rust_setsockopt)
STCP_RUST_STUB_VARARGS(stcp_rust_getsockopt)
STCP_RUST_STUB_VARARGS(stcp_rust_sendmsg)
STCP_RUST_STUB_VARARGS(stcp_rust_recvmsg)
STCP_RUST_STUB_VARARGS(stcp_rust_poll)
STCP_RUST_STUB_VARARGS(stcp_rust_shutdown)

// Mahdollinen free-hook; useampi logiikka käyttää tätä nimeä:
void stcp_rust_free(void *ptr)
{
    pr_info("stcp_rust: stub stcp_rust_free(%p) called (no-op)\n", ptr);
    // ei tee mitään vielä
}

// Mahdollinen close:
int stcp_rust_close(...)
{
    pr_info("stcp_rust: stub stcp_rust_close() called (not implemented yet)\n");
    return -EOPNOTSUPP;
}

