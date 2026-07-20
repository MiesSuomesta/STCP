#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include <nrf_modem_at.h>

#include "modem_status.h"

#define MODEM_AT_RESPONSE_SIZE 2048
#define FIELD_SIZE 96

static char at_response[MODEM_AT_RESPONSE_SIZE];
K_MUTEX_DEFINE(at_response_lock);

struct modem_snapshot {
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
    char operator_name[FIELD_SIZE];
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
    char pdp_type[FIELD_SIZE];
    char apn[FIELD_SIZE];
    char addresses[FIELD_SIZE];
};

static void print_response_lines(const struct shell *sh, const char *response)
{
    const char *line = response;

    while (line && *line) {
        const char *end = strpbrk(line, "\r\n");
        size_t len = end ? (size_t)(end - line) : strlen(line);

        if (len > 0) {
            shell_print(sh, "  %.*s", (int)len, line);
        }

        if (!end) {
            break;
        }

        line = end + 1;
        while (*line == '\r' || *line == '\n') {
            line++;
        }
    }
}

static int query_at(char *response, size_t response_size, const char *command)
{
    int rc;

    k_mutex_lock(&at_response_lock, K_FOREVER);
    memset(response, 0, response_size);
    rc = nrf_modem_at_cmd(response, response_size, "%s", command);
    k_mutex_unlock(&at_response_lock);
    return rc;
}

static int run_at(const struct shell *sh, const char *label, const char *command)
{
    int rc = query_at(at_response, sizeof(at_response), command);

    if (rc == 0) {
        shell_print(sh, "%s [%s]", label, command);
        print_response_lines(sh, at_response);
    } else {
        shell_warn(sh, "%s [%s] failed: %d", label, command, rc);
        if (at_response[0] != '\0') {
            print_response_lines(sh, at_response);
        }
    }

    return rc;
}

static const char *find_payload(const char *response, const char *prefix)
{
    const char *p = strstr(response, prefix);

    if (!p) {
        return NULL;
    }
    p += strlen(prefix);
    while (*p == ' ' || *p == ':') {
        p++;
    }
    return p;
}

static bool csv_field(const char *payload, unsigned int wanted,
                      char *out, size_t out_size)
{
    unsigned int index = 0;
    const char *p = payload;

    if (!payload || !out || out_size == 0) {
        return false;
    }

    while (*p && *p != '\r' && *p != '\n') {
        const char *start;
        const char *end;
        bool quoted = false;
        size_t len;

        while (*p == ' ') {
            p++;
        }
        if (*p == '"') {
            quoted = true;
            p++;
        }
        start = p;

        if (quoted) {
            while (*p && *p != '"') {
                p++;
            }
            end = p;
            if (*p == '"') {
                p++;
            }
        } else {
            while (*p && *p != ',' && *p != '\r' && *p != '\n') {
                p++;
            }
            end = p;
            while (end > start && end[-1] == ' ') {
                end--;
            }
        }

        if (index == wanted) {
            len = (size_t)(end - start);
            if (len >= out_size) {
                len = out_size - 1;
            }
            memcpy(out, start, len);
            out[len] = '\0';
            return true;
        }

        while (*p && *p != ',' && *p != '\r' && *p != '\n') {
            p++;
        }
        if (*p == ',') {
            p++;
            index++;
        } else {
            break;
        }
    }

    return false;
}

static bool csv_int(const char *payload, unsigned int index, int *value)
{
    char field[32];
    char *end;
    long parsed;

    if (!csv_field(payload, index, field, sizeof(field)) || field[0] == '\0') {
        return false;
    }
    errno = 0;
    parsed = strtol(field, &end, 10);
    if (errno || end == field || *end != '\0') {
        return false;
    }
    *value = (int)parsed;
    return true;
}

static const char *act_name(int act)
{
    switch (act) {
    case 7:
        return "LTE-M";
    case 9:
        return "NB-IoT";
    default:
        return "Unknown";
    }
}

