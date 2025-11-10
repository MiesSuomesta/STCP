#include <stcp/kmod.h>
#include <stcp/structures.h>
#include <stcp/lifecycle.h>
#include <stcp/state.h>


/* ihmisluettavat nimet printk:iä varten */
static const char * const phase_names[] = {
    [STCPF_PHASE_INIT]                = "INIT",
    [STCPF_PHASE_IN_PUBLIC_KEY_MODE]  = "IN_PUBLIC_KEY_MODE",
    [STCPF_PHASE_IN_AES_TRAFFIC_MODE] = "IN_AES_TRAFFIC_MODE",
    
    [STCPF_PHASE_GOT_PUBLIC_KEYS]     = "GOT_PUBLIC_KEYS",
    [STCPF_PHASE_GOT_SHARED_SECRET]   = "GOT_SHARED_SECRET",
    [STCPF_PHASE_CONNECTED]           = "CONNECTED",
    [STCPF_PHASE_CLOSING]             = "CLOSING",
    [STCPF_PHASE_CLOSED]              = "CLOSED",
    [STCPF_PHASE_DEAD]                = "DEAD",
};

static const char * const state_names[] = {
    [STCPF_STATE_INIT]            = "INIT",
    [STCPF_STATE_INNER_CREATING]  = "INNER_CREATING",
    [STCPF_STATE_READY]           = "READY",
    [STCPF_STATE_CLOSING]         = "CLOSING",
    [STCPF_STATE_CLOSED]          = "CLOSED",
    [STCPF_STATE_DEAD]            = "DEAD",
};

struct stcp_sock_ctx *stcp_sock_ctx_alloc(struct socket *sock)
{
    struct stcp_sock_ctx *theContext;

    theContext = kzalloc(sizeof(*theContext), GFP_KERNEL);
    if (!theContext)
        return NULL;

    theContext->magic = STCP_SOCK_MAGIC;
    theContext->sock  = sock;
    theContext->sk    = sock ? sock->sk : NULL;
    atomic_set(&theContext->refcnt, 1);
    spin_lock_init(&theContext->lock);

    /* oletustilat */
    theContext->current_state = STCPF_STATE_INIT;
    theContext->current_phase = STCPF_PHASE_INIT;

#if WITH_STATE_HISTORY
    __set_bit(STCPF_STATE_INIT, theContext->state_hist);
    __set_bit(STCPF_PHASE_INIT, theContext->phase_hist);
#endif 

    return theContext;
}

void stcp_sock_ctx_get(struct stcp_sock_ctx *theContext)
{
    if (theContext)
        atomic_inc(&theContext->refcnt);
}

void stcp_sock_ctx_put(struct stcp_sock_ctx *theContext)
{
    if (!theContext)
        return;
    if (atomic_dec_and_test(&theContext->refcnt)) {
        if (theContext->inner) {
            /* vain varmistus – varsinainen sulku release:ssa */
            theContext->inner->sock = NULL;
            kfree(theContext->inner);
            theContext->inner = NULL;
        }
        kfree(theContext);
    }
}

void stcp_state_debug_dump(const struct stcp_sock_ctx *theContext, const char *tag)
{

#if WITH_STATE_HISTORY
    u32 s = stcp_sock_ctx_state_get(theContext);
    u32 p = stcp_sock_ctx_phase_get(theContext);

    pr_info("stcp[%p]: %s: state=%u(%s) phase=%u(%s) sk=%p sock=%p inner=%p\n",
            theContext, tag ? tag : "-", s,
            (s < ARRAY_SIZE(state_names) && state_names[s]) ? state_names[s] : "?",
            p,
            (p < ARRAY_SIZE(phase_names) && phase_names[p]) ? phase_names[p] : "?",
            theContext->sk, theContext->sock, theContext->inner ? theContext->inner->sock : NULL);
#else 
    /* tähän debuggi ilman historiaa */
#endif

}

/* convenience: oletusvaiheet */
inline void stcp_ctx_mark_closing(struct stcp_sock_ctx *theContext)
{
    stcp_sock_ctx_state_set(theContext, STCPF_STATE_CLOSING);
    stcp_sock_ctx_phase_set(theContext, STCPF_PHASE_CLOSING);
}

inline void stcp_ctx_mark_closed(struct stcp_sock_ctx *theContext)
{
    stcp_sock_ctx_state_set(theContext, STCPF_STATE_CLOSED);
    stcp_sock_ctx_phase_set(theContext, STCPF_PHASE_CLOSED);
}

#if WITH_STATE_HISTORY
/* testaa onko joskus käyty tietyssä tilassa/fasessa */
inline bool stcp_sock_ctx_state_ever(const struct stcp_sock_ctx *theContext, u32 s)
{
    return (s < STCPF_SOCKET_PHASE_MAX) && test_bit(s, theContext->state_hist);
}
inline bool stcp_sock_ctx_phase_ever(const struct stcp_sock_ctx *theContext, u32 p)
{
    return (p < STCPF_SECURITY_LAYER_PHASE_MAX) && test_bit(p, theContext->phase_hist);
}
#else
inline bool stcp_sock_ctx_state_ever(const struct stcp_sock_ctx *theContext, u32 s) { return 0; }
inline bool stcp_sock_ctx_phase_ever(const struct stcp_sock_ctx *theContext, u32 p) { return 0; }
#endif 

