#include <errno.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <modem/lte_lc.h>
#include <modem/pdn.h>

#include <zephyr/net/socket.h>
#include <zephyr/posix/poll.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/net_config.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/conn_mgr_monitor.h>
#include <zephyr/net/conn_mgr_connectivity.h>


#include "debug.h"
#include "stcp_alloc.h"
#include "stcp_struct.h"
#include "stcp_bridge.h"
#include "stcp_net.h"
#include "utils.h"
#include "fsm.h"
#include "workers.h"
#include "stcp_operations_zephyr.h"

#include "stcp_rust_exported_functions.h"

int g_sock = -1;

LOG_MODULE_DECLARE(stcp_lte_module);

// The APN for SIM, consult your ISP
#define STCP_LTE_APN                        "internet"

// Evant mask for l4 events
/* Macros used to subscribe to specific Zephyr NET management events. */
#define L4_EVENT_MASK		(NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

// 5 minutes wait for PDN ready...
#define STCP_WAIT_FOR_DATA_PATH_IN_SECONDS  (60 * 5)

// 30 secs debounce time
#define STCP_PDN_DEBOUNCE_TIME_IN_MS        (30 * 1000)


atomic_t g_pdn_active = ATOMIC_INIT(0);
atomic_t reset_requested = ATOMIC_INIT(0);
static int64_t pdn_last_up_ms;

static struct net_mgmt_event_callback l4_cb;
static struct net_mgmt_event_callback conn_cb;
static K_SEM_DEFINE(network_connected_sem, 0, 1);


static K_SEM_DEFINE(pdn_active_sem, 0, 1);

static K_SEM_DEFINE(update_evt_seen_sem, 0, 1);

extern int stcp_lte_reset_everythign(struct stcp_ctx *ctx);

int stcp_pdn_is_active() {
    return atomic_get(&g_pdn_active) == 1;
}

int stcp_is_reset_requested() {
    int rr = atomic_get(&reset_requested);
    LDBG("Reset requested: %d", rr);
    return rr == 1;
}

void stcp_set_reset_requested() {
    LDBG("Setting reset requested ON");
    atomic_set(&reset_requested, 1);
}

int stcp_transport_soft_reset(void *vpCtx) {
#if 0 // TODO: Tämä rikkoo
      // Vapauttaa jotain jota EI saa!
      // Aktivoi kun haluat debugata paremmin
      // Ennen sitä pidä pois päältä...

    int rc = 0;
    if (g_sock > 0) {
        LDBG("SoftReset: Closing transport ...");
        stcp_transport_close(vpCtx);
    }

    LDBG("SoftReset: Setting transport...");
    rc = stcp_transport_set_connected_fd(0);
    if (rc < 0) {
        LOG_ERR("SoftReset: Error while doing transport set fd, %d", rc);
        return rc;
    }

    LDBG("SoftReset: Initialising transport ...");
    rc = stcp_transport_init();
    if (rc < 0) {
        LOG_ERR("SoftReset: Error while doing transport init, %d", rc);
        return rc;
    }

    LDBG("SoftReset: Modem setting: Offline...");
    rc = lte_lc_offline();
    if (rc < 0) {
        LOG_ERR("SoftReset: Error while setting offline, %d", rc);
        return rc;
    }

    k_sleep(K_SECONDS(1));
    LDBG("SoftReset: State setting to Normal...");
    rc = lte_lc_normal();
    if (rc < 0) {
        LOG_ERR("SoftReset: Error while setting NORMAL state, %d", rc);
        return rc;
    }

    LDBG(".========================================================");
    LDBG("| SoftReset: State: Normal...");
    LDBG("'========================================================");
#endif
    return 0;
}

static void l4_event_handler(struct net_mgmt_event_callback *cb,
			     uint32_t event,
			     struct net_if *iface)
{
	switch (event) {
        case NET_EVENT_L4_CONNECTED:
            LDBG("Network connected");
            k_sem_give(&network_connected_sem);
            break;

        case NET_EVENT_L4_DISCONNECTED:
            LDBG("Network disconnected.");
            stcp_on_disconnect(NULL);
            break;

        default:
            /* Don't care */
            return;
	}
}


