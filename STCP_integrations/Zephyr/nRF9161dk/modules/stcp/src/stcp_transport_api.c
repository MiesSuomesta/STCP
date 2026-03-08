#include <zephyr/kernel.h>
#include <stdlib.h>

#define STCP_SOCKET_INTERNAL 1
#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>

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

ssize_t stcp_api_send(struct stcp_api *api,
                       const void *buf,
                       size_t len,
                       int flags)
{
    return stcp_send(api->ctx, buf, len, flags);
}

ssize_t stcp_api_recv(struct stcp_api *api,
                      void *buf,
                      size_t len,
                      int flags
                    )
{

    flags |= api->nonblocking ? ZSOCK_MSG_DONTWAIT : 0;
    return stcp_recv(api->ctx, buf, len, flags);
}

int stcp_api_close(struct stcp_api *api)
{
    int ret = stcp_close(api->ctx);
    k_free(api);
    return ret;
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

    return -1;
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

