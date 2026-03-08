
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stcp_rx_transmission, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/fsm.h>
#include <stcp/stcp_rust_exported_functions.h>

void stcp_context_recv_stream_init(struct stcp_ctx *ctx)
{
	LDBG("Initialising stream for %p..", ctx);
	if (!ctx) return;

    ctx->rx_stream.buffer = ctx->rx_stream_buffer;
    ctx->rx_stream.pos = 0;
    ctx->rx_stream.len = 0;
	ctx->rx_frame_len = 0;

    ctx->rx_payload_pos = 0;
    ctx->rx_payload_len = 0;

	LDBG("Stream buffer %p size %d", ctx->rx_stream_buffer, STCP_RECV_STREAM_BUF_SIZE);
	LDBG("Frame buffer %p size %d", ctx->rx_frame, STCP_RECV_FRAME_BUF_SIZE);
}


static int recv_stream_fill(struct stcp_ctx *ctx)
{
    struct stcp_recv_stream *s = &ctx->rx_stream;

    if (!ctx)
        return -EINVAL;

    if (!ctx->ks.fd)
        return -EBADF;

    /* Jos bufferi on täynnä mutta alkuun on vapautunut tilaa → siirrä */
    if (s->pos > 0) {

        if (s->pos < s->len) {
            memmove(
                s->buffer,
                s->buffer + s->pos,
                s->len - s->pos
            );
        }

        s->len -= s->pos;
        s->pos = 0;
    }

    /* ei enää tilaa */
    if (s->len >= STCP_RECV_STREAM_BUF_SIZE) {
        LDBG("RX stream buffer full");
        return -ENOBUFS;
    }

    ssize_t r = zsock_recv(
        ctx->ks.fd,
        s->buffer + s->len,
        STCP_RECV_STREAM_BUF_SIZE - s->len,
        0
    );

    if (r < 0) {
        return -errno;
    }

    if (r == 0) {
        return -ECONNRESET;
    }

    s->len += r;

    LDBG("RX stream filled: +%d bytes (pos=%d len=%d)", r, s->pos, s->len);

    return r;
}

static int recv_stream_read(
    struct stcp_ctx *ctx,
    uint8_t *dst,
    size_t wanted)
{
    struct stcp_recv_stream *s = &ctx->rx_stream;

    size_t copied = 0;

    while (copied < wanted) {

        if (s->pos == s->len) {

            int r = recv_stream_fill(ctx);
            if (r < 0)
                return r;
        }

        size_t available = s->len - s->pos;
        size_t take = wanted - copied;

        if (take > available)
            take = available;

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
    uint8_t header[16];

    int r;

    r = recv_stream_read(ctx, header, 16);
    if (r < 0)
        return r;

    uint32_t version =
        (header[0] << 24) |
        (header[1] << 16) |
        (header[2] << 8)  |
        (header[3]);

    uint32_t magic =
        (header[4] << 24) |
        (header[5] << 16) |
        (header[6] << 8)  |
        (header[7]);

    uint32_t type =
        (header[8] << 24) |
        (header[9] << 16) |
        (header[10] << 8) |
        (header[11]);

    uint32_t len =
        (header[12] << 24) |
        (header[13] << 16) |
        (header[14] << 8)  |
        (header[15]);

    LDBG("[Ctx %p] Read frame header: VER: %u, Magic:%u, Type:%u, Len: %u",
        ctx, version, magic, type, len);

    if (len > STCP_RECV_FRAME_BUF_SIZE) {
        LDBG("Frame too big: %u", len);
        return -EMSGSIZE;
    }

    r = recv_stream_read(ctx, ctx->rx_frame, len);
    if (r < 0)
        return r;

    ctx->rx_frame_len = len;

    LDBG("STCP frame received: %u bytes", len);

    return len;
}
