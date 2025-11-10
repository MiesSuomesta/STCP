#include <net/sock.h>
#include <linux/printk.h>
#include <linux/socket.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/printk.h>
#include <linux/net.h>
#include <linux/kernel.h>  // container_of
#include <net/sock.h>      // struct sock
#include <linux/net.h>     // struct socket
#include <linux/poll.h>  // __poll_t

#include <stcp/kmod.h>
#include <stcp/lifecycle.h>
#include <stcp/helpers.h>

int stcp_release(struct socket *sock)
{
	struct sock *sk = sock ? sock->sk : NULL;
	struct stcp_state *st;

	pr_debug("stcp: release: enter\n");
	if (!sk)
		return 0;

	lock_sock(sk);

	st = (struct stcp_state *)READ_ONCE(sk->sk_user_data);

	if (!st) {
		pr_debug("stcp: release: no user_data -> skip protocol cleanup\n");
		release_sock(sk);
		/* anna inet_release hoitaa resurssit */
		return inet_release(sock);
	}

	if (st->cleanup_done) {
		pr_debug("stcp: release: already cleaned -> pass to inet_release\n");
		release_sock(sk);
		return inet_release(sock);
	}

	st->state = STCPF_STATE_CLOSING;

	/* siivoa oma sisäinen wrap (turvallinen, idempotentti) */
	stcp_state_free_inner(st);

	/* irrota tila soketista, ettei tule jatkokutsuja meidän tilaan */
	stcp_state_detach_state(sk);

	st->state = STCPF_STATE_CLOSED;
	st->cleanup_done = true;

	release_sock(sk);

	/* vapauta lopuksi oma st (ei enää kenenkään saavutettavissa) */
	kfree(st);
    st = NULL;

	pr_debug("stcp: release: done -> inet_release\n");
    int ret = inet_release(sock);    
    pr_debug("stcp: release: Going out: %d\n", ret);
    return ret;
}

int stcp_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_bind(sock, uaddr, addr_len);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif

    return stcp_inner_bind(st, uaddr, addr_len);
}


int stcp_connect(struct socket *sock, struct sockaddr *addr,
                 int addr_len, int flags)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }
    pr_debug("stcp: connect start...\n");

    r = stcp_rust_connect(sock, addr, addr_len, flags);
    if (r == -ENOSYS) {
        pr_debug("stcp: rust imp returned -ENOSYS\n");
    } else {
        pr_debug("stcp: rust imp handled ok? %d\n", r);
        return r;
    }

	/* nyt st on varma ja inner & phase olemassa → voit päivittää phasea */
    stcp_sock_state_set(st, STCPF_STATE_READY);

    // Liput päälle että initioi publickey moodiin ...
    stcp_sock_phase_set(st, STCPF_PHASE_INIT | STCPF_PHASE_IN_PUBLIC_KEY_MODE);

    pr_debug("stcp: %s: st=%p phase=%p lock=%p\n",
         __func__, st, st ? st->phase : NULL,
         (st && st->phase) ? &st->phase->phase_lock : NULL);


    pr_debug("stcp: Calling inner connect...\n");
    int ret = stcp_inner_connect(st, addr, addr_len, flags);
    pr_debug("stcp: Called inner connect: %d\n", ret);

    return ret;
}

void stcp_close(struct sock *sk, long timeout)
{
	struct stcp_state *st;
    
	if (!sk) return;
    
    // Kutsutaan ilman lukkoja!
    stcp_rust_close(sk, timeout);

	lock_sock(sk);
	st = (struct stcp_state *)READ_ONCE(sk->sk_user_data);


	/* Jos release teki siivouksen, ei tehdä mitään. */
	if (!st || st->cleanup_done) {
		release_sock(sk);
		return;
	}

	/* Fallback: harvinainen reitti, siivoa varovasti */
	st->state = STCPF_STATE_CLOSING;

    stcp_state_free_inner(st);
	stcp_state_detach_state(sk);

    struct stcp_sock *p = stcp_from_sk(sk);
    if (p) {
        // RUST huomio: RUSt vapauttaa itse datansa
        // Mutta kerneli puolen lukko pitää tässä kohden
        // vapauttaa.
        spin_unlock(&p->rust_lock);
        p = NULL; // turvallisuus
    }

    st->state = STCPF_STATE_CLOSED;
	st->cleanup_done = true;
	release_sock(sk);
	kfree(st);
}


int stcp_listen(struct socket *sock, int backlog)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_listen(sock, backlog);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif

    return stcp_inner_listen(st, backlog);
}

/* Yhtenäinen accept-glue kaikille kernelihaarille */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)

