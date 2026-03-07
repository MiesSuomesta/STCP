#include <zephyr/net/socket.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>

#include "stcp/debug.h"
#include "stcp/utils.h"
#include "stcp/stcp_struct.h"   // tulee SDK:n include/:sta
#include "stcp/stcp_net.h" 
#include "stcp/utils.h"
#include "stcp/stcp_transport.h"

LOG_MODULE_REGISTER(stcp_net_operations, LOG_LEVEL_INF);

int stcp_net_send(struct stcp_ctx *ctx, const uint8_t *buf, size_t len) {

    if (stcp_is_context_valid(ctx) < 0) {
        return -ENOTCONN;
    }

    if (!ctx->handshake_done) {
        LOG_ERR("STCP NET SEND blocked: handshake not done");
        return -EAGAIN;
    }

    int fd = ctx->ks.fd;

    if (fd < 0) {
        LOG_ERR("STCP NET SEND: No fd");
        return -ENOTCONN;
    }

    void *sess = ctx->session;
    void *trans = &ctx->ks;

    LERR("STCP SEND =================================================");
    ssize_t rc = rust_exported_session_sendmsg(sess, trans, buf, len);
    LERR("STCP SEND DONE =================================================");
    if (rc >= 0) return (int)rc;

    return -errno;
}

int stcp_net_recv(struct stcp_ctx *ctx, uint8_t *buf, size_t max_len, int flags) {

    if (stcp_is_context_valid(ctx) < 0) {
        return -ENOTCONN;
    }

    int fd = ctx->ks.fd;

    if (fd < 0) return -ENOTCONN;

    if (!ctx->handshake_done) {
        LOG_ERR("STCP RECV blocked: handshake not done");
        return -EAGAIN;
    }

    void *sess = ctx->session;
    void *trans = &ctx->ks;
    int ret_len = 0;

    int torc = stcp_tcp_timeout_set_to_fd(ctx->ks.fd, 10*1000);
    LDBG("Calling RUST recv msg, timeout set, rc: %d", torc);
    ssize_t rc = rust_exported_session_recvmsg(sess, trans, buf, max_len, &ret_len);
    LDBG("RECV rc: %d, errno: %d", rc, errno);

    stcp_hexdump_ascii("RX buffer", buf, (size_t)rc);

    if (rc >= 0) return (int)rc;

    return -errno;
}
