#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stcp/stcp_v2_internal.h>

/*
 * struct stcp_v2_socket embeds the Zephyr RX thread stack.
 *
 * K_KERNEL_STACK_MEMBER() gives the member the required internal layout, but
 * every array element must also start on a sufficiently aligned address.
 * The RX stack offset inside stcp_v2_socket is a multiple of 32 bytes, so
 * forcing each socket slot to 32-byte alignment also makes sock->rx_stack
 * 32-byte aligned.
 *
 * Use an aligned wrapper instead of relying on the linker alignment of a
 * plain struct array. The wrapper size is rounded up to its 32-byte
 * alignment, so every element in the array is aligned, not only element 0.
 */
struct stcp_v2_socket_slot {
    struct stcp_v2_socket socket;
} __aligned(32);

static struct stcp_v2_socket_slot socket_slots[CONFIG_STCP_V2_MAX_SOCKETS]
    __aligned(32);
static K_MUTEX_DEFINE(sockets_lock);

struct stcp_v2_socket *stcp_v2_socket_alloc(void)
{
    struct stcp_v2_socket *ret = NULL;

    k_mutex_lock(&sockets_lock, K_FOREVER);
    for (size_t i = 0; i < ARRAY_SIZE(socket_slots); ++i) {
        if (socket_slots[i].socket.used) {
            continue;
        }
        memset(&socket_slots[i].socket, 0, sizeof(socket_slots[i].socket));
        socket_slots[i].socket.used = true;
        socket_slots[i].socket.fd = -1;
        k_sem_init(&socket_slots[i].socket.event, 0, 1);
        k_sem_init(&socket_slots[i].socket.rx_ready, 0, 1);
        atomic_clear(&socket_slots[i].socket.rx_running);
        atomic_clear(&socket_slots[i].socket.rx_stop);
        ret = &socket_slots[i].socket;
        printk("STCP ALIGN slot=%u sock=%p rx_stack=%p sock_mod32=%lu stack_mod32=%lu\n",
               (unsigned int)i,
               ret,
               ret->rx_stack,
               (unsigned long)((uintptr_t)ret & 31U),
               (unsigned long)((uintptr_t)ret->rx_stack & 31U));
        break;
    }
    k_mutex_unlock(&sockets_lock);
    return ret;
}

void stcp_v2_socket_free(struct stcp_v2_socket *sock)
{
    if (sock == NULL) {
        return;
    }
    k_mutex_lock(&sockets_lock, K_FOREVER);
    memset(sock, 0, sizeof(*sock));
    sock->fd = -1;
    k_mutex_unlock(&sockets_lock);
}

void stcp_v2_signal(struct stcp_v2_socket *sock)
{
    if (sock != NULL) {
        k_sem_give(&sock->event);
    }
}
