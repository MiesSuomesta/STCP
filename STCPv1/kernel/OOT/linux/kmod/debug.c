#include <stcp/debug.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>
#include <net/tcp_states.h>

/* Turvallinen lukija: ei kaada vaikka st olisi jo detachattu, mutta huomaa:
 * jos st on oikeasti freed ->_translation_ ei auta. Tämä on debug-käyttöön.
 */
unsigned long stcp_read_ul(const unsigned long *p)
{
    return p ? READ_ONCE(*p) : 0UL;
}


static inline const char *tcp_state_name(int state)
{
    switch (state) {
    case TCP_ESTABLISHED: return "ESTABLISHED";
    case TCP_SYN_SENT:    return "SYN_SENT";
    case TCP_SYN_RECV:    return "SYN_RECV";
    case TCP_FIN_WAIT1:   return "FIN_WAIT1";
    case TCP_FIN_WAIT2:   return "FIN_WAIT2";
    case TCP_TIME_WAIT:   return "TIME_WAIT";
    case TCP_CLOSE:       return "CLOSE";
    case TCP_CLOSE_WAIT:  return "CLOSE_WAIT";
    case TCP_LAST_ACK:    return "LAST_ACK";
    case TCP_LISTEN:      return "LISTEN";
    case TCP_CLOSING:     return "CLOSING";
    case TCP_NEW_SYN_RECV:return "NEW_SYN_RECV";
    default:              return "UNKNOWN";
    }
}

void stcp_log_ctx(const char *tag, const void *st_ptr, const void *sk_ptr)
{
    /* current on aina valid */
    const int pid = task_pid_nr(current);
    const char *comm = current->comm;

    const int irq     = (in_interrupt() > 0);
    const int softirq = (in_softirq() > 0);
    const int irqd    = (irqs_disabled() > 0);
    const int preempt = preempt_count();

    /* Aikaleima */
    u64 ns = ktime_get_ns();

    SDBG("CTX DUMP [%s] t=%llu ns pid=%d comm=%s irq=%d softirq=%d irqs_dis=%d preempt=%d st=%px sk=%px",
        tag, ns, pid, comm, irq, softirq, irqd, preempt, st_ptr, sk_ptr);

    const struct stcp_sock *st = st_ptr;
    const struct sock *sk = sk_ptr;

    if (sk && READ_ONCE(sk->sk_prot) != &tcp_prot) {
        SDBG("WARN: sk=%px sk_prot changed! sk_prot=%px tcp_prot=%px",
             sk, sk->sk_prot, &tcp_prot);
    }
    
#define CHECK_BIT(st, bit) \
 ((st) ? test_bit(STCP_FLAG_HS_STARTED_BIT, &((st)->flags)) : -1)

    SDBG("CTX DUMP [%s] sk=%px st=%px tcp=%d(%s) err=%d shut=%x "
        "HS{st=%d pend=%d q=%d ok=%d fail=%d exit=%d} "
        "IO{int=%d} "
        "WQ{wq=%px pend=%d} "
        "LT{det=%d d_q=%d magic=%x}",
        tag, sk, st,
        sk ? sk->sk_state : -1, sk ? tcp_state_name(sk->sk_state) : "?",
        sk ? sk->sk_err : 0, sk ? sk->sk_shutdown : 0,
        CHECK_BIT(st, STCP_FLAG_HS_STARTED_BIT),
        CHECK_BIT(st, STCP_FLAG_HS_PENDING_BIT),
        CHECK_BIT(st, STCP_FLAG_HS_QUEUED_BIT),
        CHECK_BIT(st, STCP_FLAG_HS_COMPLETE_BIT),
        CHECK_BIT(st, STCP_FLAG_HS_FAILED_BIT),
        CHECK_BIT(st, STCP_FLAG_HS_EXIT_MODE_BIT),
        CHECK_BIT(st, STCP_FLAG_INTERNAL_IO_BIT),
        st ? READ_ONCE(st->the_wq) : NULL,
        st ? delayed_work_pending(&st->handshake_work) : 0,
        CHECK_BIT(st, STCP_FLAG_SOCKET_DETACHED_BIT),
        CHECK_BIT(st, STCP_FLAG_SOCKET_DESTROY_QUEUED_BIT),
        st ? READ_ONCE(st->magic) : 0);
}

/* Ratelimit-versio (hyvä steady/churniin) */
void stcp_log_ctx_rl(const char *tag, const void *st_ptr, const void *sk_ptr)
{
    if (printk_ratelimit())
        stcp_log_ctx(tag, st_ptr, sk_ptr);
}

/* Kun sulla on struct stcp_sock *st ja haluat tulostaa kenttiä */
void stcp_log_st_fields(const char *tag,
                                               const struct stcp_sock *st,
                                               const struct sock *sk)
{
    /* HUOM: nämä READ_ONCE:t olettaa että st-osoitin on valid.
     * Käytä tätä vain kun olet varma ettei st ole freed.
     */
    unsigned long magic = st ? READ_ONCE(st->magic) : 0UL;
    unsigned long flags = st ? READ_ONCE(st->flags) : 0UL;
    unsigned long hs_status = st ? READ_ONCE(st->handshake_status) : 0UL;

    SDBG("[%s] st=%px sk=%px magic=%lx flags=%lx hs_status=%lx state=%d err=%d shutdown=%u",
        tag,
        st, sk,
        magic, flags, hs_status,
        sk ? READ_ONCE(sk->sk_state) : -1,
        sk ? READ_ONCE(sk->sk_err) : 0,
        sk ? READ_ONCE(sk->sk_shutdown) : 0);
}

void stcp_log_st_fields_rl(const char *tag,
                                                  const struct stcp_sock *st,
                                                  const struct sock *sk)
{
    if (printk_ratelimit())
        stcp_log_st_fields(tag, st, sk);
}