static const char *registration_name(int status)
{
    switch (status) {
    case 1:
        return "Registered (home)";
    case 5:
        return "Registered (roaming)";
    case 2:
        return "Searching";
    case 3:
        return "Denied";
    case 4:
        return "Unknown";
    default:
        return "Not registered";
    }
}

static const char *coverage_name(int rsrp_dbm, int rsrq_tenths_db)
{
    if (rsrp_dbm >= -100 && rsrq_tenths_db >= -100) {
        return "Good";
    }
    if (rsrp_dbm >= -115 && rsrq_tenths_db >= -140) {
        return "Fair";
    }
    if (rsrp_dbm >= -125 && rsrq_tenths_db >= -170) {
        return "Weak";
    }
    return "Poor";
}

static void format_tenths(char *buf, size_t size, int tenths)
{
    int abs_value = tenths < 0 ? -tenths : tenths;

    snprintf(buf, size, "%s%d.%d", tenths < 0 ? "-" : "",
             abs_value / 10, abs_value % 10);
}

static void snapshot_system(struct modem_snapshot *s)
{
    const char *p;

    if (query_at(at_response, sizeof(at_response), "AT%XSYSTEMMODE?") == 0) {
        p = find_payload(at_response, "%XSYSTEMMODE");
        if (p) {
            int ltem, nbiot, gnss, pref;
            if (sscanf(p, "%d,%d,%d,%d", &ltem, &nbiot, &gnss, &pref) == 4) {
                s->system_valid = true;
                s->ltem_enabled = ltem != 0;
                s->nbiot_enabled = nbiot != 0;
                s->gnss_enabled = gnss != 0;
                s->preference = pref;
            }
        }
    }
}

static void snapshot_signal(struct modem_snapshot *s)
{
    const char *p;

    if (query_at(at_response, sizeof(at_response), "AT+CESQ") == 0) {
        p = find_payload(at_response, "+CESQ");
        if (p) {
            int a, b, c, d, rsrq, rsrp;
            if (sscanf(p, "%d,%d,%d,%d,%d,%d", &a, &b, &c, &d,
                       &rsrq, &rsrp) == 6 && rsrq != 255 && rsrp != 255) {
                s->signal_valid = true;
                s->rsrq_index = rsrq;
                s->rsrp_index = rsrp;
                s->rsrq_tenths_db = (rsrq - 40) * 5;
                s->rsrp_dbm = rsrp - 141;
            }
        }
    }
}

static void snapshot_monitor(struct modem_snapshot *s)
{
    const char *p;

    if (query_at(at_response, sizeof(at_response), "AT%XMONITOR") != 0) {
        return;
    }
    p = find_payload(at_response, "%XMONITOR");
    if (!p) {
        return;
    }

    s->monitor_valid = true;
    (void)csv_field(p, 1, s->operator_name, sizeof(s->operator_name));
    (void)csv_int(p, 5, &s->current_act);
    (void)csv_int(p, 6, &s->band);
    (void)csv_int(p, 10, &s->monitor_rsrp_index);
    (void)csv_int(p, 11, &s->snr_raw);
}

static void snapshot_registration(struct modem_snapshot *s)
{
    const char *p;

    if (query_at(at_response, sizeof(at_response), "AT+CEREG?") != 0) {
        return;
    }
    p = find_payload(at_response, "+CEREG");
    if (!p) {
        return;
    }

    /* +CEREG: <n>,<stat>,...,<AcT> */
    s->registration_valid = csv_int(p, 1, &s->registration_status);
    (void)csv_int(p, 4, &s->registration_act);
}

static void snapshot_rrc(struct modem_snapshot *s)
{
    const char *p;

    if (query_at(at_response, sizeof(at_response), "AT+CSCON?") != 0) {
        return;
    }
    p = find_payload(at_response, "+CSCON");
    if (p && csv_int(p, 0, &s->rrc_mode) && csv_int(p, 1, &s->rrc_state)) {
        s->rrc_valid = true;
    }
}

