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
#include <modem/lte_lc.h>
#include <modem/at_cmd_custom.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_config.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/conn_mgr_monitor.h>
#include <zephyr/net/conn_mgr_connectivity.h>
#include <stcp/stcp_rx_transmission.h>


#define STCP_SOCKET_INTERNAL 1
#include <stcp_api.h>
#include <stcp/fsm.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_transport.h>    
#include <stcp/stcp_platform.h>    

#include "stcp/stcp_rust_exported_functions.h"

// LAtenssi jne. mittailuja 
//#if CONFIG_STCP_DEBUG_LATENCY
//#endif



// Evant mask for l4 events
/* Macros used to subscribe to specific Zephyr NET management events. */
#define L4_EVENT_MASK		(NET_EVENT_L4_CONNECTED | NET_EVENT_L4_DISCONNECTED)

// 5 minutes wait for PDN ready...
#define STCP_WAIT_FOR_DATA_PATH_IN_SECONDS  (60 * 5)

// 30 secs debounce time
#define STCP_PDN_DEBOUNCE_TIME_IN_MS        (30 * 1000)
#define MQTT_CONNECT_TIMEOUT_MS             500


atomic_t g_pdn_active = ATOMIC_INIT(0);
atomic_t reset_requested = ATOMIC_INIT(0);

static struct net_mgmt_event_callback l4_cb;

#if 0
static int64_t pdn_last_up_ms;
static struct net_mgmt_event_callback conn_cb;
#endif

static K_SEM_DEFINE(network_connected_sem, 0, 1);


static K_SEM_DEFINE(update_evt_seen_sem, 0, 1);


extern struct k_sem g_sem_lte_ready;
extern struct k_sem g_sem_pdn_ready;
extern struct k_sem g_sem_ip_ready;

extern int stcp_lte_reset_everythign(struct stcp_ctx *ctx);

int stcp_transport_close(void *vpCtx)
{
    LDBG("Transport closing scheduling destroy for %p...", vpCtx);
    worker_schedule_cleanup((struct stcp_ctx *)vpCtx);
    return 0;
}

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

// Watchdog update, ei omassa headerissa, koska tämä on piilossa kaikilta.
void stcp_watchdog_update_activity(void);


static int stcp_transport_modem_has_ip() {
    char buf[128];
    (void)nrf_modem_at_cmd(buf, sizeof(buf), "AT+CGPADDR");
    /* crude but effective check for IPv4 */
    if (strstr(buf, "\"10.") ||
        strstr(buf, "\"100.") ||
        strstr(buf, "\"172.") ||
        strstr(buf, "\"192.")) {
        return 1;
    }
    return 0;
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
        LERR("SoftReset: Error while doing transport set fd, %d", rc);
        return rc;
    }

    LDBG("SoftReset: Initialising transport ...");
    rc = stcp_transport_init();
    if (rc < 0) {
        LERR("SoftReset: Error while doing transport init, %d", rc);
        return rc;
    }

    LDBG("SoftReset: Modem setting: Offline...");
    rc = lte_lc_offline();
    if (rc < 0) {
        LERR("SoftReset: Error while setting offline, %d", rc);
        return rc;
    }

    k_sleep(K_SECONDS(1));
    LDBG("SoftReset: State setting to Normal...");
    rc = lte_lc_normal();
    if (rc < 0) {
        LERR("SoftReset: Error while setting NORMAL state, %d", rc);
        return rc;
    }

    LDBG(".========================================================");
    LDBG("| SoftReset: State: Normal...");
    LDBG("'========================================================");
#endif
    return 0;
}


void stcp_on_disconnect(struct stcp_ctx *ctx)
{
    LDBG("On disconnect called....");
    if (!ctx)  { return; }
    ctx->handshake_done = false;
    stcp_lte_reset_everythign(ctx);
}

static void l4_event_handler(struct net_mgmt_event_callback *cb,
			     uint32_t event,
			     struct net_if *iface)
{
    // Joka eventillä WD update
    stcp_watchdog_update_activity();
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
   LDBG("Waiting network bring up (%d sec max)...", seconds);
//
   int rc = k_sem_take(&network_connected_sem, K_SECONDS(seconds));
   if (rc < 0) {
       LERR("Network never became up!");
       stcp_lte_reset_everythign(NULL);
       return rc;
   }
   return 0;
}