int stcp_transport_wait_for_network_up(int seconds)
{
   // LDBG("Waiting network bring up (%d sec max)...", seconds);
//
   // int rc = k_sem_take(&network_connected_sem, K_SECONDS(seconds));
   // if (rc < 0) {
   //     LOG_ERR("Network never became up!");
   //     stcp_lte_reset_everythign(NULL);
   //     return rc;
   // }
    return 0;
}

int stcp_transport_wait_for_data_path(int seconds)
{
    LDBG("Waiting PDN data path (%d sec max)...", seconds);

    if (atomic_get(&g_pdn_active)) {
        LDBG("PDN already active");
        return 0;
    }

    int rc = k_sem_take(&pdn_active_sem, K_SECONDS(seconds));
    if (rc < 0) {
        LOG_ERR("PDN never became active!");
        stcp_reset_everything();
        return rc;
    }

    return 0;
}

static void pdn_event_handler(uint8_t cid, enum pdn_event event, int reason)
{
    LDBG("PDN EVT: cid=%d event=%d reason=%d", cid, event, reason);

    if (event == PDN_EVENT_ACTIVATED) {
        if (!atomic_get(&g_pdn_active)) {
            LDBG("PDN is ACTIVE -> data path ready");
            atomic_set(&g_pdn_active, 1);
            k_sem_give(&pdn_active_sem);

            stcp_platform_mark_ready();

            pdn_last_up_ms = k_uptime_get();

            LDBG("STCP: Notifying: PDN ready...");
            stcp_fsm_reached_pnd_ready(&theFSM);

        }
    }

    if (event == PDN_EVENT_DEACTIVATED) {
        LDBG("PDN deactivate event...");
        if (atomic_get(&g_pdn_active)) {
            LDBG("PDN deactivate event while PDN active!");

            int64_t now = k_uptime_get();
            int64_t diff = now - pdn_last_up_ms;

            if (diff < STCP_PDN_DEBOUNCE_TIME_IN_MS) {
                LDBG("PDN deactivate ignored..");
                return;
            }

            LDBG("PDN Deactivate: Setting reset requested...");
            stcp_set_reset_requested();
        }
    }
}

static void on_platform_ready(void)
{
    printk("STCP platform ready starts");
    stcp_transport_init();
    stcp_transport_connect();
    printk("STCP platform ready DONE");
}

int stcp_pdn_wait_until_active_or_secs_passed(int seconds)
{
    LDBG("Waiting for PDN semaphore (pdn_active) %d seconds MAX.....", seconds);
    return k_sem_take(&pdn_active_sem, K_SECONDS(seconds));
}


int stcp_update_cell_event_wait_until_seen_or_secs_passed(int seconds)
{
    LDBG("Waiting for LTE update cell semaphore (update_evt_seen_sem) %d seconds MAX.....", seconds);
    return k_sem_take(&update_evt_seen_sem, K_SECONDS(seconds));
}