static void snapshot_psm(struct modem_snapshot *s)
{
    const char *p;

    if (query_at(at_response, sizeof(at_response), "AT+CPSMS?") != 0) {
        return;
    }
    p = find_payload(at_response, "+CPSMS");
    if (p && csv_int(p, 0, &s->psm_enabled)) {
        s->psm_valid = true;
    }
}

static void snapshot_pdp(struct modem_snapshot *s)
{
    const char *p;

    if (query_at(at_response, sizeof(at_response), "AT+CGDCONT?") != 0) {
        return;
    }
    p = find_payload(at_response, "+CGDCONT");
    if (!p) {
        return;
    }
    s->pdp_valid = true;
    (void)csv_field(p, 1, s->pdp_type, sizeof(s->pdp_type));
    (void)csv_field(p, 2, s->apn, sizeof(s->apn));
    (void)csv_field(p, 3, s->addresses, sizeof(s->addresses));
}

static void take_snapshot(struct modem_snapshot *s)
{
    memset(s, 0, sizeof(*s));
    snapshot_system(s);
    snapshot_signal(s);
    snapshot_monitor(s);
    snapshot_registration(s);
    snapshot_rrc(s);
    snapshot_psm(s);
    snapshot_pdp(s);
}

int modem_status_system(const struct shell *sh)
{
    struct modem_snapshot s;
    int act;

    take_snapshot(&s);
    shell_print(sh, "========== MODEM SYSTEM ==========");

    if (s.system_valid) {
        shell_print(sh, "Configured LTE-M : %s", s.ltem_enabled ? "enabled" : "disabled");
        shell_print(sh, "Configured NB-IoT: %s", s.nbiot_enabled ? "enabled" : "disabled");
        shell_print(sh, "Configured GNSS  : %s", s.gnss_enabled ? "enabled" : "disabled");
        shell_print(sh, "Preference       : %d", s.preference);
    } else {
        shell_warn(sh, "System-mode configuration unavailable");
    }

    act = s.monitor_valid ? s.current_act : s.registration_act;
    shell_print(sh, "Current RAT      : %s (%d)", act_name(act), act);
    if (s.monitor_valid) {
        shell_print(sh, "Operator         : %s", s.operator_name[0] ? s.operator_name : "unknown");
        shell_print(sh, "LTE band         : %d", s.band);
    }
    if (s.registration_valid) {
        shell_print(sh, "Registration     : %s", registration_name(s.registration_status));
    }
    shell_print(sh, "==================================");

    return s.system_valid || s.monitor_valid ? 0 : -EIO;
}

int modem_status_health(const struct shell *sh)
{
    struct modem_snapshot s;
    char rsrq[24];
    int act;

    take_snapshot(&s);
    shell_print(sh, "=========== MODEM HEALTH ===========");

    act = s.monitor_valid ? s.current_act : s.registration_act;
    shell_print(sh, "Operator      : %s", s.monitor_valid && s.operator_name[0] ?
                s.operator_name : "unknown");
    shell_print(sh, "Technology    : %s", act_name(act));
    shell_print(sh, "Band          : %d", s.monitor_valid ? s.band : -1);

    if (s.signal_valid) {
        format_tenths(rsrq, sizeof(rsrq), s.rsrq_tenths_db);
        shell_print(sh, "RSRP          : %d dBm", s.rsrp_dbm);
        shell_print(sh, "RSRQ          : %s dB", rsrq);
        shell_print(sh, "Coverage      : %s", coverage_name(s.rsrp_dbm, s.rsrq_tenths_db));
    } else {
        shell_print(sh, "RSRP/RSRQ     : unavailable");
    }

    if (s.monitor_valid) {
        shell_print(sh, "SNR raw       : %d", s.snr_raw);
    }
    if (s.registration_valid) {
        shell_print(sh, "Registration  : %s", registration_name(s.registration_status));
    }
    if (s.rrc_valid) {
        shell_print(sh, "RRC           : %s", s.rrc_state ? "connected" : "idle");
    }
    if (s.psm_valid) {
        shell_print(sh, "PSM           : %s", s.psm_enabled ? "enabled" : "disabled");
    }
    if (s.pdp_valid) {
        shell_print(sh, "PDP type      : %s", s.pdp_type[0] ? s.pdp_type : "unknown");
        shell_print(sh, "APN           : %s", s.apn[0] ? s.apn : "unknown");
        shell_print(sh, "Addresses     : %s", s.addresses[0] ? s.addresses : "unknown");
    }
    shell_print(sh, "====================================");

    return s.monitor_valid || s.signal_valid ? 0 : -EIO;
}

