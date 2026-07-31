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

// 5 minutes wait for PDN ready...
#define STCP_WAIT_FOR_DATA_PATH_IN_SECONDS  (60 * 5)

// 30 secs debounce time
#define STCP_PDN_DEBOUNCE_TIME_IN_MS        (30 * 1000)


atomic_t g_pdn_active = ATOMIC_INIT(0);
atomic_t g_lte_active = ATOMIC_INIT(0);
atomic_t g_ip_active = ATOMIC_INIT(0);
atomic_t g_radio_active = ATOMIC_INIT(0);
atomic_t reset_requested = ATOMIC_INIT(0);

static K_SEM_DEFINE(network_connected_sem, 0, 1);


static K_SEM_DEFINE(update_evt_seen_sem, 0, 1);

// Semaforit

struct k_sem g_sem_lte_ready;
struct k_sem g_sem_pdn_ready;
struct k_sem g_sem_ip_ready;


int stcp_pdn_is_active() {
    return atomic_get(&g_pdn_active) == 1;
}

int stcp_transport_modem_has_ip() {
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


void stcp_on_disconnect(struct stcp_ctx *ctx)
{
    LDBG("On disconnect called....");
    if (!ctx)  { return; }
    ctx->handshake_done = false;
    stcp_lte_reset_everythign(ctx);
}

int stcp_transport_wait_for_network_up(int seconds)
{
   LDBG("Waiting network bring up (%d sec max)...", seconds);
   if (atomic_get(&g_ip_active)) {
        LDBG("Network marked active, no need to wait");
        return 0;
   }

   int rc = k_sem_take(&network_connected_sem, K_SECONDS(seconds));
   if (rc < 0) {
       LERR("Network never became up!");
       return rc;
   }
   return 0;
}

int stcp_transport_wait_for_data_path(int seconds)
{
    LDBG("Waiting PDN data path (%d sec max)...", seconds);
    if (atomic_get(&g_pdn_active)) {
        LDBG("PDN marked active, no need to wait");
        return 0;
    }

    int rc = k_sem_take(&g_sem_pdn_ready, K_SECONDS(seconds));
    if (rc < 0) {
        LERR("PDN never became active!");
        return rc;
    }

    return 0;
}

int stcp_pdn_wait_until_active_or_secs_passed(int seconds)
{
    LDBG("Waiting for PDN semaphore (pdn_active) %d seconds MAX.....", seconds);
    if (atomic_get(&g_pdn_active)) {
        LDBG("PDN marked active, no need to wait");
        return 0;
    }
    return k_sem_take(&g_sem_pdn_ready, K_SECONDS(seconds));
}

int stcp_library_wait_until_lte_ready(int timeout) {
    if (atomic_get(&g_lte_active)) {
        LDBG("LTE marked active, no need to wait");
        return 0;
    }

    return stcp_pdn_wait_until_active_or_secs_passed(timeout);
}

int stcp_update_cell_event_wait_until_seen_or_secs_passed(int seconds)
{
    LDBG("Waiting for LTE update cell semaphore (update_evt_seen_sem) %d seconds MAX.....", seconds);
    return k_sem_take(&update_evt_seen_sem, K_SECONDS(seconds));
}

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

int stcp_transport_pdn_is_active() {
    char buf[128];
    nrf_modem_at_cmd(buf, sizeof(buf), "AT+CGACT?");

    if (strstr(buf, "0,1")) {
        return 1;
    }
    return 0;
}

static void the_lte_event_handler(const struct lte_lc_evt *evt)
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
            atomic_set(&g_lte_active, 0);
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
            atomic_set(&g_lte_active, 0);
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
        int connected = (evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED);
        LINF("RRC UPDATE: mode=%d => state: %s",
            evt->rrc_mode,
            connected ? "CONNECTED" : "IDLE"
        );

        atomic_set(&g_radio_active, connected);


#if CONFIG_STCP_DEBUG_LATENCY
    {
        static uint32_t rrc_start = 0;
        if (connected) {
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

int stcp_transport_init(void)
{
    static int onceCalled = 0;

    k_sem_reset(&network_connected_sem);

    atomic_set(&g_lte_active, 0);
    atomic_set(&g_pdn_active, 0);
    atomic_set(&g_ip_active, 0);
    
    LDBG("@INIT: resetting semaphores LTE/PDN/IP...");

    if (!onceCalled) {
        onceCalled = 1;

        LDBGBIG("[Transport Init] Registering LTE eventhandler... ");
        lte_lc_register_handler(the_lte_event_handler);

        LDBGBIG("[Transport Init] Registering LTE modem library... ");
        int err = nrf_modem_lib_init();
        LDBGBIG("[Transport Init] Registering LTE modem library, rc: %d / errno: %d", err, errno);
        if (err < 0) {
            LERRBIG("[Transport Init] Registering LTE modem library FAILED, rc: %d / errno: %d", err, errno);
            k_panic();
        }

        atomic_set(&g_lte_active, 1);

        LDBGBIG("[Transport Init] Setting APN to %s", STCP_LTE_APN);
        stcp_modem_force_apn();

        LDBGBIG("[Transport Init] Turnng CSCON on");
        stcp_lte_issue_at_command("AT+CSCON=1");

        LDBGBIG("[Transport Init] Forcing LTEM mode");
        err = lte_lc_system_mode_set(LTE_LC_SYSTEM_MODE_LTEM, LTE_LC_SYSTEM_MODE_LTEM);
        if (err < 0) {
            LERRBIG("[Transport Init] Forcing mode to LTEM failed, rc: %d / errno: %d", err, errno);
        }


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

        if (pdn_ok) {
            LDBGBIG("[Transport Init] PDN is OK.");
        } else {
            LWRNBIG("[Transport Init] PDN is not OK after a while..");
        }
        atomic_set(&g_pdn_active, 1);
        k_sem_give(&g_sem_pdn_ready);

        cnt = 6*60 * 2; // 6 minutes
        int net_ok = stcp_transport_modem_lte_network_is_up();
        LDBGBIG("[Transport Init] Network ok? %d ...", net_ok);
        while (!net_ok && cnt > 0) {
            // Wait a bit for state....
            k_msleep(500);
            net_ok = stcp_transport_modem_lte_network_is_up();
            cnt--;
        }

        if (net_ok) {
            LDBGBIG("[Transport Init] Network is UP..");
            k_sem_give(&network_connected_sem);
        } else {
            LWRNBIG("[Transport Init] Network is not UP after a while..");
            k_sem_give(&network_connected_sem);
        }
        atomic_set(&g_ip_active, 1);

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

int stcp_transport_wait_until_ready(int seconds)
{
    return stcp_platform_wait_until_platform_ready(seconds);
}
