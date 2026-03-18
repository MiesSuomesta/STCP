#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <stdbool.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>

#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_platform.h>
#include <stcp_api.h>
#include <stcp/debug.h>




#ifndef STCP_VERSION_STR
#define STCP_VERSION_STR "unknown"
#endif

#ifndef STCP_BUILD_DATE
#define STCP_BUILD_DATE "unknown"
#endif

#ifndef STCP_GIT_SHA
#define STCP_GIT_SHA "unknown"
#endif

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

typedef void (*stcp_platform_ready_cb_t)(void);

static K_SEM_DEFINE(platform_ready_sem, 0, 1);
static int _stcp_platform_is_ready = 0;
static stcp_platform_ready_cb_t user_platform_ready_callback = NULL;

// tässä myöhemmin: socket connect, stcp_net_set_sock, rng init, jne.
extern void stcp_rust_log(int level, const uint8_t *buf, uintptr_t len);

extern void *stcp_rust_kernel_socket_create(int fd);
extern void  stcp_rust_kernel_socket_destroy(void * p);

#if CONFIG_STCP_SELFTEST
extern int stcp_crypto_selftest(void);
#endif

static void stcp_force_rust_symbols(void)
{
    // Käyttö, niin tulee mukaan moduuliin varmasti!
    void *tmp = stcp_rust_kernel_socket_create(0);
    if (tmp)
        stcp_rust_kernel_socket_destroy(tmp);
    char *pMsg = "RUST logging enabled.";
    stcp_rust_log(1, pMsg, strlen(pMsg));
}

int stcp_platform_init_banner(void)
{

    stcp_force_rust_symbols();

#if CONFIG_STCP_SELFTEST
    int ret = stcp_crypto_selftest();
#endif

    printk(".----<[ STCP by Paxsudos IT (c) 2026 - 2050 ]>------------------------------------------------------------>\n");
    printk("|  ✅ STCP Initialised (Version %s), Protocol number %d\n", STCP_VERSION_STR, IPPROTO_STCP);
    printk("|  🕓 Build at %s (%s)\n", STCP_BUILD_DATE, STCP_GIT_SHA);
    printk("|  🔐 Crypto: ECDH + AES-256-GCM\n");
#if CONFIG_STCP_SELFTEST
    printk("|  🧪 STCP Self-test: %s (%d)\n", (ret) ? "FAIL" : "PASS", ret);
#else
    printk("|  🧪 STCP Self-test: DISABLED\n");
#endif
    printk("|\n");
    printk("| Configuration:\n");
    printk("|   * The APN: %s\n", CONFIG_STCP_LTE_APN_NAME);

#if CONFIG_MQTT_LIB_STCP
    printk("|   * STCP MQTT transport enabled.\n");
#else
    printk("|   * STCP MQTT transport disabled.\n");
#endif

#if CONFIG_STCP_DEBUG
    printk("|   * STCP DEBUG enabled.\n");
#else
    printk("|   * STCP DEBUG disabled.\n");
#endif

#if CONFIG_STCP_AES_BYPASS
    printk("|   * AES BYPASS ENABLED!\n");
#else
    printk("|   * STCP AES bypass disabled.\n");
#endif

#if CONFIG_STCP_WATCHDOG_ENABLE
    printk("|   * Watchdog ENABLED!\n");
    printk("|       Max timeout   : %d seconds\n", CONFIG_STCP_WATCHDOG_MAX_TIMEOUT);
    printk("|       Check interval: %d seconds\n", CONFIG_STCP_WATCHDOG_CHECK_INTERVAL);
#else
    printk("|   * Watchdog disabled.\n");
#endif

#if CONFIG_STCP_STATISTICS
    printk("|   * Statistics ENABLED!\n");
    printk("|       Logging interval: %d\n", CONFIG_STCP_STATISTICS_LOG_INTERVAL);
#else
    printk("|   * Statistics disabled.\n");
#endif

#if CONFIG_STCP_DEBUG_LATENCY
    printk("|   * Latency debug enabled.\n");
#else
    printk("|   * Latency debug disabled.\n");
#endif

#if CONFIG_STCP_TESTING
    printk("|   * Test-server ENABLED, mode: %d\n",
        CONFIG_STCP_TESTING_MODE);
    printk("|        Hostname to connect: %s\n", CONFIG_STCP_TESTING_PEER_HOSTNAME_TO_CONNECT);
    printk("|        Port to connect    : %s\n", CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT);
#endif


    printk("'---------------------------------------------------------------------->\n");

    return 0;
}

void stcp_platform_mark_ready(void)
{
    if (!_stcp_platform_is_ready) {
        _stcp_platform_is_ready = 1;
        k_sem_give(&platform_ready_sem);
        if (user_platform_ready_callback != NULL) {
            SDBG("Calling user CB for platform ready");
            user_platform_ready_callback();
        }
    }
}

void my_fault_handler(struct nrf_modem_fault_info *fault)
{
    LERR("MODEM FAULT: reason=%d pc=0x%x\n",
           fault->reason, fault->program_counter);
}

int stcp_platform_init(stcp_platform_ready_cb_t cb) {
    stcp_platform_soft_reset();
    user_platform_ready_callback = cb;
    return 0;
}

int stcp_platform_soft_reset() {
    k_sem_init(&platform_ready_sem, 0, 1);
     _stcp_platform_is_ready = 0;
    return 0;
}

bool stcp_platform_is_ready(void) {
    return _stcp_platform_is_ready == 1;
}

int stcp_platform_wait_until_platform_ready(int seconds) {
    SDBG("LTE: Waiting for platform ready (%d Seconds)...", seconds);
	int semRet = k_sem_take(&platform_ready_sem, K_SECONDS(seconds));
    if (semRet<0) {
        LERR("STCP: Platform not ready within %d seconds, RC: %d",
            seconds, semRet);
    }
    SDBG("LTE: Plaform OK?: %d", semRet);
    return semRet;
}