int stcp_transport_wait_for_data_path(int seconds)
{
    LDBG("Waiting PDN data path (%d sec max)...", seconds);

    if (atomic_get(&g_pdn_active)) {
        LDBG("PDN already active");
        return 0;
    }

    int rc = k_sem_take(&g_sem_pdn_ready, K_SECONDS(seconds));
    if (rc < 0) {
        LERR("PDN never became active!");
        stcp_lte_reset_everythign(NULL);
        return rc;
    }

    return 0;
}

#if 0
static void on_platform_ready(void)
{
    printk("STCP platform ready starts");
    stcp_transport_init();
    stcp_transport_connect();
    printk("STCP platform ready DONE");
}
#endif

int stcp_pdn_wait_until_active_or_secs_passed(int seconds)
{
    LDBG("Waiting for PDN semaphore (pdn_active) %d seconds MAX.....", seconds);
    return k_sem_take(&g_sem_pdn_ready, K_SECONDS(seconds));
}

int stcp_library_wait_until_lte_ready(int timeout) {
    return stcp_pdn_wait_until_active_or_secs_passed(timeout);
}

int stcp_update_cell_event_wait_until_seen_or_secs_passed(int seconds)
{
    LDBG("Waiting for LTE update cell semaphore (update_evt_seen_sem) %d seconds MAX.....", seconds);
    return k_sem_take(&update_evt_seen_sem, K_SECONDS(seconds));
}


void dump_modem_full_status(void);

int stcp_transport_modem_lte_network_is_up(void)
{
    char buf[256];
    int ret;

    /* Check packet attach */
    memset(buf, 0, sizeof(buf));
    ret = nrf_modem_at_cmd(buf, sizeof(buf), "AT+CGATT?");
    if (ret != 0 || strstr(buf, "+CGATT: 1") == NULL) {
        return 0;
    }

    /* Check PDN active */
    memset(buf, 0, sizeof(buf));
    ret = nrf_modem_at_cmd(buf, sizeof(buf), "AT+CGACT?");
    if (ret != 0 || strstr(buf, "+CGACT: 0,1") == NULL) {
        return 0;
    }

    return stcp_transport_modem_has_ip();
}

static int stcp_transport_pdn_is_active() {
    char buf[128];
    nrf_modem_at_cmd(buf, sizeof(buf), "AT+CGACT?");

    if (strstr(buf, "0,1")) {
        return 1;
    }
    return 0;
}

