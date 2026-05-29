#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_transport.h>
#include <stcp/stcp_rust_exported_functions.h>

#define STCP_FRAME_HEADER_MAGIC 0x53544350 // STCP

static inline int stcp_retryable(int rc)
{
    return
        rc == -EAGAIN ||
        rc == -EWOULDBLOCK ||
        rc == -ETIMEDOUT;
}

int stcp_context_recv_stream_init(struct stcp_ctx *ctx)
{
    if (!ctx)
        return -EINVAL;

    ctx->rx_stream.buffer = ctx->rx_stream_buffer;
    ctx->rx_stream.pos = 0;
    ctx->rx_stream.len = 0;

    ctx->rx_frame_len = 0;

    ctx->rx_payload_pos = 0;
    ctx->rx_payload_len = 0;

    return 0;
}

static int recv_stream_fill(struct stcp_ctx *ctx)
{
    if (!ctx)
        return -EINVAL;

    if (!stcp_is_context_valid(ctx))
        return -ECONNRESET;

    struct stcp_recv_stream *s = &ctx->rx_stream;

    if (!s->buffer)
        return -EINVAL;

    int fd = ctx->ks.fd;

    if (fd < 0)
        return -EBADF;

    /*
     * Compact stream buffer.
     */
    if (s->pos > 0) {

        if (s->pos < s->len) {

            size_t remain = s->len - s->pos;

            memmove(
                s->buffer,
                s->buffer + s->pos,
                remain
            );

            s->len = remain;

        } else {

            s->len = 0;
        }

        s->pos = 0;
    }

    if (s->len >= STCP_RECV_STREAM_BUF_SIZE)
        return -ENOBUFS;

    ssize_t rc = zsock_recv(
        fd,
        s->buffer + s->len,
        STCP_RECV_STREAM_BUF_SIZE - s->len,
        ZSOCK_MSG_DONTWAIT
    );

    if (rc == 0)
        return -ECONNRESET;

    if (rc < 0) {

        rc = -errno;

        if (stcp_retryable(rc))
            return -EAGAIN;

        return rc;
    }

    s->len += rc;

    return rc;
}

static int recv_stream_read(
    struct stcp_ctx *ctx,
    uint8_t *dst,
    size_t wanted)
{
    if (!ctx || !dst || wanted == 0)
        return -EINVAL;

    struct stcp_recv_stream *s = &ctx->rx_stream;

    size_t copied = 0;

    while (copied < wanted) {

        if (s->pos == s->len) {

            int rc = recv_stream_fill(ctx);

            if (rc < 0)
                return rc;
        }

        size_t available =
            s->len - s->pos;

        if (available == 0)
            continue;

        size_t take =
            MIN(
                available,
                wanted - copied
            );

        memcpy(
            dst + copied,
            s->buffer + s->pos,
            take
        );

        s->pos += take;
        copied += take;
    }

    return copied;
}

int stcp_recv_frame(struct stcp_ctx *ctx)
{
    if (!ctx)
        return -EINVAL;

    if (atomic_get(&ctx->connection_closed))
        return -ECONNRESET;

    uint8_t header[16];

    int rc = recv_stream_read(
        ctx,
        header,
        sizeof(header)
    );

    if (rc < 0)
        return rc;

    uint32_t magic =
        ((uint32_t)header[4] << 24) |
        ((uint32_t)header[5] << 16) |
        ((uint32_t)header[6] << 8)  |
        ((uint32_t)header[7]);

    uint32_t len =
        ((uint32_t)header[12] << 24) |
        ((uint32_t)header[13] << 16) |
        ((uint32_t)header[14] << 8)  |
        ((uint32_t)header[15]);

    if (magic != STCP_FRAME_HEADER_MAGIC) {

        LERR(
            "STCP frame desync magic=0x%x",
            magic
        );

        stcp_context_recv_stream_init(ctx);

        return -EPROTO;
    }

    if (len == 0 ||
        len > STCP_RECV_FRAME_BUF_SIZE)
    {
        LERR(
            "Invalid STCP frame len=%u",
            len
        );

        stcp_context_recv_stream_init(ctx);

        return -EMSGSIZE;
    }

    rc = recv_stream_read(
        ctx,
        ctx->rx_frame,
        len
    );

    if (rc < 0)
        return rc;

    ctx->rx_frame_len = len;

    return len;
}