int modem_status_signal(const struct shell *sh)
{
    struct modem_snapshot s;
    char rsrq[24];
    int rc = 0;

    take_snapshot(&s);
    shell_print(sh, "========== MODEM SIGNAL ==========");
    if (s.signal_valid) {
        format_tenths(rsrq, sizeof(rsrq), s.rsrq_tenths_db);
        shell_print(sh, "RSRP       : %d dBm (index %d)", s.rsrp_dbm, s.rsrp_index);
        shell_print(sh, "RSRQ       : %s dB (index %d)", rsrq, s.rsrq_index);
        shell_print(sh, "Quality    : %s", coverage_name(s.rsrp_dbm, s.rsrq_tenths_db));
    } else {
        shell_warn(sh, "CESQ radio metrics unavailable");
        rc = -EIO;
    }
    if (s.monitor_valid) {
        shell_print(sh, "SNR raw    : %d", s.snr_raw);
        shell_print(sh, "Band       : %d", s.band);
        shell_print(sh, "Technology : %s", act_name(s.current_act));
    }
    shell_print(sh, "==================================");
    return rc;
}

int modem_status_network(const struct shell *sh)
{
    int rc = 0;

    if (run_at(sh, "Registration", "AT+CEREG?") != 0) rc = -EIO;
    if (run_at(sh, "RRC connection", "AT+CSCON?") != 0) rc = -EIO;
    if (run_at(sh, "Functional mode", "AT+CFUN?") != 0) rc = -EIO;
    return rc;
}

int modem_status_band(const struct shell *sh)
{
    return run_at(sh, "LTE band", "AT%XCBAND") == 0 ? 0 : -EIO;
}

int modem_status_packet(const struct shell *sh)
{
    return run_at(sh, "Packet-domain statistics", "AT%XCONNSTAT?") == 0 ? 0 : -EIO;
}

int modem_status_sleep(const struct shell *sh)
{
    int rc = 0;

    if (run_at(sh, "Modem sleep state", "AT%XMODEMSLEEP?") != 0) rc = -EIO;
    if (run_at(sh, "PSM settings", "AT+CPSMS?") != 0) rc = -EIO;
    if (run_at(sh, "eDRX settings", "AT+CEDRXS?") != 0) rc = -EIO;
    return rc;
}

int modem_status_apn(const struct shell *sh)
{
    int rc = 0;

    if (run_at(sh, "PDP contexts", "AT+CGDCONT?") != 0) rc = -EIO;
    if (run_at(sh, "Active PDP context", "AT+CGCONTRDP") != 0) rc = -EIO;
    return rc;
}

int modem_status_all(const struct shell *sh)
{
    int failures = 0;

    shell_print(sh, "=== MODEM STATUS ===");
    failures += modem_status_health(sh) != 0;
    failures += modem_status_system(sh) != 0;
    failures += modem_status_signal(sh) != 0;
    failures += modem_status_network(sh) != 0;
    failures += modem_status_band(sh) != 0;
    failures += modem_status_packet(sh) != 0;
    failures += modem_status_sleep(sh) != 0;
    failures += modem_status_apn(sh) != 0;
    shell_print(sh, "=== MODEM STATUS COMPLETE (%d group%s with errors) ===",
                failures, failures == 1 ? "" : "s");

    return failures ? -EIO : 0;
}
