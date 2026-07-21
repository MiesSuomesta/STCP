#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/net/socket.h>

#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>
#include <nrf_modem_at.h>

#include "stcp_lte_transport.h"

LOG_MODULE_REGISTER(stcp_lte_transport, LOG_LEVEL_INF);

static K_SEM_DEFINE(lte_ready_sem, 0, 1);
static K_SEM_DEFINE(pdn_ready_sem, 0, 1);
static K_SEM_DEFINE(network_ready_sem, 0, 1);
static K_SEM_DEFINE(rrc_ready_sem, 0, 1);

static atomic_t initialized = ATOMIC_INIT(0);
static atomic_t registered = ATOMIC_INIT(0);
static atomic_t pdn_active = ATOMIC_INIT(0);
static atomic_t ip_active = ATOMIC_INIT(0);
static atomic_t rrc_connected = ATOMIC_INIT(0);
static atomic_t roaming = ATOMIC_INIT(0);

static uint8_t active_pdn_cid;
static int active_pdn_id = -1;
static atomic_t custom_pdn_active = ATOMIC_INIT(0);
static int last_esm_error;

static void reset_runtime_state(void)
{
    atomic_clear(&registered);
    atomic_clear(&pdn_active);
    atomic_clear(&ip_active);
    atomic_clear(&rrc_connected);
    atomic_clear(&roaming);

    active_pdn_cid = 0;
    active_pdn_id = -1;
    atomic_clear(&custom_pdn_active);
    last_esm_error = 0;

    k_sem_reset(&lte_ready_sem);
    k_sem_reset(&pdn_ready_sem);
    k_sem_reset(&network_ready_sem);
    k_sem_reset(&rrc_ready_sem);
}

static bool modem_has_ip(void)
{
    char response[192] = { 0 };
    int ret;

    ret = nrf_modem_at_cmd(response, sizeof(response), "AT+CGPADDR");
    if (ret != 0) {
        return false;
    }

    /* Any non-empty IPv4/IPv6 address reported for a context is enough. */
    return strstr(response, ".") != NULL || strstr(response, ":") != NULL;
}

static bool modem_default_pdn_active(void)
{
    char response[128] = { 0 };
    int ret;

    ret = nrf_modem_at_cmd(response, sizeof(response), "AT+CGACT?");
    if (ret != 0) {
        return false;
    }

    return strstr(response, "+CGACT: 0,1") != NULL ||
           strstr(response, "+CGACT: 0, 1") != NULL;
}

static void mark_network_ready_if_possible(void)
{
    if (!atomic_get(&registered)) {
        return;
    }

    if (!atomic_get(&pdn_active) && modem_default_pdn_active()) {
        atomic_set(&pdn_active, 1);
        k_sem_give(&pdn_ready_sem);
    }

    if (!atomic_get(&ip_active) && modem_has_ip()) {
        atomic_set(&ip_active, 1);
    }

    if (atomic_get(&pdn_active) && atomic_get(&ip_active)) {
        k_sem_give(&network_ready_sem);
    }
}