static void lte_event_handler(const struct lte_lc_evt *evt)
{
    if (!evt) {
        LERR("LTE EVT: NULL event");
        return;
    }

    LINF("LTE EVT: type=%d", evt->type);

    // Joka eventillä WD update
    stcp_watchdog_update_activity();

    switch (evt->type) {

    case LTE_LC_EVT_NW_REG_STATUS:
        LINF("NW REG STATUS: %d", evt->nw_reg_status);
        switch (evt->nw_reg_status) {

        case LTE_LC_NW_REG_NOT_REGISTERED:
            LINF("LTE: NOT REGISTERED, starting reconnect...");
            worker_schedule_lte_reconnect();
            break;

        case LTE_LC_NW_REG_REGISTERED_HOME:
            LINFBIG("LTE: REGISTERED HOME");
            stcp_fsm_reached_lte_ready(NULL);
            dump_modem_full_status();
            break;

        case LTE_LC_NW_REG_SEARCHING:
            LINF("LTE: SEARCHING, starting reconnect...");
            worker_schedule_lte_reconnect();
            break;

        case LTE_LC_NW_REG_REGISTRATION_DENIED:
            LINF("LTE: REGISTRATION DENIED");
            break;

        case LTE_LC_NW_REG_UNKNOWN:
            LINF("LTE: UNKNOWN");
            break;

        case LTE_LC_NW_REG_REGISTERED_ROAMING:
            LINFBIG("LTE: REGISTERED ROAMING");
            stcp_fsm_reached_lte_ready(NULL);
            dump_modem_full_status();
            break;

        case LTE_LC_NW_REG_UICC_FAIL:
            LERR("LTE: UICC FAIL");
            break;

        default:
            LWRN("LTE: UNKNOWN REG STATUS %d", evt->nw_reg_status);
            break;
        }

        break;


#if defined(CONFIG_LTE_LC_PSM_MODULE)
    case LTE_LC_EVT_PSM_UPDATE:
        LINF("PSM UPDATE: TAU=%d ActiveTime=%d",
                evt->psm_cfg.tau,
                evt->psm_cfg.active_time);
        break;
#endif


#if defined(CONFIG_LTE_LC_EDRX_MODULE)
    case LTE_LC_EVT_EDRX_UPDATE:
        LINF("EDRX UPDATE: eDRX=%f PTW=%f",
                evt->edrx_cfg.edrx,
                evt->edrx_cfg.ptw);
        break;
#endif


    case LTE_LC_EVT_RRC_UPDATE:

        LINF("RRC UPDATE: mode=%d => state: %s",
            evt->rrc_mode,
            (evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED) ? "CONNECTED" : "IDLE"
        );
#if CONFIG_STCP_DEBUG_LATENCY
    {
        static uint32_t rrc_start = 0;
        if (evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED) {
            if (rrc_start > 0) {
                LINF("LTE: radio was idle for %u ms before wake",
                    k_uptime_get_32() - rrc_start
                );
                rrc_start = 0;
            }
        } else {
            rrc_start = k_uptime_get_32();
        }
    }
#endif

        break;

    case LTE_LC_EVT_CELL_UPDATE:
        LINFBIG("CELL UPDATE: id=%d tac=%d",
                evt->cell.id, evt->cell.tac);

        if (evt->cell.id == -1) {
            LINF("Starting reconnectin, CELL not found currently...");
            worker_schedule_lte_reconnect();
        }

        break;


    case LTE_LC_EVT_LTE_MODE_UPDATE:
        LINF("LTE MODE UPDATE: mode=%d", evt->lte_mode);
        break;


#if defined(CONFIG_LTE_LC_TAU_PRE_WARNING_MODULE)
    case LTE_LC_EVT_TAU_PRE_WARNING:
        LINF("TAU PRE WARNING: %llu ms",
                evt->time);
        break;
#endif


#if defined(CONFIG_LTE_LC_NEIGHBOR_CELL_MEAS_MODULE)
    case LTE_LC_EVT_NEIGHBOR_CELL_MEAS:
        LINF("NEIGHBOR CELL MEAS received");
        break;
#endif


#if defined(CONFIG_LTE_LC_MODEM_SLEEP_MODULE)
    case LTE_LC_EVT_MODEM_SLEEP_EXIT_PRE_WARNING:
        LINF("MODEM SLEEP EXIT PRE WARNING: %llu ms",
                evt->modem_sleep.time);
        break;

    case LTE_LC_EVT_MODEM_SLEEP_EXIT:
        LINF("MODEM SLEEP EXIT");
        break;

    case LTE_LC_EVT_MODEM_SLEEP_ENTER:
        LINF("MODEM SLEEP ENTER: duration %llu ms",
                evt->modem_sleep.time);
        break;
#endif


    case LTE_LC_EVT_MODEM_EVENT:
        LINF("MODEM EVENT: %d", evt->modem_evt);
        break;


#if defined(CONFIG_LTE_LC_RAI_MODULE)
    case LTE_LC_EVT_RAI_UPDATE:
        LINF("RAI UPDATE");
        break;
#endif


    default:
        LWRN("UNKNOWN LTE EVENT: %d", evt->type);
        break;
    }
}

int stcp_lte_issue_at_command(char *cmd) {
    char buf[512] = { 0 };
    int err = nrf_modem_at_cmd(buf, sizeof(buf), "%s", cmd);
    LDBG("%s rc=%d resp=%s", cmd, err, buf);
    if (err < 0) {
        LWRN("Command '%s' returned: %d", cmd, err);
    }

    return err;
}

static void dump_sim_status_of_command(char *cmd) {
    char buf[256] = { 0 };
    int err = nrf_modem_at_cmd(buf, sizeof(buf), "%s", cmd);
    LDBG("%s rc=%d resp=%s", cmd, err, buf);
}


