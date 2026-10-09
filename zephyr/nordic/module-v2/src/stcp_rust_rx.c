#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/util.h>
#include <stcp/stcp_internal.h>
#include <stcp/stcp_debug.h>
#include <stcp/stcp_rust_ffi.h>
LOG_MODULE_REGISTER(stcp_rust_rx, CONFIG_STCP_LOG_LEVEL);

static void log_current_stack(const char *where)
{
#if defined(CONFIG_THREAD_STACK_INFO)
    size_t unused = 0;
    int rc = k_thread_stack_space_get(k_current_get(), &unused);

    if (rc == 0) {
        STCP_RX_INF("stack %s: unused=%zu used=%zu total=%u",
                where, unused,
                (size_t)CONFIG_STCP_RUST_RX_STACK_SIZE - unused,
                CONFIG_STCP_RUST_RX_STACK_SIZE);
    } else {
        STCP_RX_WRN("stack %s: measurement failed rc=%d", where, rc);
    }
#else
    ARG_UNUSED(where);
#endif
}

static void rust_rx_thread(void *p1, void *p2, void *p3)
{
    struct stcp_ctx *ctx = p1;
    uint8_t *buffer;
    ARG_UNUSED(p2); ARG_UNUSED(p3);

    buffer = k_malloc(CONFIG_STCP_RUST_RX_BUFFER_SIZE);
    if (buffer == NULL) {
        LOG_ERR("RX thread buffer allocation failed: bytes=%u",
                CONFIG_STCP_RUST_RX_BUFFER_SIZE);
        ctx->rust_rx_running = false;
        k_sem_give(&ctx->rust_event);
        return;
    }

    STCP_RX_INF("RX thread started: stack=%u heap_buffer=%u",
            CONFIG_STCP_RUST_RX_STACK_SIZE,
            CONFIG_STCP_RUST_RX_BUFFER_SIZE);
    log_current_stack("thread-start");
    while (!ctx->rust_rx_stop && ctx->carrier_fd >= 0) {
        ssize_t n = zsock_recv(ctx->carrier_fd, buffer, CONFIG_STCP_RUST_RX_BUFFER_SIZE, 0);
        if (n > 0) {
            int rc;
            if (IS_ENABLED(CONFIG_STCP_RUST_TRACE_WIRE)) {
                STCP_RX_HEXDUMP(buffer, MIN((size_t)n, (size_t)CONFIG_STCP_RUST_HEXDUMP_BYTES),
                                "Rust STCP RX");
            }
            STCP_RX_INF("Rust carrier receive start: bytes=%zd", n);
            log_current_stack("before-rust-receive");
            rc = stcp_rust_carrier_receive(ctx->rust_ctx, buffer, (size_t)n);
            log_current_stack("after-rust-receive");
            STCP_RX_INF("Rust carrier receive return: bytes=%zd rc=%d", n, rc);
            if (rc < 0 && rc != -EAGAIN) {
                LOG_ERR("rust carrier receive rc=%d", rc);
            }
            /* Synchronize poll readiness after Rust has parsed every complete
             * frame.  The Rust wake callback also latches this bit, but this
             * explicit check covers backends that complete parsing without a
             * separate callback. */
            if (stcp_rust_has_data(ctx->rust_ctx) > 0) {
                ctx->poll_read_wakeup = true;
            }
            k_sem_give(&ctx->rust_event);
            continue;
        }
        if (n == 0) {
            if (!ctx->rust_rx_stop) {
                ctx->peer_closed = true;
                ctx->state = STCP_CLOSED;
            }
            k_sem_give(&ctx->rust_event);
            break;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            k_sleep(K_MSEC(1));
            continue;
        }
        if (!ctx->rust_rx_stop) {
            ctx->last_error = errno != 0 ? errno : EIO;
            ctx->state = STCP_CLOSED;
            LOG_ERR("carrier recv failed errno=%d", errno);
        }
        k_sem_give(&ctx->rust_event);
        break;
    }
    log_current_stack("thread-exit");
    k_free(buffer);
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