static void lte_event_handler(const struct lte_lc_evt *evt)
{
    if (evt == NULL) {
        return;
    }

    switch (evt->type) {
    case LTE_LC_EVT_NW_REG_STATUS:
        switch (evt->nw_reg_status) {
        case LTE_LC_NW_REG_REGISTERED_HOME:
            atomic_set(&registered, 1);
            atomic_clear(&roaming);
            k_sem_give(&lte_ready_sem);
            LOG_INF("LTE registered (home)");
            mark_network_ready_if_possible();
            break;

        case LTE_LC_NW_REG_REGISTERED_ROAMING:
            atomic_set(&registered, 1);
            atomic_set(&roaming, 1);
            k_sem_give(&lte_ready_sem);
            LOG_INF("LTE registered (roaming)");
            mark_network_ready_if_possible();
            break;

        case LTE_LC_NW_REG_REGISTRATION_DENIED:
            atomic_clear(&registered);
            atomic_clear(&pdn_active);
            atomic_clear(&ip_active);
            LOG_ERR("LTE registration denied");
            break;

        case LTE_LC_NW_REG_UICC_FAIL:
            atomic_clear(&registered);
            atomic_clear(&pdn_active);
            atomic_clear(&ip_active);
            LOG_ERR("LTE UICC failure");
            break;

        default:
            if (evt->nw_reg_status == LTE_LC_NW_REG_NOT_REGISTERED) {
                atomic_clear(&registered);
                atomic_clear(&pdn_active);
                atomic_clear(&ip_active);
            }
            break;
        }
        break;

#if defined(CONFIG_LTE_LC_PDN_MODULE)
    case LTE_LC_EVT_PDN:
        active_pdn_cid = evt->pdn.cid;
        last_esm_error = evt->pdn.esm_err;

        switch (evt->pdn.type) {
        case LTE_LC_EVT_PDN_ACTIVATED:
            atomic_set(&pdn_active, 1);
            atomic_set(&ip_active, 1);
            k_sem_give(&pdn_ready_sem);
            k_sem_give(&network_ready_sem);
            LOG_INF("PDN activated: cid=%u", evt->pdn.cid);
            break;

        case LTE_LC_EVT_PDN_DEACTIVATED:
        case LTE_LC_EVT_PDN_CTX_DESTROYED:
        case LTE_LC_EVT_PDN_NETWORK_DETACH:
            atomic_clear(&pdn_active);
            atomic_clear(&ip_active);
            LOG_WRN("PDN inactive: cid=%u event=%d",
                    evt->pdn.cid, evt->pdn.type);
            break;

        case LTE_LC_EVT_PDN_SUSPENDED:
            atomic_clear(&ip_active);
            LOG_WRN("PDN suspended: cid=%u", evt->pdn.cid);
            break;

        case LTE_LC_EVT_PDN_RESUMED:
            atomic_set(&pdn_active, 1);
            atomic_set(&ip_active, 1);
            k_sem_give(&pdn_ready_sem);
            k_sem_give(&network_ready_sem);
            LOG_INF("PDN resumed: cid=%u", evt->pdn.cid);
            break;

        case LTE_LC_EVT_PDN_ESM_ERROR:
            atomic_clear(&pdn_active);
            atomic_clear(&ip_active);
            LOG_ERR("PDN ESM error: cid=%u error=%d",
                    evt->pdn.cid, evt->pdn.esm_err);
            break;

        case LTE_LC_EVT_PDN_APN_RATE_CONTROL_ON:
            LOG_WRN("APN rate control enabled: cid=%u", evt->pdn.cid);
            break;

        case LTE_LC_EVT_PDN_APN_RATE_CONTROL_OFF:
            LOG_INF("APN rate control disabled: cid=%u", evt->pdn.cid);
            break;

        default:
            break;
        }
        break;
#endif

    case LTE_LC_EVT_RRC_UPDATE:
        if (evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED) {
            atomic_set(&rrc_connected, 1);
            k_sem_give(&rrc_ready_sem);
            LOG_INF("RRC connected");
        } else {
            atomic_clear(&rrc_connected);
            k_sem_reset(&rrc_ready_sem);
            LOG_INF("RRC idle");
        }
        break;

    case LTE_LC_EVT_LTE_MODE_UPDATE:
        LOG_INF("LTE mode update: %d", evt->lte_mode);
        break;

    case LTE_LC_EVT_CELL_UPDATE:
        LOG_INF("Cell update: id=%d tac=%d", evt->cell.id, evt->cell.tac);
        break;

    default:
        break;
    }
}