void dump_sim_status(void)
{
    dump_sim_status_of_command("AT+CPIN?");
    dump_sim_status_of_command("AT+CGPADDR");
    dump_sim_status_of_command("AT+CEREG?");
    dump_sim_status_of_command("AT+CGATT?");
    dump_sim_status_of_command("AT+COPS?");
    dump_sim_status_of_command("AT+CSQ");
    dump_sim_status_of_command("AT+CGDCONT?");

    dump_sim_status_of_command("AT+CESQ");
    dump_sim_status_of_command("AT%%XMONITOR");
    dump_sim_status_of_command("AT%%XCONNSTAT?");
    dump_sim_status_of_command("AT%%XSYSTEMMODE?");

}

void dump_modem_full_status(void)
{
    LDBGBIG("MODEM STATUS DUMP");

    dump_sim_status_of_command("AT");
    dump_sim_status_of_command("AT+CFUN?");
    dump_sim_status_of_command("AT+CPIN?");
    dump_sim_status_of_command("AT+COPS?");
    dump_sim_status_of_command("AT+CEREG?");
    dump_sim_status_of_command("AT+CGATT?");
    dump_sim_status_of_command("AT+CGDCONT?");
    dump_sim_status_of_command("AT+CSQ");
    dump_sim_status_of_command("AT+CESQ");

    dump_sim_status_of_command("AT%XSYSTEMMODE?");
    dump_sim_status_of_command("AT%XMONITOR");
    dump_sim_status_of_command("AT%XCONNSTAT?");
    dump_sim_status_of_command("AT%XCBAND");

    dump_sim_status_of_command("AT+CGPADDR");
    dump_sim_status_of_command("AT+CGACT?");
    dump_sim_status_of_command("AT%XVBAT");
    dump_sim_status_of_command("AT%XTEMP?");

    LDBG("===========================================");
}

#if 0
static void wait_until_sim_ready(int maxTimes) {
    char resp[64] = { 0 };
    for (int i = 0; i < maxTimes; i++) {
        LDBG("Waiting SIM ready... %d / %d sec", i, maxTimes);
        nrf_modem_at_cmd(resp, sizeof(resp), "%s", "AT+CPIN?");
        if (strstr(resp, "READY")) {
            break;
        }
        k_sleep(K_SECONDS(1));
    }
}
#endif 
int stcp_poll_fd_changes(int fd, int timeout, int events)
{
    struct zsock_pollfd pfd = {
        .fd = fd,
        .events = events,
    };

    int rc = zsock_poll(&pfd, 1, timeout);

    if (rc < 0) {
        LERR("poll() error rc=%d errno=%d", rc, errno);
        return -errno;
    }

    if (rc == 0) {
        LDBG("poll() timeout");
        return -ETIMEDOUT;
    }

    /* Poll heräsi */
    LDBG("poll revents = 0x%x", pfd.revents);

    if (pfd.revents & ZSOCK_POLLIN) {
        return 1;  // data available
    }

    if (pfd.revents & ZSOCK_POLLOUT) {
        return 2;  // send possible
    }

    if (pfd.revents & ZSOCK_POLLERR) {
        LDBG("poll: socket closed");

        return -ECONNRESET;   // ei EIO
    }

    if (pfd.revents & ZSOCK_POLLHUP) {
        LERR("poll: peer closed");
        return -ECONNRESET;
    }

    if (pfd.revents & ZSOCK_POLLNVAL) {
        LERR("poll: invalid fd");
        return -EBADF;
    }

    return 0;
}

static void stcp_modem_force_apn(void)
{
    char buf[128];

    int ret = nrf_modem_at_cmd(buf, sizeof(buf), "%s", 
                               "AT+CGDCONT=1,\"IP\",\"internet\"");
    LDBG("CGDCONT set ret=%d resp=%s\n", ret, buf);

    ret = nrf_modem_at_cmd(buf, sizeof(buf), "%s", 
                           "AT+CGDCONT?");
    LDBG("CGDCONT query ret=%d resp=%s\n", ret, buf);
}

#if 0
static void dump_lte_monitor(void)
{
    char buf[256] = {0};

    int rc = nrf_modem_at_cmd(buf, sizeof(buf), "%s", "AT%%XMONITOR");

    if (rc) {
        LERR("[Transport Init] XMONITOR failed rc=%d", rc);
        return;
    }

    LDBGBIG("[Transport Init] MODEM RAW: %s", buf);
}
#endif

