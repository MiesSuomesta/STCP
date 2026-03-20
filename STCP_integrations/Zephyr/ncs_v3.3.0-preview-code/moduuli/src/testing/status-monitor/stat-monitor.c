#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <errno.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_transport_api.h>
#include <stcp/stcp_transport.h>
#include <stcp/debug.h>
 
#define LOGTAG     "[STCP/Statistic] "
#include <stcp_testing_bplate.h>
#include <status_monitor.h>
#include <stcp/stcp_tcp_low_level_operations.h>


static struct k_mutex g_server_globals_mutex;
static struct stcp_server_stats g_server_globals_statistics;

#define STCP_MASTER_SERVER_STATS_STACK 1024
#define STCP_MASTER_SERVER_STATS_PRIO  4

K_THREAD_STACK_DEFINE(stcp_status_monitor_stack, STCP_MASTER_SERVER_STATS_STACK);
static struct k_thread stcp_status_monitor_thread;

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS

static void stcp_statistics_monitor_globals_mutex_lock() {
    k_mutex_lock(&g_server_globals_mutex, K_FOREVER);
}

static void stcp_statistics_monitor_globals_mutex_unlock() {
    k_mutex_unlock(&g_server_globals_mutex);
}

#define INC_STAT(storage, value) \
    do {                                                  \
        stcp_statistics_monitor_globals_mutex_lock();     \
        struct stcp_server_stats *pStats =                \
            stcp_server_statistics_get_ptr();             \
        if (pStats) {                                     \
            TINF("Increasing %s by %llu",                 \
                    #storage, value);                     \
            pStats->storage += value;                     \
        }                                                 \
        stcp_statistics_monitor_globals_mutex_unlock();   \
    } while (0)
    
#define DEC_STAT(storage, value) \
    do {                                                 \
        stcp_statistics_monitor_globals_mutex_lock();    \
        struct stcp_server_stats *pStats =               \
            stcp_server_statistics_get_ptr();            \
        if (pStats) {                                    \
            TINF("Decreasing %s by %llu",                \
                    #storage, value);                    \
            pStats->storage -= value;                    \
        }                                                \
        stcp_statistics_monitor_globals_mutex_unlock();  \
    } while (0)


#define GET_STAT(storage) \
    do {                                                 \
        uint64_t __return_value = 0;                     \
        stcp_statistics_monitor_globals_mutex_lock();    \
        struct stcp_server_stats *pStats =               \
            stcp_server_statistics_get_ptr();            \
        if (pStats) {                                    \
            __return_value = pStats->storage;            \
            TINF("Getting value of %s => %llu",          \
                    #storage, __return_value);           \
        }                                                \
        stcp_statistics_monitor_globals_mutex_unlock();  \
        return __return_value;                           \
    } while (0)

#define SET_STAT(storage, value) \
    do {                                                 \
        stcp_statistics_monitor_globals_mutex_lock();    \
        struct stcp_server_stats *pStats =               \
            stcp_server_statistics_get_ptr();            \
        if (pStats) {                                    \
            pStats->storage = value;                     \
            TINF("Setting value of %s => %llu",          \
                    #storage, value);                    \
        }                                                \
        stcp_statistics_monitor_globals_mutex_unlock();  \
    } while (0)

#else
#  define INC_STAT(storage, value)
#  define DEC_STAT(storage, value)
#  define GET_STAT(storage)
#  define SET_STAT(storage, value)
#endif

int stcp_server_check_for_command(char *buf, const char *pCmd, int len) {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    return memcmp(buf, pCmd, len) == 0;
#else
    return 0;
#endif
}

int stcp_statistic_monitor_check_for_command_reqest(char *buf, int len) {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    int cmdLen = sizeof(STCP_MASTER_SERVER_STATISTIC_CMD);
    if (len == cmdLen) {
        return memcmp(buf, STCP_MASTER_SERVER_STATISTIC_CMD, len) == 0;
    }
#endif
    return 0;
}



struct stcp_server_stats* stcp_server_statistics_get_ptr() {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    return &g_server_globals_statistics;
#else
    TWRN("Returning null.");
    return NULL;
#endif
}
atomic_t connection_good = ATOMIC_INIT(0);
static void stcp_master_server_stats_monitor(void *a, void *b, void *c)
{
#if CONFIG_STCP_STATUS_MONITOR
    int log_interval = CONFIG_STCP_STATUS_MONITOR_LOG_INTERVAL;
    LDBG("Starting status monitor, logging interval %d seconds",
        log_interval);

    while (1) {

        k_sleep(K_SECONDS(log_interval));

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
        uint64_t prev_rx = 0;
        uint64_t prev_tx = 0;
        struct stcp_server_stats *pStatistics = stcp_server_statistics_get_ptr();
        uint64_t rx;
        uint64_t tx;
        uint32_t running;
        uint32_t rejected;
        uint64_t uptime;
        uint64_t messages;
        uint64_t errors;

        stcp_statistics_monitor_globals_mutex_lock();

            rx       = pStatistics->rx_bytes;
            tx       = pStatistics->tx_bytes;
            running  = pStatistics->running;
            rejected = pStatistics->rejected;
            uptime = (k_uptime_get() - pStatistics->start_time) / 1000;
            messages = pStatistics->messages;
            errors   = pStatistics->errors;

        stcp_statistics_monitor_globals_mutex_unlock();

        uint64_t rx_delta = rx - prev_rx;
        uint64_t tx_delta = tx - prev_tx;

        prev_rx = rx;
        prev_tx = tx;

        uint32_t rx_rate = rx_delta / log_interval;
        uint32_t tx_rate = tx_delta / log_interval;

        TINF("STCP stats: uptime=%llu sec running=%u rx=%u B/s tx=%u B/s rejected=%u messages=%llu errors=%llu",
                uptime,
                running,
                rx_rate,
                tx_rate,
                rejected,
                messages,
                errors
        );
#endif

#define __GAT(val)    atomic_get(&(val))
#define GET_ATOM(val) __GAT(val)
#define OK(val)    ((__GAT(val) > 0) ? "OK"        : "NOK")
#define RADIO(val) ((__GAT(val) > 0) ? "CONNECTED" : "IDLE")

        u_int64_t up = k_uptime_get();

        int connection_ok = GET_ATOM(g_lte_active)    &&
                            GET_ATOM(g_pdn_active)    &&
                            GET_ATOM(g_ip_active)     &&
                            GET_ATOM(g_radio_active);

        atomic_set(&connection_good, connection_ok);

        int pdn = stcp_transport_pdn_is_active();
        int lte = stcp_transport_modem_lte_network_is_up();
        int ip  = stcp_transport_modem_has_ip();
        int real_connection_ok = lte && ip && pdn && GET_ATOM(g_radio_active);


        printk("[%llu ms] STCP stack state flags: LTE: %s PDN: %s IP: %s RADIO: %s => Connection: %s\n",
            up,
            OK(g_lte_active),
            OK(g_pdn_active),
            OK(g_ip_active),
            RADIO(g_radio_active),
            connection_ok ? "OK" : "NOK"
        );

        printk("[%llu ms] STCP stack state real : LTE: %s PDN: %s IP: %s RADIO: %s => Connection: %s\n",
            up,
            lte ? "OK" : "NOK",
            pdn ? "OK" : "NOK",
            ip ? "OK" : "NOK",
            RADIO(g_radio_active),
            real_connection_ok ? "OK" : "NOK"
        );
    }
#endif // MONITOR
}

void stcp_statistic_monitor_send_state_to(int fd) {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    char reply[650];
    stcp_statistics_monitor_globals_mutex_lock();
        struct stcp_server_stats *pStatistics = stcp_server_statistics_get_ptr();    
        uint64_t uptime = (k_uptime_get() - pStatistics->start_time) / 1000;
        int len = snprintk(reply, sizeof(reply),
            "uptime=%llu\n"
            "connections=%u\n"
            "running=%u\n"
            "max_running=%u\n"
            "rx_bytes=%llu\n"
            "tx_bytes=%llu\n"
            "messages=%llu\n"
            "errors=%llu\n"
            "recv_calls=%llu\n"
            "send_errors=%llu\n"
            "recv_errors=%llu\n"
            "rejected=%llu\n",
            (unsigned long long)uptime,
            pStatistics->connections,
            pStatistics->running,
            pStatistics->max_running,
            (unsigned long long)pStatistics->rx_bytes,
            (unsigned long long)pStatistics->tx_bytes,
            (unsigned long long)pStatistics->messages,
            (unsigned long long)pStatistics->errors,
            (unsigned long long)pStatistics->recv_calls,
            (unsigned long long)pStatistics->send_errors,                               
            (unsigned long long)pStatistics->recv_errors,
            (unsigned long long)pStatistics->rejected
        );
        stcp_tcp_send_via_fd(fd, reply, len);
    stcp_statistics_monitor_globals_mutex_unlock();
#endif
}

void stcp_status_monitor_start(void)
{
    static int running = 0;
    // pidä statistiikka arvot yli resumen...
    if (running) {
        LWRN("Already running....");
    } else {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
        LWRN("Inittin stats...");
        k_mutex_init(&g_server_globals_mutex);
        memset(&g_server_globals_statistics, 0, sizeof(g_server_globals_statistics));
    
        g_server_globals_statistics.start_time = k_uptime_get();
#endif
    }
    running = 1;

    TDBG("Monitor starting ....");
        k_tid_t tmp = k_thread_create(
            &stcp_status_monitor_thread,
            stcp_status_monitor_stack,
            K_THREAD_STACK_SIZEOF(stcp_status_monitor_stack),
            stcp_master_server_stats_monitor,
            NULL, NULL, NULL,
            STCP_MASTER_SERVER_STATS_PRIO,
            0,
            K_NO_WAIT
        );
        k_thread_name_set(tmp, "stcp_status_monitor_thread");
    TDBG("Monitor started ....");
}

void stcp_statistics_set(statistic_type_t statType, uint64_t val) {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    switch (statType) {

        case STAT_RX_BYTES:         SET_STAT(rx_bytes, val); break;
        case STAT_TX_BYTES:         SET_STAT(tx_bytes, val); break;
        case STAT_MESSAGES:         SET_STAT(messages, val); break;
        case STAT_START_TIME:       SET_STAT(start_time, val); break;
        case STAT_CONNECTIONS:      SET_STAT(connections, val); break;
        case STAT_ERRORS:           SET_STAT(errors, val); break;
        case STAT_RUNNING:          SET_STAT(running, val); break;
        case STAT_REJECTED:         SET_STAT(rejected, val); break;
        case STAT_MAX_RUNNING:      SET_STAT(max_running, val); break;
        case STAT_RECV_CALLS:       SET_STAT(recv_calls, val); break;
        case STAT_SEND_CALLS:       SET_STAT(send_calls, val); break;
        case STAT_RECV_ZERO:        SET_STAT(recv_zero, val); break;
        case STAT_RECV_ERRORS:      SET_STAT(recv_errors, val); break;
        case STAT_SEND_ERRORS:      SET_STAT(send_errors, val); break;
        case STAT_STAT_REQUESTS:    SET_STAT(stat_requests, val); break;
        case STAT_SHORT_READS:      SET_STAT(short_reads, val); break;
        case STAT_OVERSIZED_FRAMES: SET_STAT(oversized_frames, val); break;
        case STAT_TIME_PDN_ACTIVE:  SET_STAT(time_pdn_activated, val); break;
        case STAT_TIME_IP_ACTIVE:   SET_STAT(time_ip_activated, val); break;
        case STAT_TIME_LTE_ACTIVE:  SET_STAT(time_lte_activated, val); break;
        default: break;
    }
#endif    
}


uint64_t stcp_statistics_get(statistic_type_t statType)
{
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    switch (statType) {

        case STAT_RX_BYTES:         GET_STAT(rx_bytes); break;
        case STAT_TX_BYTES:         GET_STAT(tx_bytes); break;
        case STAT_MESSAGES:         GET_STAT(messages); break;
        case STAT_START_TIME:       GET_STAT(start_time); break;
        case STAT_CONNECTIONS:      GET_STAT(connections); break;
        case STAT_ERRORS:           GET_STAT(errors); break;
        case STAT_RUNNING:          GET_STAT(running); break;
        case STAT_REJECTED:         GET_STAT(rejected); break;
        case STAT_MAX_RUNNING:      GET_STAT(max_running); break;
        case STAT_RECV_CALLS:       GET_STAT(recv_calls); break;
        case STAT_SEND_CALLS:       GET_STAT(send_calls); break;
        case STAT_RECV_ZERO:        GET_STAT(recv_zero); break;
        case STAT_RECV_ERRORS:      GET_STAT(recv_errors); break;
        case STAT_SEND_ERRORS:      GET_STAT(send_errors); break;
        case STAT_STAT_REQUESTS:    GET_STAT(stat_requests); break;
        case STAT_SHORT_READS:      GET_STAT(short_reads); break;
        case STAT_OVERSIZED_FRAMES: GET_STAT(oversized_frames); break;

        case STAT_TIME_PDN_ACTIVE:  GET_STAT(time_pdn_activated); break;
        case STAT_TIME_IP_ACTIVE:   GET_STAT(time_ip_activated); break;
        case STAT_TIME_LTE_ACTIVE:  GET_STAT(time_lte_activated); break;
        default: break;
    }
#endif    
    return 0;
}

void stcp_statistics_inc(statistic_type_t statType, uint64_t val) {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    switch (statType) {

        case STAT_RX_BYTES:         INC_STAT(rx_bytes, val); break;
        case STAT_TX_BYTES:         INC_STAT(tx_bytes, val); break;
        case STAT_MESSAGES:         INC_STAT(messages, val); break;
        case STAT_START_TIME:       INC_STAT(start_time, val); break;
        case STAT_CONNECTIONS:      INC_STAT(connections, val); break;
        case STAT_ERRORS:           INC_STAT(errors, val); break;
        case STAT_RUNNING:          INC_STAT(running, val); break;
        case STAT_REJECTED:         INC_STAT(rejected, val); break;
        case STAT_MAX_RUNNING:      INC_STAT(max_running, val); break;
        case STAT_RECV_CALLS:       INC_STAT(recv_calls, val); break;
        case STAT_SEND_CALLS:       INC_STAT(send_calls, val); break;
        case STAT_RECV_ZERO:        INC_STAT(recv_zero, val); break;
        case STAT_RECV_ERRORS:      INC_STAT(recv_errors, val); break;
        case STAT_SEND_ERRORS:      INC_STAT(send_errors, val); break;
        case STAT_STAT_REQUESTS:    INC_STAT(stat_requests, val); break;
        case STAT_SHORT_READS:      INC_STAT(short_reads, val); break;
        case STAT_OVERSIZED_FRAMES: INC_STAT(oversized_frames, val); break;

        case STAT_TIME_PDN_ACTIVE:  INC_STAT(time_pdn_activated, val); break;
        case STAT_TIME_IP_ACTIVE:   INC_STAT(time_ip_activated, val); break;
        case STAT_TIME_LTE_ACTIVE:  INC_STAT(time_lte_activated, val); break;
        default: break;
    }
#endif    
}

void stcp_statistics_dec(statistic_type_t statType, uint64_t val) {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    switch (statType) {

        case STAT_RX_BYTES:         DEC_STAT(rx_bytes, val); break;
        case STAT_TX_BYTES:         DEC_STAT(tx_bytes, val); break;
        case STAT_MESSAGES:         DEC_STAT(messages, val); break;
        case STAT_START_TIME:       DEC_STAT(start_time, val); break;
        case STAT_CONNECTIONS:      DEC_STAT(connections, val); break;
        case STAT_ERRORS:           DEC_STAT(errors, val); break;
        case STAT_RUNNING:          DEC_STAT(running, val); break;
        case STAT_REJECTED:         DEC_STAT(rejected, val); break;
        case STAT_MAX_RUNNING:      DEC_STAT(max_running, val); break;
        case STAT_RECV_CALLS:       DEC_STAT(recv_calls, val); break;
        case STAT_SEND_CALLS:       DEC_STAT(send_calls, val); break;
        case STAT_RECV_ZERO:        DEC_STAT(recv_zero, val); break;
        case STAT_RECV_ERRORS:      DEC_STAT(recv_errors, val); break;
        case STAT_SEND_ERRORS:      DEC_STAT(send_errors, val); break;
        case STAT_STAT_REQUESTS:    DEC_STAT(stat_requests, val); break;
        case STAT_SHORT_READS:      DEC_STAT(short_reads, val); break;
        case STAT_OVERSIZED_FRAMES: DEC_STAT(oversized_frames, val); break;
        case STAT_TIME_PDN_ACTIVE:  DEC_STAT(time_pdn_activated, val); break;
        case STAT_TIME_IP_ACTIVE:   DEC_STAT(time_ip_activated, val); break;
        case STAT_TIME_LTE_ACTIVE:  DEC_STAT(time_lte_activated, val); break;
        default: break;
    }
#endif    
}