static int configure_default_apn(void)
{
#if defined(CONFIG_STCP_LTE_FORCE_DEFAULT_APN)
    char response[256] = { 0 };
    int ret;

    /* Configure default PDP context CID 0 before LTE attach. */
    ret = nrf_modem_at_printf(
        "AT+CGDCONT=0,\"IPV4V6\",\"%s\"",
        CONFIG_STCP_LTE_DEFAULT_APN);
    if (ret != 0) {
        LOG_ERR("Default APN configure failed: %d", ret);
        return -EIO;
    }

    ret = nrf_modem_at_cmd(response, sizeof(response), "AT+CGDCONT?");
    if (ret != 0) {
        LOG_ERR("Default APN verification failed: %d", ret);
        return -EIO;
    }

    LOG_INF("PDP contexts before LTE attach: %s", response);

    if (strstr(response, CONFIG_STCP_LTE_DEFAULT_APN) == NULL) {
        LOG_ERR("Requested APN was not stored in CID 0");
        return -EIO;
    }
#endif

    return 0;
}

static int configure_optional_custom_pdn(void)
{
#if defined(CONFIG_STCP_LTE_CUSTOM_PDN)
    uint8_t cid = 0;
    int esm = 0;
    int pdn_id;
    int ret;
    char response[384] = { 0 };

    ret = lte_lc_pdn_ctx_create(&cid);
    if (ret < 0) {
        LOG_ERR("PDN context create failed: %d", ret);
        return ret;
    }

    ret = lte_lc_pdn_ctx_configure(cid, CONFIG_STCP_LTE_APN,
                                   LTE_LC_PDN_FAM_IPV4V6, NULL);
    if (ret < 0) {
        LOG_ERR("PDN context configure failed: cid=%u ret=%d", cid, ret);
        (void)lte_lc_pdn_ctx_destroy(cid);
        return ret;
    }

    ret = lte_lc_pdn_activate(cid, &esm, NULL);
    if (ret < 0) {
        LOG_ERR("PDN context activate failed: cid=%u ret=%d esm=%d",
                cid, ret, esm);
        (void)lte_lc_pdn_ctx_destroy(cid);
        return ret;
    }

    pdn_id = lte_lc_pdn_id_get(cid);
    if (pdn_id < 0) {
        LOG_ERR("PDN ID lookup failed: cid=%u ret=%d", cid, pdn_id);
        (void)lte_lc_pdn_deactivate(cid);
        (void)lte_lc_pdn_ctx_destroy(cid);
        return pdn_id;
    }

    active_pdn_cid = cid;
    active_pdn_id = pdn_id;
    atomic_set(&custom_pdn_active, 1);

    ret = nrf_modem_at_cmd(response, sizeof(response), "AT+CGDCONT?");
    if (ret == 0) {
        LOG_INF("PDP contexts after custom activation: %s", response);
    }

    LOG_INF("Custom PDN active: cid=%u pdn_id=%d apn='%s'",
            cid, pdn_id, CONFIG_STCP_LTE_APN);
#endif
    return 0;
}

static int wait_for_default_data_path(void)
{
    int64_t deadline = k_uptime_get() +
        (int64_t)CONFIG_STCP_LTE_PDN_TIMEOUT_SECONDS * 1000;

    while (k_uptime_get() < deadline) {
        mark_network_ready_if_possible();

        if (atomic_get(&pdn_active) && atomic_get(&ip_active)) {
            return 0;
        }

        (void)k_sem_take(&network_ready_sem, K_MSEC(500));
    }

    return -ETIMEDOUT;
}

