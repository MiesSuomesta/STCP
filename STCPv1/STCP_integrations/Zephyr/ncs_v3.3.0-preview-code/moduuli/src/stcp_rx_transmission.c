
#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_transport.h>
#include <stcp/fsm.h>
#include <stcp/stcp_rust_exported_functions.h>

#define STCP_TIMEOUT_MAX_LOOPS 5
#define STCP_FRAME_HEADER_MAGIC 0x53544350 // STCP


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

    if (ctx->ks.fd < 0)
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
    
    int ret = 1;
    int keep_polling = 1;
    uint64_t start = k_uptime_get();
    while (keep_polling) {
        int fd = ctx->ks.fd;

        ret = stcp_poll_fd_changes(fd, 1, ZSOCK_POLLIN);

        if (ret == 0) {
            ctx->poll_timeouts++;
            LDBG("Timeout (fd: %d, timeouts: %d)", fd, ctx->poll_timeouts);
            if (ctx->poll_timeouts > STCP_TIMEOUT_MAX_LOOPS) {
                ctx->poll_timeouts = 0;
                LDBG("Exess timeouts, returning EAGAIN");
                return -EAGAIN;
            }
            continue;     // timeout
        }

        if (ret < 0) {

            if (ret == -ECONNRESET) {
                LINF("Connection reset by peer.");
            }

            LERR("Poll error: %d", ret);

            atomic_set(&ctx->connection_closed, 1);
            break;   // 🔴 tärkein muutos
        } else {
            LDBG("Resetting poll timeouts...");
            ctx->poll_timeouts = 0;
        }

        keep_polling = 0; 
    }
    uint64_t end = k_uptime_get();

    LDBG("RX: Polled for %llu ms", end - start);

    if (ret < 0) {
        LERR("Poll while exit with returned: %d / %d", ret, errno);
        return ret;
    }

    ssize_t r = zsock_recv(
        ctx->ks.fd,
        s->buffer + s->len,
        STCP_RECV_STREAM_BUF_SIZE - s->len,
        0
    );

    if (r < 0) {

        // Tärkeitä, älä koske!
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -EAGAIN;
        }

        if (errno == ETIMEDOUT) {
            return -EAGAIN;
        }

        // NYT vasta errno, jos jotain muuta
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
            LERR("RECV: recv_stream_fill ret: %d", r);
            
            if (r < 0) {
                
                if (r == -ETIMEDOUT) {
                    LDBG("Timeout. returnign -EAGAIN...");
                    return -EAGAIN;    
                }

                if (r == -EAGAIN) {
                    LDBG("Got EAGAIN...");
                    return -EAGAIN;
                }

                LDBG("RECV: fill returned %d (errno: %d)", r, errno);
                return r;
            }
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
    int ret = 0;
    int r;

    if (!ctx) {
        return -EBADFD;
    }

    LDBG("Locking context %p ..", ctx);    
    k_mutex_lock(&ctx->lock, K_FOREVER);
    LDBG("Locked context %p ..", ctx);    

    if (! stcp_ctx_ref_count_get(ctx) ) {
        LDBGBIG("Could not get context..");
        ret = -EAGAIN;
        goto out;
    }
    LDBG("Refcount get %p", ctx);

    if (atomic_get(&ctx->connection_closed)) {
        LDBGBIG("Connection marked as closed => returning -ECONNRESET");
        ret = -ECONNRESET;
        goto out;
    }

    r = recv_stream_read(ctx, header, 16);
    LERR("RECV: recv_stream_read header ret: %d", r);
    if (r < 0) {
        LDBG("RECV: stream read returned %d (errno: %d)", r, errno);
        ret = -EAGAIN;
        goto out;
    }

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

    if (magic != STCP_FRAME_HEADER_MAGIC) {
        LERR("STCP desync detected: magic %08x", magic);
        /* stream out-of-sync → reset parser */
        stcp_context_recv_stream_init(ctx);
        ret = -EPROTO;
        goto out;
    }


    if (len > STCP_RECV_FRAME_BUF_SIZE) {
        LDBG("Frame too big: %u", len);
        ret = -EMSGSIZE;
        goto out;
    }

    r = recv_stream_read(ctx, ctx->rx_frame, len);
    LERR("RECV: recv_stream_read frame ret: %d", r);
    if (r < 0) {
        LDBG("RECV: stream read returned %d (errno: %d)", r, errno);
        goto out;
    }
    ctx->rx_frame_len = len;

    ret = len;

    LDBG("STCP frame received: %u bytes", len);

out:
    LDBG("Refcount put %p", ctx);
    stcp_ctx_ref_count_put(ctx);
    LDBG("Unlocking context %p ..", ctx);    
    k_mutex_unlock(&ctx->lock);
    LDBG("Unlocked context %p ..", ctx);    

    return ret;
}
