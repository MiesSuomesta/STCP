	/*
	* STCP socket dispatcher for Zephyr / NCS 2.9 (Zephyr 3.7)
	*
	* Transparent protocol hook:
	*   STCP_SOCKET_OPEN(..., IPPROTO_STCP) -> stcp_socket()
	*
	* STCP rides on TCP underneath (for now)
	*/

	#include <zephyr/logging/log.h>

	#include <zephyr/kernel.h>
	#include <zephyr/net/socket.h>
	#include <zephyr/net/socket_offload.h>
	#include <zephyr/sys/fdtable.h>
	#include <zephyr/sys/util.h>

	#include <errno.h>

	#include <stcp/debug.h>
	#include <stcp/stcp_api_internal.h>
	#include <stcp/stcp_struct.h>
	#include <stcp/fsm.h>
	#include <stcp/utils.h>
	#include <stcp/stcp_rust_exported_functions.h>
	#include <stcp/stcp_alloc.h>
	#include <stcp/dns.h>
	#include <stcp/low_level_pointer.h>

	#include <stcp/low_level_refcount_tracker.h>

	#ifndef IPPROTO_STCP
	#define IPPROTO_STCP 253
	#endif

	/* -------------------------------------------------------------------------- */
	/* Missing from socket API: Connect call                                      */
	/* -------------------------------------------------------------------------- */

	int stcp_wait_for_hanshake_done(struct stcp_ctx *ctx, int timeout_ms) {
		struct stcp_api *api = GET_API_FROM_CTX(ctx);

		if (!api) {
			LWRN("No API in context %p to set connected!", ctx);
		} else {
			int conn = atomic_get(&api->connected);
			if (conn) {
				LDBG("Connection got already without need to wait");
				return 0;
			} else {
				int rc = k_sem_take(&api->connected_sem, K_MSEC(timeout_ms));
				LDBG("API connected sema waited, ret: %d", rc);
				return rc;
			}
		}

	}

	int stcp_handshake_for_context(struct stcp_ctx *ctx)
	{

		LDBG("Starting handshake for LTE %d / %p", ctx->ks.fd, ctx);

		LDBG("======================================================================================");
		LDBG("=== HS Start for LTE =================================================================");
		LDBG("======================================================================================");
		LDBG("Context at %s: %p session: %p // HS Done: %d // FD: %d",
			__func__, ctx, ctx->session, ctx->handshake_done, ctx->ks.fd
		);

		if (ctx->handshake_done) {
			LWRN("Handshake for context %p already done hanshake, returning EALREADY", ctx);
			return -EALREADY;
		}

		STCP_REF_COUNT_GET(ctx, "@ handshake", return -EACCES; );
		CONTEXT_LOCK(ctx);
			int bypass = stcp_crypto_is_aes_bypass_enabled();
			LDBG("Creating STCP session for context %p, AES Bypass: %d", ctx, bypass);
			int rc = rust_exported_session_create(&(ctx->session), &(ctx->ks), bypass);
			LDBGBIG("[HS/session] rc=%d errno=%d fd=%d", rc, errno, ctx->ks.fd);

			if (rc < 0) {
				LERR("STCP: No mana?, session not created...");
				CONTEXT_UNLOCK(ctx);
				STCP_REF_COUNT_PUT(ctx, "@ handshake");
				rc = -ENOMEM;
				return rc;
			}
			LDBG("STCP: Setting STCP CTX %p as bloking", ctx);
			stcp_set_non_bloking_to(ctx, 0);

			if (rc == 0) { 
				rc = rust_session_handshake_lte(ctx->session, &(ctx->ks));
				LDBGBIG("[HS/lte] rc=%d errno=%d fd=%d", rc, errno, ctx->ks.fd);
				ctx->handshake_done = rc == 1;
			}

			if (ctx->handshake_done) {
				LDBGBIG("[HS/lte] Marking context %p connection via FD %d as CONNECTED", ctx, ctx->ks.fd);
				struct stcp_api *api = GET_API_FROM_CTX(ctx);
				if (!api) {
					LWRN("[HS/lte] No API in context %p to set connected!", ctx);
				} else {
					stcp_api_connection_set_as_connected(api);
				}
			}

			LDBG("STCP: Setting STCP CTX %p as non bloking", ctx);
			stcp_set_non_bloking_to(ctx, 1);

			LDBGBIG("Context at %s: %p // HS Done: %d // FD: %d",
				__func__, ctx, ctx->handshake_done, ctx->ks.fd
			);

			LDBG("======================================================================================");
			LDBG("======================================================================================");
			LDBGBIG("=== HS END for LTE %d / %p DONE, RC: %d / %d", ctx->ks.fd, ctx, rc, errno);
			LDBG("======================================================================================");
			LDBG("======================================================================================");

		CONTEXT_UNLOCK(ctx);

		if (rc < 0) {
			LINF("Resetting everything cause of failure");
			rust_session_reset_everything_now(ctx->session);
			LERRBIG("Closing socket, cause session create failure!");
			stcp_close(ctx);
			LERR("Handshake error: %d", rc);
		}

		STCP_REF_COUNT_PUT(ctx, "@ handshake");
		return rc;
	}

	int stcp_net_close_fd(int *fd)
	{
		if (*fd < 0) {
			LWRN("close skipped, fd already invalid");
			return 0;
		}

		int old = *fd;
		*fd = -1; 

		int linger = 0;
		LDBG("Setting linger OFF for fd: %d", old);
		zsock_setsockopt(old, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
		
		int rc = STCP_SOCKET_CLOSE(old);
		LINF("close(fd=%d) rc=%d but returning 0", old, rc);

		return 0; // Oli rc
	}

	int stcp_wait_for_handshake_signal(struct stcp_ctx *ctx) {

		if (!ctx) {
			return -EBADFD;
		}

		struct k_poll_event events[1];

		k_poll_event_init(&events[0],
			K_POLL_TYPE_SIGNAL,
			K_POLL_MODE_NOTIFY_ONLY,
			&ctx->handshake_signal);

		LINF("[CTX %p] Waiting for hanshake signal..", ctx);
		int ret = k_poll(events, 1, K_FOREVER);
		LINF("[CTX %p] Got hanshake signal, ret: %d", ctx, ret);
		k_poll_signal_reset(&ctx->handshake_signal);
		return ret;
	}

	void stcp_create_soft_reset_context(struct stcp_ctx *ctx) {
		if (!ctx) {
			LERR("[SOFT INIT] No context");
			return;
		}
		// FSM:lle .. EI saa memsettailla mitään jne.

		ctx->ks.fd = -1;
		ctx->ks.kctx = ctx;
		ctx->doing_replace = 0;
		ctx->handshake_done = 0;
		ctx->poll_timeouts = 0;
		ctx->state = STCP_STATE_INIT;

		atomic_set(&ctx->closing, 0);
		atomic_set(&ctx->cleanup_running, 0);
		atomic_set(&ctx->cleanup_is_rescheduled, 0);
		atomic_set(&ctx->cleanup_work_owns_ref, 0);

		atomic_set(&ctx->connection_closed, 0);
		atomic_set(&ctx->allow_api_access, 0);
		atomic_set(&ctx->destroyed, 0);
		atomic_set(&ctx->connected, 0);
		

	#if CONFIG_STCP_TESTING
		char *pHostName = CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT;
		int pHostPort = CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT;
	#else
		char *pHostName = CONFIG_STCP_CONNECT_TO_HOST;
		int pHostPort = CONFIG_STCP_CONNECT_TO_PORT;
	#endif

		LINF("[SOFT INIT] Set context %p target %s:%d", ctx, pHostName, pHostPort);
		stcp_context_set_target(ctx, pHostName, pHostPort);

		stcp_context_recv_stream_init(ctx);
		LDBG("[SOFT INIT] Api init done.");
	}

	void stcp_create_init_new_context(struct stcp_ctx *ctx) {
		if (!ctx) {
			LERR("[INIT] No context");
			return;
		}

		LDBG("[INIT] Doing memset for CTX %p", ctx);
		memset(ctx, 0, sizeof(*ctx));


		LDBG("[INIT] Doing mutex init");
		k_mutex_init(&ctx->lock);

		// Heti ekaksi, reffi
		LDBG("[INIT] Setting ref to 1");
		atomic_set(&ctx->refcnt, 1); // Ottaa defaulttina itselleen referenssin, jos tippuu nollaan niin konteksti kuolee..

		LDBG("[INIT] Doing magic init");
		ctx->magic = STCP_CTX_MAGIC_ALIVE;
		ctx->magic_footer = STCP_CTX_MAGIC_ALIVE_FOOTER;

		// Tärkeitä!
		ctx->ks.fd = -1;
		ctx->ks.kctx = ctx;

		ctx->doing_replace = 0;
		ctx->handshake_done = 0;
		ctx->poll_timeouts = 0;
		ctx->state = STCP_STATE_INIT;
		LDBG("[INIT] resetting atomics...");
		atomic_set(&ctx->closing, 0);
		atomic_set(&ctx->cleanup_running, 0);
		atomic_set(&ctx->cleanup_is_rescheduled, 0);
		atomic_set(&ctx->cleanup_work_owns_ref, 0);
		atomic_set(&ctx->connection_closed, 0);
		atomic_set(&ctx->allow_api_access, 0);
		atomic_set(&ctx->destroyed, 0);
		atomic_set(&ctx->connected, 0);
		atomic_set(&ctx->owns, 1);
		

		k_poll_signal_init(&(ctx->handshake_signal));

	#if CONFIG_STCP_TESTING
		char *pHostName = CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT;
		int pHostPort = CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT;
	#else
		char *pHostName = CONFIG_STCP_CONNECT_TO_HOST;
		int pHostPort = CONFIG_STCP_CONNECT_TO_PORT;
	#endif

		LINF("[INIT] Set context %p target %s:%d", ctx, pHostName, pHostPort);
		stcp_context_set_target(ctx, pHostName, pHostPort);

		stcp_context_recv_stream_init(ctx);

		LDBG("[INIT] Api init done.");
	}

	struct stcp_ctx *stcp_create_new_context(int under) {
		LDBG("Got FD for under: %d", under);
	
		struct stcp_ctx* newCtx = 
			STCP_MEMORY_ALLOC(sizeof(struct stcp_ctx));

		if (newCtx) {
			stcp_create_init_new_context(newCtx);
		} else {
			LERR("STCP: Out of mana!");
			return NULL;
		}

		int bypass = stcp_crypto_is_aes_bypass_enabled();
		LDBG("Creating STCP session for %p, AES Bypass: %d", newCtx, bypass);
		int ret = rust_exported_session_create(&newCtx->session, &newCtx->ks, bypass);

		LDBGBIG("CREATION: Created STCP session for %p: %p", newCtx, newCtx->session);

		if (ret < 0) {
			LDBG("Error while creating STCP session: %d", ret);
			stcp_close(newCtx);
			errno = ENOMEM;
			return NULL;
		}

		//LDBG("New context done: %p", newCtx);
		//stcp_debug_dump_stcp_ctx(newCtx);
		return newCtx;
	}

	