static void lte_event_handler(const struct lte_lc_evt *evt)
{
    LDBG("LTE EVT: type=%d nw_reg_status=%d", evt->type, evt->nw_reg_status);

	switch (evt->type) {
		case LTE_LC_EVT_NW_REG_STATUS:
			LDBG("LTE NW REG status: %d", evt->nw_reg_status);
			break;

		case LTE_LC_EVT_CELL_UPDATE:
			LDBG("LTE cell update: id=%d tac=%d",
					evt->cell.id, evt->cell.tac);
            int id = evt->cell.id;
            int tac = evt->cell.tac;
            if ((id > 0) && (tac > 0)) {
                k_sem_give(&update_evt_seen_sem);
                LDBG("Update sema given....");
            }
			break;

        case LTE_LC_NW_REG_SEARCHING: 				
            LDBG("LTE_LC_NW_REG_SEARCHING event..");
            break;

        case LTE_LC_NW_REG_REGISTRATION_DENIED:	
            LDBG("LTE_LC_NW_REG_REGISTRATION_DENIED event..");
            break;

        case LTE_LC_NW_REG_REGISTERED_ROAMING:
        	LDBG("LTE_LC_NW_REG_REGISTERED_ROAMING event..Sema given");
            LDBG("STCP: Notifying: LTE ready...");
            stcp_fsm_reached_lte_ready(&theFSM);
            break;

        case LTE_LC_NW_REG_REGISTERED_HOME:
        	LDBG("LTE_LC_NW_REG_REGISTERED_HOME event..Sema given");
            LDBG("STCP: Notifying: LTE ready...");
            stcp_fsm_reached_lte_ready(&theFSM);
            break;


        case LTE_LC_NW_REG_UICC_FAIL:
            LDBG("LTE_LC_NW_REG_UICC_FAIL event..");
            break;

		default:
			LDBG("LTE Event witout case...");
			break;
    }
}

int stcp_transport_set_connected_fd(int fd) {

    LDBG("Setting transport FD to: %d", fd);
    if (fd < 0) {
        return -EINVAL;
    }

    g_sock = fd;
    return 0;
}

static void dump_sim_status_of_command(const char *cmd) {
    char buf[1280] = { 0 };
    int err = nrf_modem_at_cmd(buf, sizeof(buf), cmd);
    LDBG("%s rc=%d resp=%s", cmd, err, buf);
}

static void dump_sim_status(void)
{
    dump_sim_status_of_command("AT+CPIN?");
    dump_sim_status_of_command("AT+CEREG?");
    dump_sim_status_of_command("AT+CGATT?");
    dump_sim_status_of_command("AT+COPS?");
    dump_sim_status_of_command("AT+CSQ");
}


static int stcp_lte_setup_apn(const char *apn)
{
    int cid = 0;
    int err = 0;

    err = pdn_default_ctx_cb_reg(pdn_event_handler);
    LOG_INF("PDN context register evant handler, RC: %d", err);
    if (err < 0) {
        LOG_ERR("PDN context register evant handler FAILED");
        return err;
    }

    err = pdn_ctx_create(&cid, NULL);
    if (err < 0) {
        LOG_ERR("pdn_ctx_create failed: %d", err);
        return err;
    }

    LDBG("Got CID for APN: %s, CID=%d", apn, cid);

    err = pdn_ctx_configure(cid, apn, PDN_FAM_IPV4, NULL);
    if (err < 0) {
        LOG_ERR("pdn_ctx_configure %s failed: %d", apn, err);
        return err;
    }

    LOG_INF("PDN context created with APN=%s (cid=%d)", apn, cid);
    return 0;
}

int stcp_poll_fd_changes(int fd, int timeout, int events) {

    struct pollfd pfd = {
        .fd = fd,
        .events = events,
    };

    int rc = zsock_poll(&pfd, 1, timeout);  // 10s timeout

    if (rc < 0) {
        LERR("Poll (events: %d) exit: %d", events, rc);
        return -ETIMEDOUT;
    }

    return rc;
}

void stcp_on_disconnect(struct stcp_ctx *ctx)
{
    LDBG("On disconnect called....");
    if (!ctx)  { return; }
    ctx->handshake_done = false;
    stcp_lte_reset_everythign(ctx);
}