int stcp_transport_init(void)
{
    static int onceCalled = 0;

    k_sem_reset(&network_connected_sem);

    atomic_set(&g_pdn_active, 0);

    if (!onceCalled) {
        onceCalled = 1;

        LDBGBIG("[Transport Init] Registering LTE eventhandler... ");
        lte_lc_register_handler(lte_event_handler);

        LDBGBIG("[Transport Init] Registering LTE modem library... ");
        int err = nrf_modem_lib_init();
        LDBGBIG("[Transport Init] Registering LTE modem library, rc: %d / errno: %d", err, errno);
        if (err < 0) {
            LERRBIG("[Transport Init] Registering LTE modem library FAILED, rc: %d / errno: %d", err, errno);
            k_panic();
        }

        LDBGBIG("[Transport Init] Setting APN to %s", STCP_LTE_APN);
        stcp_modem_force_apn();

        LDBGBIG("[Transport Init] Turnng CSCON on", STCP_LTE_APN);
        stcp_lte_issue_at_command("AT+CSCON=1");

        LDBGBIG("[Transport Init] Registering LTE L4 event handler .....");
        net_mgmt_init_event_callback(&l4_cb, l4_event_handler, L4_EVENT_MASK);
        net_mgmt_add_event_callback(&l4_cb);

        LDBGBIG("[Transport Init] Starting LTE Connect .....");
        err = lte_lc_connect();
        if (err < 0) {
            LERRBIG("[Transport Init] Starting LTE Connect done, rc: %d / %d", err, errno);
            return err;
        }

        int cnt = 3*60 * 2; // 3 minutes
        int pdn_ok = stcp_transport_pdn_is_active();
        LDBGBIG("[Transport Init] PDN ok? %d ...", pdn_ok);
        while (!pdn_ok && cnt > 0) {
            // Wait a bit for state....
            k_msleep(500);
            pdn_ok = stcp_transport_pdn_is_active();
            LDBG("[Transport Init] PDN status: %d ...", pdn_ok);
            cnt--;
        }

        if (stcp_transport_pdn_is_active()) {
            LDBGBIG("[Transport Init] PDN is OK.");
        } else {
            LWRNBIG("[Transport Init] PDN is not OK after a while..");
        }
        atomic_set(&g_pdn_active, 1);
        k_sem_give(&g_sem_pdn_ready);

        cnt = 3*60 * 2; // 3 minutes
        int net_ok = stcp_transport_modem_lte_network_is_up();
        LDBGBIG("[Transport Init] Network ok? %d ...", net_ok);
        while (!net_ok && cnt > 0) {
            // Wait a bit for state....
            k_msleep(500);
            net_ok = stcp_transport_modem_lte_network_is_up();
            cnt--;
        }

        if (stcp_transport_modem_lte_network_is_up()) {
            LDBGBIG("[Transport Init] Network is UP..");
            k_sem_give(&network_connected_sem);
        } else {
            LWRNBIG("[Transport Init] Network is not UP after a while..");
            k_sem_give(&network_connected_sem);
        }

        LDBGBIG("[Transport Init] Calling platform ready....");
        stcp_platform_mark_ready();


//     wait_until_sim_ready(10*60); // 10min...
    } else {
        LDBG("Already called once..");
    }

    /*
    LDBG("Initialising LTE to normal state...");
    int err = lte_lc_normal();
    LDBG("LTE to normal-state done, RC: %d", err);
    if (err < 0) {
        LERR("LTE normal state failed!");
        return err;
    }
*/
    LDBGBIG("[Transport Init] Modem Status");
    dump_modem_full_status();

    //LDBG("Wait until SIM is registered, or 180 sec..");
    //stcp_registering_complete_wait_until_seen_or_secs_passed(180);

        /* Setup handler for Zephyr NET Connection Manager events. */

    LINFBIG("[Transport Init] Transport layer fully initialised!");
    return 0;
}