int stcp_lte_transport_init(void)
{
    int ret;

    if (!atomic_cas(&initialized, 0, 1)) {
        return stcp_lte_transport_is_ready() ? 0 : -EALREADY;
    }

    reset_runtime_state();

    /* STCPv1 order: modem -> handler -> default PDN events -> connect. */
    ret = nrf_modem_lib_init();
    if (ret < 0) {
        LOG_ERR("nrf_modem_lib_init failed: %d", ret);
        atomic_clear(&initialized);
        return ret;
    }
    LOG_INF("nRF modem library initialized");

    ret = configure_default_apn();
    if (ret < 0) {
        atomic_clear(&initialized);
        return ret;
    }

    lte_lc_register_handler(lte_event_handler);

#if defined(CONFIG_LTE_LC_PDN_MODULE)
    ret = lte_lc_pdn_default_ctx_events_enable();
    if (ret < 0) {
        LOG_ERR("Default PDN event enable failed: %d", ret);
        atomic_clear(&initialized);
        return ret;
    }
#endif

    LOG_INF("Connecting LTE");
    ret = lte_lc_connect();
    if (ret < 0) {
        LOG_ERR("lte_lc_connect failed: %d", ret);
        atomic_clear(&initialized);
        return ret;
    }

    ret = k_sem_take(&lte_ready_sem,
                     K_SECONDS(CONFIG_STCP_LTE_REG_TIMEOUT_SECONDS));
    if (ret < 0 && !atomic_get(&registered)) {
        LOG_ERR("LTE registration timeout");
        atomic_clear(&initialized);
        return -ETIMEDOUT;
    }

    ret = configure_optional_custom_pdn();
    if (ret < 0) {
        atomic_clear(&initialized);
        return ret;
    }

    ret = wait_for_default_data_path();
    if (ret < 0) {
        LOG_ERR("LTE data path timeout");
        atomic_clear(&initialized);
        return ret;
    }

    LOG_INF("LTE transport ready: registered=%ld pdn=%ld ip=%ld cid=%u pdn_id=%d",
            (long)atomic_get(&registered),
            (long)atomic_get(&pdn_active),
            (long)atomic_get(&ip_active),
            active_pdn_cid, active_pdn_id);

    return 0;
}

int stcp_lte_transport_wait_ready(int timeout_seconds)
{
    int64_t deadline = k_uptime_get() + (int64_t)timeout_seconds * 1000;

    while (!stcp_lte_transport_is_ready()) {
        int64_t remaining = deadline - k_uptime_get();

        if (remaining <= 0) {
            return -ETIMEDOUT;
        }

        mark_network_ready_if_possible();
        k_sleep(K_MSEC(MIN(remaining, 100)));
    }

    return 0;
}

bool stcp_lte_transport_is_ready(void)
{
    return atomic_get(&registered) &&
           atomic_get(&pdn_active) &&
           atomic_get(&ip_active);
}

uint8_t stcp_lte_transport_pdn_cid(void)
{
    return active_pdn_cid;
}

int stcp_lte_transport_pdn_id(void)
{
    return active_pdn_id;
}

int stcp_lte_transport_bind_socket(int fd)
{
#if defined(CONFIG_STCP_LTE_CUSTOM_PDN)
    int pdn_id = active_pdn_id;

    if (!atomic_get(&custom_pdn_active) || pdn_id < 0) {
        return -ENETDOWN;
    }

    if (zsock_setsockopt(fd, SOL_SOCKET, SO_BINDTOPDN,
                         &pdn_id, sizeof(pdn_id)) < 0) {
        int err = errno;
        LOG_ERR("SO_BINDTOPDN failed: fd=%d cid=%u pdn_id=%d errno=%d",
                fd, active_pdn_cid, pdn_id, err);
        return -err;
    }

    LOG_INF("Socket %d bound to custom PDN: cid=%u pdn_id=%d",
            fd, active_pdn_cid, pdn_id);
#else
    ARG_UNUSED(fd);
#endif
    return 0;
}

int stcp_lte_transport_get_state(struct stcp_lte_transport_state *state)
{
    if (state == NULL) {
        return -EINVAL;
    }

    memset(state, 0, sizeof(*state));
    state->initialized = atomic_get(&initialized) != 0;
    state->registered = atomic_get(&registered) != 0;
    state->pdn_active = atomic_get(&pdn_active) != 0;
    state->ip_active = atomic_get(&ip_active) != 0;
    state->rrc_connected = atomic_get(&rrc_connected) != 0;
    state->roaming = atomic_get(&roaming) != 0;
    state->custom_pdn_active = atomic_get(&custom_pdn_active) != 0;
    state->pdn_cid = active_pdn_cid;
    state->pdn_id = active_pdn_id;
    state->last_esm_error = last_esm_error;

    return 0;
}
