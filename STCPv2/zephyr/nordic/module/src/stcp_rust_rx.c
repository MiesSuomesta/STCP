#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_internal.h>
#include <stcp/stcp_rust_ffi.h>
LOG_MODULE_REGISTER(stcp_rust_rx, CONFIG_STCP_LOG_LEVEL);

static void rust_rx_thread(void *p1, void *p2, void *p3)
{
    struct stcp_ctx *ctx = p1;
    uint8_t buffer[CONFIG_STCP_RUST_RX_BUFFER_SIZE];
    ARG_UNUSED(p2); ARG_UNUSED(p3);
    while (!ctx->rust_rx_stop && ctx->carrier_fd >= 0) {
        ssize_t n = zsock_recv(ctx->carrier_fd, buffer, sizeof(buffer), 0);
        if (n > 0) {
            int rc;
            if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_WIRE)) {
                LOG_HEXDUMP_DBG(buffer, MIN((size_t)n, (size_t)CONFIG_STCP_RUST_HEXDUMP_BYTES),
                                "Rust STCP RX");
            }
            rc = stcp_rust_carrier_receive(ctx->rust_ctx, buffer, (size_t)n);
            if (rc < 0 && rc != -EAGAIN) LOG_ERR("rust carrier receive rc=%d", rc);
            k_sem_give(&ctx->rust_event);
            continue;
        }
        if (n == 0) break;
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            k_sleep(K_MSEC(1));
            continue;
        }
        if (!ctx->rust_rx_stop) LOG_ERR("carrier recv failed errno=%d", errno);
        break;
    }
    ctx->rust_rx_running = false;
    k_sem_give(&ctx->rust_event);
}

int stcp_rust_rx_start(struct stcp_ctx *ctx)
{
    if (!ctx || ctx->rust_rx_running) return 0;
    ctx->rust_rx_stop = false;
    ctx->rust_rx_running = true;
    k_thread_create(&ctx->rust_rx_thread, ctx->rust_rx_stack,
                    K_KERNEL_STACK_SIZEOF(ctx->rust_rx_stack),
                    rust_rx_thread, ctx, NULL, NULL,
                    CONFIG_STCP_RUST_RX_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&ctx->rust_rx_thread, "stcp-rust-rx");
    return 0;
}

void stcp_rust_rx_stop(struct stcp_ctx *ctx)
{
    if (!ctx || !ctx->rust_rx_running) return;
    ctx->rust_rx_stop = true;
    if (ctx->carrier_fd >= 0) (void)zsock_shutdown(ctx->carrier_fd, ZSOCK_SHUT_RDWR);
    k_sem_take(&ctx->rust_event, K_MSEC(250));
    if (ctx->rust_rx_running) {
        k_thread_abort(&ctx->rust_rx_thread);
        ctx->rust_rx_running = false;
    }
}
