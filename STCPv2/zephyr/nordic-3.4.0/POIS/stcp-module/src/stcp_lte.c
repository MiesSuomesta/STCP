#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_at.h>

#include <stcp/stcp_lte.h>

LOG_MODULE_REGISTER(stcp_lte, CONFIG_STCP_LOG_LEVEL);

static atomic_t lte_registered = ATOMIC_INIT(0);
static atomic_t pdn_active = ATOMIC_INIT(0);
static atomic_t ip_ready = ATOMIC_INIT(0);
static atomic_t radio_connected = ATOMIC_INIT(0);
static atomic_t initialized = ATOMIC_INIT(0);

K_SEM_DEFINE(lte_ready_sem, 0, 1);
K_SEM_DEFINE(pdn_ready_sem, 0, 1);
K_SEM_DEFINE(ip_ready_sem, 0, 1);

static bool modem_has_ip(void)
{
    char response[160] = {0};
    int ret = nrf_modem_at_cmd(response, sizeof(response), "AT+CGPADDR");

    if (ret != 0) {
        return false;
    }

    /* Keep the STCPv1 behaviour: accept common private IPv4 ranges. */
    return strstr(response, "10.") != NULL ||
           strstr(response, "100.") != NULL ||
           strstr(response, "172.") != NULL ||
           strstr(response, "192.") != NULL;
}

static void update_ip_state(void)
{
    if (atomic_get(&pdn_active) != 0 && modem_has_ip()) {
        atomic_set(&ip_ready, 1);
        k_sem_give(&ip_ready_sem);
    }
}

static void lte_event_handler(const struct lte_lc_evt *evt)
{
    if (evt == NULL) {
        return;
    }

    switch (evt->type) {
    case LTE_LC_EVT_NW_REG_STATUS:
        if (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ||
            evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING) {
            atomic_set(&lte_registered, 1);
            k_sem_give(&lte_ready_sem);
            LOG_INF("LTE registered (%s)",
                    evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ?
                    "home" : "roaming");
        } else if (evt->nw_reg_status == LTE_LC_NW_REG_NOT_REGISTERED ||
                   evt->nw_reg_status == LTE_LC_NW_REG_REGISTRATION_DENIED ||
                   evt->nw_reg_status == LTE_LC_NW_REG_UICC_FAIL) {
            atomic_set(&lte_registered, 0);
            atomic_set(&pdn_active, 0);
            atomic_set(&ip_ready, 0);
        }
        break;

    case LTE_LC_EVT_PDN:
        switch (evt->pdn.type) {
        case LTE_LC_EVT_PDN_ACTIVATED:
        case LTE_LC_EVT_PDN_RESUMED:
            atomic_set(&pdn_active, 1);
            k_sem_give(&pdn_ready_sem);
            update_ip_state();
            LOG_INF("PDN data path active");
            break;
        case LTE_LC_EVT_PDN_DEACTIVATED:
        case LTE_LC_EVT_PDN_SUSPENDED:
        case LTE_LC_EVT_PDN_NETWORK_DETACH:
        case LTE_LC_EVT_PDN_CTX_DESTROYED:
        case LTE_LC_EVT_PDN_ESM_ERROR:
            atomic_set(&pdn_active, 0);
            atomic_set(&ip_ready, 0);
            LOG_WRN("PDN data path down, event=%d", evt->pdn.type);
            break;
        default:
            break;
        }
        break;

    case LTE_LC_EVT_RRC_UPDATE:
        atomic_set(&radio_connected,
                   evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ? 1 : 0);
        break;

    default:
        break;
    }
}

static int wait_sem_or_state(struct k_sem *sem, atomic_t *state,
                             int timeout_seconds)
{
    if (atomic_get(state) != 0) {
        return 0;
    }

    if (k_sem_take(sem, K_SECONDS(timeout_seconds)) != 0) {
        return -ETIMEDOUT;
    }

    return atomic_get(state) != 0 ? 0 : -ENETDOWN;
}

int stcp_lte_wait_until_ready(int timeout_seconds)
{
    int ret;

    ret = wait_sem_or_state(&lte_ready_sem, &lte_registered, timeout_seconds);
    if (ret < 0) {
        return ret;
    }

    ret = wait_sem_or_state(&pdn_ready_sem, &pdn_active, timeout_seconds);
    if (ret < 0) {
        return ret;
    }

    update_ip_state();
    ret = wait_sem_or_state(&ip_ready_sem, &ip_ready, timeout_seconds);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int stcp_lte_init_and_connect(int timeout_seconds)
{
    int ret;

    if (!atomic_cas(&initialized, 0, 1)) {
        return stcp_lte_wait_until_ready(timeout_seconds);
    }

    atomic_set(&lte_registered, 0);
    atomic_set(&pdn_active, 0);
    atomic_set(&ip_ready, 0);
    atomic_set(&radio_connected, 0);
    k_sem_reset(&lte_ready_sem);
    k_sem_reset(&pdn_ready_sem);
    k_sem_reset(&ip_ready_sem);

    ret = nrf_modem_lib_init();
    if (ret != 0) {
        atomic_set(&initialized, 0);
        LOG_ERR("nrf_modem_lib_init failed: %d", ret);
        return ret;
    }

    lte_lc_register_handler(lte_event_handler);

    ret = lte_lc_pdn_default_ctx_events_enable();
    if (ret < 0) {
        atomic_set(&initialized, 0);
        LOG_ERR("default PDN events enable failed: %d", ret);
        return ret;
    }

    LOG_INF("Connecting modem to LTE network");
    ret = lte_lc_connect();
    if (ret < 0) {
        atomic_set(&initialized, 0);
        LOG_ERR("lte_lc_connect failed: %d", ret);
        return ret;
    }

    return stcp_lte_wait_until_ready(timeout_seconds);
}

int stcp_lte_get_state(struct stcp_lte_state *state)
{
    if (state == NULL) {
        return -EINVAL;
    }

    state->registered = atomic_get(&lte_registered) != 0;
    state->pdn_active = atomic_get(&pdn_active) != 0;
    state->ip_ready = atomic_get(&ip_ready) != 0;
    state->radio_connected = atomic_get(&radio_connected) != 0;
    return 0;
}

int stcp_lte_dump_status(void)
{
    static const char *const commands[] = {
        "AT+CFUN?", "AT+CPIN?", "AT+CEREG?", "AT+CGATT?",
        "AT+CGDCONT?", "AT+CGPADDR", "AT+CSQ", "AT%XMONITOR"
    };
    char response[256];

    for (size_t i = 0; i < ARRAY_SIZE(commands); ++i) {
        memset(response, 0, sizeof(response));
        int ret = nrf_modem_at_cmd(response, sizeof(response), "%s", commands[i]);
        LOG_INF("%s -> rc=%d %s", commands[i], ret, response);
    }

    return 0;
}