/* Kernel 6.8+: accept(struct socket *sock, struct socket *newsock, struct proto_accept_arg *arg) */
int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                     struct proto_accept_arg *arg)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    /* Poimi flags/kern arg-rakenteesta */
    int  flags = arg ? arg->flags : 0;
    bool kern  = arg ? arg->kern  : false;

    /* Anna mahdollisuus Rustille hoitaa */
    r = stcp_rust_accept(sock, newsock, flags, kern);
    if (r != -ENOSYS)
        return r;

    /* C-fallback: älä vielä graftaa; pidetään turvallisena */
    {

    
#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif
 
        r = stcp_inner_accept(st, NULL, flags);
        if (r) return r;

        pr_info("stcp: accept fallback (>=6.8), pending inner=%px\n", st->accept_pending);
        return -EOPNOTSUPP;
    }
}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0)

/* Kernel 5.9+ .. <6.8: accept(struct socket *sock, struct socket *newsock, int flags, bool kern) */
int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                     int flags, bool kern)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_accept(sock, newsock, flags, kern);
    if (r != -ENOSYS)
        return r;

    /* C-fallback */
    {
    
#ifdef USE_ENSURES_FROM_OLD_CODE
        STCP_GET_ST_OR_RET(sock);
        STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
        struct stcp_sock *st = stcp_from_socket(sock);
        if (!st) return -EINVAL;
#endif

        r = stcp_inner_accept(st, NULL, flags);
        if (r) return r;

        pr_info("stcp: accept fallback (>=5.9,<6.8), pending inner=%px\n", st->accept_pending);
        return -EOPNOTSUPP;
    }
}

#else

/* Vanhat puut: accept(struct socket *sock, struct socket *newsock, int flags) — ei kern-paramia */
int stcp_accept_glue(struct socket *sock, struct socket *newsock,
                     int flags)
{

    bool kern = false; /* ei paramia → oletus */
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_accept(sock, newsock, flags, kern);
    if (r != -ENOSYS)
        return r;


    /* C-fallback */
    {
#ifdef USE_ENSURES_FROM_OLD_CODE
        STCP_GET_ST_OR_RET(sock);
        STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
        struct stcp_sock *st = stcp_from_socket(sock);
        if (!st) return -EINVAL;
#endif
        r = stcp_inner_accept(st, NULL, flags);
        if (r) return r;

        pr_info("stcp: accept fallback (<5.9), pending inner=%px\n", st->accept_pending);
        return -EOPNOTSUPP;
    }
}

#endif /* version split */
 // ACCEPTi ideffit loppuu


int stcp_sendmsg(struct socket *sock, struct msghdr *msg, size_t len)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_sendmsg(sock, msg, len);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif


    return stcp_inner_sendmsg(st, msg, len);
}

int stcp_recvmsg(struct socket *sock, struct msghdr *msg, size_t len, int flags)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_recvmsg(sock, msg, len, flags);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif

    return stcp_inner_recvmsg(st, msg, len, flags);
}

int stcp_shutdown(struct socket *sock, int how)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_shutdown(sock, how);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif

    return stcp_inner_shutdown(st, how);
}

int stcp_getname(struct socket *sock, struct sockaddr *uaddr, int peer)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    int addrlen = 0;
    r = stcp_rust_getname(sock, uaddr, &addrlen, peer);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif

    return stcp_inner_getname(st, uaddr, &addrlen, peer);
}

int stcp_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_ioctl(sock, cmd, arg);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif


    return -ENOIOCTLCMD;
}



__poll_t stcp_poll(struct file *file, struct socket *sock, poll_table *wait)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_poll(file, sock, wait);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif


    return 0; // tai stcp_inner_poll(st, file, wait) jos toteutettu
}

int stcp_setsockopt(struct socket *sock, int level, int optname, sockptr_t optval, unsigned int optlen)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }

    r = stcp_rust_setsockopt(sock, level, optname, optval, optlen);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif

    return -EOPNOTSUPP;
}

int stcp_getsockopt(struct socket *sock, int level, int optname, char __user *optval, int __user *optlen)
{
    int r = -EINVAL;
    struct stcp_sock *st = NULL;

    // Ensure hoitaa homman kotia 
    r = stcp_ensure_all_ok(sock, &st, __func__);

    if (r) {
        pr_err("stcp: ensure failed\n");
        return r;
    }
    
    r = stcp_rust_getsockopt(sock, level, optname, optval, optlen);
    if (r != -ENOSYS) return r;

#ifdef USE_ENSURES_FROM_OLD_CODE
    STCP_GET_ST_OR_RET(sock);
    STCP_ENSURE_INNER_OR_RET(sock);          // <-- tämä luo innerin tarvittaessa
    struct stcp_sock *st = stcp_from_socket(sock);
#endif

    return -EOPNOTSUPP;
}