int stcp_transport_init(void)
{
    static int onceCalled = 0;

    k_sem_reset(&pdn_active_sem);
    k_sem_reset(&network_connected_sem);

    atomic_set(&g_pdn_active, 0);

    if (!onceCalled) {
        onceCalled = 1;

        LDBG("Registering LTE event handler ...");
        lte_lc_register_handler(lte_event_handler);

        LDBG("Initialising modem lib...");
        int err = nrf_modem_lib_init();
        LDBG("Modem lib init done, RC: %d", err);
        if (err < 0) {
            LOG_ERR("Modem init failed!");
            k_panic();
        }

        //LDBG("Setting LTE system mode  ...");
        //lte_lc_system_mode_set(LTE_LC_SYSTEM_MODE_LTEM, LTE_LC_SYSTEM_MODE_PREFER_AUTO);
        //k_sleep(K_SECONDS(1));

        LDBG("Registering APN");
        err = stcp_lte_setup_apn(STCP_LTE_APN);
        if (err < 0) {
            LOG_ERR("APN setting failed!, RC: %d", err);
            k_panic();
        }
    } else {
        LDBG("Already called once..");
    }

    LDBG("Initialising LTE to normal state...");
    int err = lte_lc_normal();
    LDBG("LTE to normal-state done, RC: %d", err);
    if (err < 0) {
        LOG_ERR("LTE normal state failed!");
        return err;
    }

    LDBG("SIM Dump:");
    dump_sim_status();

    //LDBG("Wait until SIM is registered, or 180 sec..");
    //stcp_registering_complete_wait_until_seen_or_secs_passed(180);

	/* Setup handler for Zephyr NET Connection Manager events. */
	net_mgmt_init_event_callback(&l4_cb, l4_event_handler, L4_EVENT_MASK);
	net_mgmt_add_event_callback(&l4_cb);

    return 0;
}

int stcp_transport_connect(void)
{
	int ret = conn_mgr_all_if_up(true);
	if (ret) {
		LOG_ERR("conn_mgr_all_if_up, error: %d", ret);
		return ret;
	}

  	ret = conn_mgr_all_if_connect(true);
	if (ret) {
		LOG_ERR("conn_mgr_all_if_connect, error: %d", ret);
		return ret;
	}

    //LDBG("Waiting for network up ........");
    //k_sem_take(&network_connected_sem, K_FOREVER);

    LDBG("Waiting LTE registration...");
    int rc = stcp_transport_wait_for_data_path(STCP_WAIT_FOR_DATA_PATH_IN_SECONDS);
    if (rc < 0) {
        LOG_ERR("PDN data path failed");
        return rc;
    }
    
    LDBG("LTE data path ready");

    dump_sim_status();
    return 0;
}

int stcp_transport_send(struct stcp_ctx *ctx, const uint8_t *buf, size_t len)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LDBG("Called RUST send message, without context...");
        return -EBADFD;
    }
    
    if (!buf) {
        LDBG("Called RUST send message, without buffer...");
        return -EBADFD;
    }

    if (len < 1) {
        LDBG("Called RUST send message, without lenght...");
        return -EBADFD;
    }

    LDBG("Sending via rust %d bytes: %s", len, buf);
    int ret = rust_exported_session_sendmsg(ctx->session, &ctx->ks, buf, len);
    LDBG("Called RUST send message, rc: %d", ret);
    return ret;
}

int stcp_transport_recv(struct stcp_ctx *ctx, uint8_t *buf, size_t maxlen)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LDBG("Called RUST recv message, without context...");
        return -EBADFD;
    }
    
    if (!buf) {
        LDBG("Called RUST recv message, without buffer...");
        return -EBADFD;
    }

    if (maxlen < 1) {
        LDBG("Called RUST recv message, without lenght...");
        return -EBADFD;
    }
    int ret = rust_exported_session_recvmsg(ctx->session, &ctx->ks, buf, maxlen, /* no blocking */ 1 );
    LDBG("Called RUST recv message, rc: %d", ret);
    return ret;
}

void stcp_transport_close(void *vpCtx)
{
    LDBG("Transport closing...(NOP)");
    //if (stcp_is_context_valid(vpCtx) < 0) {
    //    LDBG("Scheduling destroy....");
    //    worker_schedule_cleanup((struct stcp_ctx *)vpCtx);
    //}
}

int stcp_transport_wait_until_ready(int seconds)
{
    return stcp_platform_wait_until_platform_ready(seconds);
}