int stcp_transport_connect(void)
{
	int ret = conn_mgr_all_if_up(true);
	if (ret) {
		LERR("conn_mgr_all_if_up, error: %d", ret);
		return ret;
	}

  	ret = conn_mgr_all_if_connect(true);
	if (ret) {
		LERR("conn_mgr_all_if_connect, error: %d", ret);
		return ret;
	}

    //LDBG("Waiting for network up ........");
    //k_sem_take(&network_connected_sem, K_FOREVER);

    LDBG("Waiting LTE registration...");
    int rc = stcp_transport_wait_for_data_path(STCP_WAIT_FOR_DATA_PATH_IN_SECONDS);
    if (rc < 0) {
        LERR("PDN data path failed");
        return rc;
    }
    
    LDBGBIG("STCP LTE PDN ready!");

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

    LDBG("Sending %d bytes: %s", len, buf);
    int ret = 0;
    if (stcp_config_aes_bypass_enabled()) {
        LDBG("Bypassing AES...");
    } 
    ret = rust_exported_session_sendmsg(ctx->session, &ctx->ks, buf, len);
    LDBG("Called send message, rc: %d", ret);
    return ret;
}

int stcp_transport_send_iovec(struct stcp_ctx *ctx, const struct msghdr *message)
{
    if (stcp_is_context_valid(ctx) < 0) {
        LDBG("Called RUST send message, without context...");
        return -EBADFD;
    }
    
    if (!message) {
        LDBG("Called RUST send message, without buffer...");
        return -EBADFD;
    }

    int ret = 0;
    if (stcp_config_aes_bypass_enabled()) {
        LDBG("Bypassing AES...");
    }
    ret = rust_exported_session_sendmsg_iovec(ctx->session, &ctx->ks, (void *)message, 0, 1);
    LDBG("Called send message, rc: %d", ret);
    return ret;
}

int stcp_transport_recv(struct stcp_ctx *ctx, uint8_t *buf, size_t maxlen)
{
again:

    if (stcp_is_context_valid(ctx) < 0)
        return -EBADFD;

    if (atomic_get(&ctx->connection_closed))
        return -ENOTCONN;

    if (!buf)
        return -EINVAL;

    if (maxlen == 0)
        return -EINVAL;

        /* ------------------------------------------------ */
    /* 1. Onko decryptattu payload bufferissa           */
    /* ------------------------------------------------ */

    if (ctx->rx_payload_pos < ctx->rx_payload_len) {

        size_t avail = ctx->rx_payload_len - ctx->rx_payload_pos;
        size_t n = MIN(avail, maxlen);

        memcpy(buf,
               ctx->rx_payload + ctx->rx_payload_pos,
               n);

        ctx->rx_payload_pos += n;

        if (ctx->rx_payload_pos == ctx->rx_payload_len) {
            ctx->rx_payload_pos = 0;
            ctx->rx_payload_len = 0;
        }

        return n;
    }

    /* ------------------------------------------------ */
    /* 2. Yritetään lukea uusi STCP frame                */
    /* ------------------------------------------------ */

    int ret = stcp_recv_frame(ctx);

    if (ret == -EAGAIN) {
        LDBG("Ret: EAGAIN");
        return -EAGAIN;
    }

    if (ret == -ETIMEDOUT) {
        LDBG("Ret: ETIMEDOUT => EAGAIN");
        return -EAGAIN;
    }

    if (ret < 0) {
        LDBG("Error => Exit with ret: %d", ret);
        return ret;
    }

    LDBG("Frame received len=%d", ctx->rx_frame_len);

    /* ------------------------------------------------ */
    /* 3. decryptataan payload payload-bufferiin        */
    /* ------------------------------------------------ */

    int dec = rust_exported_session_recvmsg(
        ctx->session,
        ctx->rx_frame,
        ctx->rx_frame_len,
        ctx->rx_payload,
        sizeof(ctx->rx_payload)
    );

    if (dec == -EAGAIN)
        return -EAGAIN;


    if (dec == -EAGAIN) {
        LDBG("Ret @ dec: EAGAIN");
        return -EAGAIN;
    }

    if (dec < 0) {
        LDBG("Err @ dec: %d", dec);
        return dec;
    }

    ctx->rx_payload_len = dec;
    ctx->rx_payload_pos = 0;

    LDBG("Frame received len=%d", ctx->rx_frame_len);

    goto again;
}

int stcp_transport_wait_until_ready(int seconds)
{
    return stcp_platform_wait_until_platform_ready(seconds);
}
