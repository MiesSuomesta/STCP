#pragma once
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <errno.h>

typedef enum {
    STAT_RX_BYTES = 0,
    STAT_TX_BYTES,
    STAT_MESSAGES,
    STAT_START_TIME,
    STAT_CONNECTIONS,
    STAT_ERRORS,
    STAT_RUNNING,
    STAT_REJECTED,
    STAT_MAX_RUNNING,
    STAT_RECV_CALLS,
    STAT_SEND_CALLS,
    STAT_RECV_ZERO,
    STAT_RECV_ERRORS,
    STAT_SEND_ERRORS,
    STAT_STAT_REQUESTS,
    STAT_SHORT_READS,
    STAT_OVERSIZED_FRAMES,

    STAT_TIME_PDN_ACTIVE,
    STAT_TIME_IP_ACTIVE,
    STAT_TIME_LTE_ACTIVE
} statistic_type_t;


typedef enum {
    STCP_COMMAND_STATS,
    STCP_COMMAND_PING
} commands_type_t;

struct stcp_server_stats {
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t messages;
    uint64_t start_time;
    uint32_t connections;
    uint32_t errors;
    uint32_t running;
    uint32_t rejected;
    uint32_t max_running;
    uint64_t recv_calls;
    uint64_t send_calls;
    uint64_t recv_zero;
    uint64_t recv_errors;
    uint64_t send_errors;
    uint64_t stat_requests;
    uint64_t short_reads;
    uint64_t oversized_frames;

    uint64_t time_pdn_activated;
    uint64_t time_ip_activated;
    uint64_t time_lte_activated;
};

#define STCP_MASTER_SERVER_STATISTIC_CMD    "STATISTICS"
#define STCP_MASTER_SERVER_PING_CMD         "PING"

struct stcp_server_stats* stcp_server_statistics_get_ptr();

void   stcp_statistic_monitor_send_state_to(int fd);
void   stcp_status_monitor_start(void);
int    stcp_statistic_monitor_check_for_command_reqest(char *buf, int len);

void     stcp_statistics_inc(statistic_type_t statType, uint64_t val);
uint64_t stcp_statistics_get(statistic_type_t statType);
void     stcp_statistics_dec(statistic_type_t statType, uint64_t val);