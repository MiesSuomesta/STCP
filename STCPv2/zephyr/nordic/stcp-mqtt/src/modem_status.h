#ifndef MODEM_STATUS_H_
#define MODEM_STATUS_H_

#include <stdbool.h>
#include <zephyr/shell/shell.h>

#define MODEM_STATUS_FIELD_SIZE 96

struct modem_status_snapshot {
    bool system_valid;
    bool ltem_enabled;
    bool nbiot_enabled;
    bool gnss_enabled;
    int preference;
    bool signal_valid;
    int rsrq_index;
    int rsrp_index;
    int rsrq_tenths_db;
    int rsrp_dbm;
    bool monitor_valid;
    char operator_name[MODEM_STATUS_FIELD_SIZE];
    int current_act;
    int band;
    int monitor_rsrp_index;
    int snr_raw;
    bool registration_valid;
    int registration_status;
    int registration_act;
    bool rrc_valid;
    int rrc_mode;
    int rrc_state;
    bool psm_valid;
    int psm_enabled;
    bool pdp_valid;
    char pdp_type[MODEM_STATUS_FIELD_SIZE];
    char apn[MODEM_STATUS_FIELD_SIZE];
    char addresses[MODEM_STATUS_FIELD_SIZE];
};

int modem_status_get_snapshot(struct modem_status_snapshot *snapshot);

int modem_status_system(const struct shell *sh);
int modem_status_health(const struct shell *sh);
int modem_status_signal(const struct shell *sh);
int modem_status_network(const struct shell *sh);
int modem_status_band(const struct shell *sh);
int modem_status_packet(const struct shell *sh);
int modem_status_sleep(const struct shell *sh);
int modem_status_apn(const struct shell *sh);
int modem_status_contexts(const struct shell *sh);
int modem_status_all(const struct shell *sh);

#endif /* MODEM_STATUS_H_ */
