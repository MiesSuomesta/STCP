#include <zephyr/kernel.h>
#include <stdlib.h>

#define STCP_SOCKET_INTERNAL 1
#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_lte.h>


#define STCP_RESET_WAIT_SERVICES_FOR_SECONDS (5*60)

int stcp_api_init(struct stcp_api **api)
{
    if (!api) {
        return -EINVAL;
    }

    struct stcp_api *inst = k_malloc(sizeof(*inst));
    if (!inst) {
        return -ENOMEM;
    }

    int ret = stcp_new_context(&inst->ctx);
    if (ret < 0) {
        k_free(inst);
        return ret;
    }

    // Tärkeä, referenssiä käytetään cleanupissa!
    inst->ctx->api = inst; 

    inst->nonblocking = 0;

    *api = inst;
    return 0;
}

int stcp_api_init_with_fd(struct stcp_api **api, int fd)
{
    if (!api) {
        return -EINVAL;
    }

    struct stcp_api *inst = k_malloc(sizeof(*inst));
    if (!inst) {
        return -ENOMEM;
    }

    int ret = stcp_new_context_with_fd(&inst->ctx, fd);
    if (ret < 0) {
        k_free(inst);
        return ret;
    }

    inst->nonblocking = 0;
    inst->ctx->ks.fd = fd;
    // Tärkeä, referenssiä käytetään cleanupissa!
    inst->ctx->api = inst; 

    *api = inst;
    return 0;
}

int stcp_api_resolve(const char *host, const char *port, struct zsock_addrinfo **result) {
    return stcp_util_hostname_resolver(host, port, result);
}

int stcp_api_connect(struct stcp_api *api,
                     const struct zsock_addrinfo *addr,
                     socklen_t addrlen)
{
    return stcp_connect(api->ctx, (struct sockaddr *)addr, addrlen);
}

int stcp_api_set_io_timeout(struct stcp_api *api, int timeout_ms)
{
    if (!api) {
        return -EINVAL;
    }
    int fd = stcp_api_get_fd(api);
    LINF("Setting timeout for IO to %d msec", timeout_ms);
    int rv = stcp_tcp_timeout_set_to_fd(fd, timeout_ms);
    LDBG("Setting ret: %d", rv);
    return rv;
}

int stcp_api_connection_reset(struct stcp_api *api)
{
    if (!api) {
        return -EINVAL;
    }
    int ret = -ENOBUFS;

    if (api->ctx) {
        ret = stcp_lte_do_full_reset(api->ctx, 0);

        LDBG("Forcing reattach....");
        (void)stcp_lte_issue_at_command("AT+CFUN=0");
        (void)stcp_lte_issue_at_command("AT+CFUN=1");

        LINFBIG("Full reset completed!");
    }

    return ret;
}

ssize_t stcp_api_send(struct stcp_api *api,
                       const void *buf,
                       size_t len,
                       int flags)
{

    ssize_t ret = 0;

    LDBG("API CALL: SEND");
    ret = stcp_send(api->ctx, buf, len, flags);
    LDBG("API CALL: SEND, ret %d", (int)ret);

    if (ret < 0) {
        switch (ret)
        {
            case -ETIMEDOUT:
                LDBG("Timeout => returning -EAGAIN");
                return -EAGAIN;

            case -ECONNRESET:
            case -EPIPE:
                LDBG("Connection lost => returning -ENOTCONN");
                return -ENOTCONN;

            default:
                if (ret == -1) {
                    LERR("STCP returned -1, mapping it to -EBADMSG");
                    ret = -EBADMSG;
                }
        }
    }
    return ret;
}

ssize_t stcp_api_recv(struct stcp_api *api,
                      void *buf,
                      size_t len,
                      int flags
                    )
{

    ssize_t ret = 0;
    flags |= api->nonblocking ? ZSOCK_MSG_DONTWAIT : 0;
    LDBG("API CALL: RECV");
    ret = stcp_recv(api->ctx, buf, len, flags);
    LDBG("API CALL: RECV, ret: %d", (int) ret );

    if (ret < 0) {
        switch (ret)
        {
            case -ETIMEDOUT:
                LDBG("Timeout => returning -EAGAIN");
                return -EAGAIN;

            case -ECONNRESET:
            case -EPIPE:
                LDBG("Connection lost => returning -ENOTCONN");
                return -ENOTCONN;

            default:
                if (ret == -1) {
                    LERR("STCP returned -1, mapping it to -EBADMSG");
                    ret = -EBADMSG;
                }
        }
    }

    return ret;
}

int stcp_api_close(struct stcp_api *api)
{
    if (!api) {
        return -EBADFD;
    }

    if (!api->ctx) {
        return -EBADFD;
    }

    int ret = stcp_close(api->ctx);
    return ret;
}

int stcp_api_reset(struct stcp_api *api)
{
    if (!api) {
        return -EBADFD;
    }

    if (!api->ctx) {
        return -EBADFD;
    }
    
    stcp_do_full_reset(api->ctx);
    return 0;
}

int stcp_api_accept(struct stcp_api *api,
                    struct stcp_api **new_api,
                    struct zsock_addrinfo *peer,
                    socklen_t *peer_len)
{
    struct stcp_ctx *child = NULL;

    int ret = stcp_accept(api->ctx, &child, (struct sockaddr *)peer, peer_len);
    if (ret < 0) {
        return ret;
    }

    struct stcp_api *inst = k_malloc(sizeof(*inst));
    if (!inst) {
        return -ENOMEM;
    }

    inst->ctx = child;
    *new_api = inst;

    return 0;
}

ssize_t stcp_api_sendmsg(struct stcp_api *api,
                         const struct msghdr *msg)
{
    if (!api || !msg) {
        return -EINVAL;
    }

    return stcp_send_msg(api->ctx, msg);
}


int stcp_api_poll(struct stcp_api *api,
                  int events,
                  int timeout_ms,
                  int *revents)
{
    if (!api || !revents) {
        return -EINVAL;
    }

    int fd = api->ctx->ks.fd;

    struct zsock_pollfd pfd = {
        .fd = fd,
        .events = events,
        .revents = 0,
    };
    
    *revents = 0;
    int ret = zsock_poll(&pfd, 1, timeout_ms);
    if (ret < 0) {
        return ret;   // error
    }

    // Timeout ?

    *revents = pfd.revents;

    return ret;      // number of ready fds (POSIX style)
}

int stcp_api_get_fd(struct stcp_api *api)
{
    if (api && api->ctx)
        return api->ctx->ks.fd;

    return -errno;
}

int stcp_api_set_nonblocking(struct stcp_api *api,
                             bool enable)
{
    if (!api) {
        return -EINVAL;
    }
    api->nonblocking = (enable) ? 1 : 0;
    return stcp_set_non_bloking_to(api->ctx, api->nonblocking);
}

