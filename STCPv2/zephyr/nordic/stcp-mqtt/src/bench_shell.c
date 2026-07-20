#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include "echo_benchmark.h"
#include "modem_status.h"

static struct bench_config shell_cfg;
static bool shell_cfg_ready;

static void ensure_config(void)
{
    if (!shell_cfg_ready) {
        bench_config_defaults(&shell_cfg);
        shell_cfg_ready = true;
    }
}

static int parse_u32(const struct shell *sh, const char *text,
                     uint32_t min, uint32_t max, uint32_t *value)
{
    char *end = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(text, &end, 0);
    if (errno || !end || *end != '\0' || parsed < min || parsed > max) {
        shell_error(sh, "Invalid value '%s' (range %u..%u)", text, min, max);
        return -EINVAL;
    }

    *value = (uint32_t)parsed;
    return 0;
}

static int cmd_config_show(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    ensure_config();
    shell_print(sh, "Host      : %s", shell_cfg.host);
    shell_print(sh, "Port      : %s", shell_cfg.port);
    shell_print(sh, "Transport : %s", bench_transport_name(shell_cfg.transport));
    shell_print(sh, "Chunk     : %u bytes", shell_cfg.chunk_size);
    shell_print(sh, "Total     : %u bytes", shell_cfg.total_bytes);
    shell_print(sh, "Timeout   : %u ms", shell_cfg.timeout_ms);
    shell_print(sh, "Report    : %u ms", shell_cfg.report_interval_ms);
    return 0;
}

static int cmd_config_host(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_config();
    if (strlen(argv[1]) >= sizeof(shell_cfg.host)) {
        shell_error(sh, "Host is too long");
        return -ENAMETOOLONG;
    }
    strcpy(shell_cfg.host, argv[1]);
    shell_print(sh, "Host = %s", shell_cfg.host);
    return 0;
}

static int cmd_config_port(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t port;
    ARG_UNUSED(argc);
    ensure_config();
    if (parse_u32(sh, argv[1], 1, 65535, &port) < 0) return -EINVAL;
    snprintk(shell_cfg.port, sizeof(shell_cfg.port), "%u", port);
    shell_print(sh, "Port = %s", shell_cfg.port);
    return 0;
}

static int cmd_config_chunk(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_config();
    if (parse_u32(sh, argv[1], 1, CONFIG_BENCH_MAX_CHUNK_SIZE, &shell_cfg.chunk_size) < 0) return -EINVAL;
    shell_print(sh, "Chunk = %u", shell_cfg.chunk_size);
    return 0;
}

static int cmd_config_total(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_config();
    if (parse_u32(sh, argv[1], 1, 64U * 1024U * 1024U, &shell_cfg.total_bytes) < 0) return -EINVAL;
    shell_print(sh, "Total = %u", shell_cfg.total_bytes);
    return 0;
}

static int cmd_config_timeout(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_config();
    if (parse_u32(sh, argv[1], 1000, 600000, &shell_cfg.timeout_ms) < 0) return -EINVAL;
    shell_print(sh, "Timeout = %u ms", shell_cfg.timeout_ms);
    return 0;
}


static int cmd_config_report(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_config();
    if (parse_u32(sh, argv[1], 0, 600000, &shell_cfg.report_interval_ms) < 0) return -EINVAL;
    shell_print(sh, "Report interval = %u ms", shell_cfg.report_interval_ms);
    return 0;
}

static int cmd_config_transport(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_config();
    if (!strcmp(argv[1], "tcp")) shell_cfg.transport = BENCH_TRANSPORT_TCP;
    else if (!strcmp(argv[1], "stcp")) shell_cfg.transport = BENCH_TRANSPORT_STCP;
    else if (!strcmp(argv[1], "tls")) shell_cfg.transport = BENCH_TRANSPORT_TLS;
    else {
        shell_error(sh, "Transport must be tcp, tls or stcp");
        return -EINVAL;
    }
    shell_print(sh, "Transport = %s", bench_transport_name(shell_cfg.transport));
    return 0;
}

static int run_and_report(const struct shell *sh, const char *name,
                          int (*fn)(const struct bench_config *))
{
    int rc;
    ensure_config();
    shell_print(sh, "%s: %s://%s:%s, total=%u, chunk=%u", name,
                bench_transport_name(shell_cfg.transport), shell_cfg.host,
                shell_cfg.port, shell_cfg.total_bytes, shell_cfg.chunk_size);
    rc = fn(&shell_cfg);
    if (rc < 0) {
        shell_error(sh, "%s failed: %d", name, rc);
        return rc;
    }
    shell_print(sh, "%s completed successfully", name);
    return 0;
}

static int cmd_bench_upload(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return run_and_report(sh, "upload", bench_run_upload); }
static int cmd_bench_download(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return run_and_report(sh, "download", bench_run_download); }
static int cmd_bench_full(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return run_and_report(sh, "full", bench_run_full); }
static int cmd_bench_all(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return run_and_report(sh, "all", bench_run_all); }

SHELL_STATIC_SUBCMD_SET_CREATE(config_cmds,
    SHELL_CMD(show, NULL, "Show runtime benchmark configuration", cmd_config_show),
    SHELL_CMD_ARG(host, NULL, "Set host: stcp config host <name|ip>", cmd_config_host, 2, 0),
    SHELL_CMD_ARG(port, NULL, "Set port: stcp config port <1..65535>", cmd_config_port, 2, 0),
    SHELL_CMD_ARG(chunk, NULL, "Set chunk bytes: stcp config chunk <1..65536>", cmd_config_chunk, 2, 0),
    SHELL_CMD_ARG(total, NULL, "Set total bytes: stcp config total <bytes>", cmd_config_total, 2, 0),
    SHELL_CMD_ARG(timeout, NULL, "Set inactivity timeout ms", cmd_config_timeout, 2, 0),
    SHELL_CMD_ARG(report, NULL, "Set progress report interval ms (0 disables)", cmd_config_report, 2, 0),
    SHELL_CMD_ARG(transport, NULL, "Set tcp, tls or stcp", cmd_config_transport, 2, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(bench_cmds,
    SHELL_CMD(upload, NULL, "Run continuous upload benchmark", cmd_bench_upload),
    SHELL_CMD(download, NULL, "Run continuous download benchmark", cmd_bench_download),
    SHELL_CMD(full, NULL, "Run full-duplex benchmark", cmd_bench_full),
    SHELL_CMD(all, NULL, "Run upload, download and full-duplex", cmd_bench_all),
    SHELL_SUBCMD_SET_END
);


static int cmd_modem_system(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_system(sh); }
static int cmd_modem_health(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_health(sh); }
static int cmd_modem_signal(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_signal(sh); }
static int cmd_modem_network(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_network(sh); }
static int cmd_modem_band(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_band(sh); }
static int cmd_modem_packet(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_packet(sh); }
static int cmd_modem_sleep(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_sleep(sh); }
static int cmd_modem_apn(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_apn(sh); }
static int cmd_modem_all(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return modem_status_all(sh); }

SHELL_STATIC_SUBCMD_SET_CREATE(modem_cmds,
    SHELL_CMD(system, NULL, "Show configured and currently attached radio system", cmd_modem_system),
    SHELL_CMD(health, NULL, "Show interpreted modem, radio and PDP summary", cmd_modem_health),
    SHELL_CMD(signal, NULL, "Show CESQ and serving-cell radio metrics", cmd_modem_signal),
    SHELL_CMD(network, NULL, "Show registration, RRC and functional mode", cmd_modem_network),
    SHELL_CMD(band, NULL, "Show current LTE band", cmd_modem_band),
    SHELL_CMD(packet, NULL, "Show packet-domain connection statistics", cmd_modem_packet),
    SHELL_CMD(sleep, NULL, "Show modem sleep, PSM and eDRX settings", cmd_modem_sleep),
    SHELL_CMD(apn, NULL, "Show configured and active PDP/APN contexts", cmd_modem_apn),
    SHELL_CMD(all, NULL, "Show all available modem status information", cmd_modem_all),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(stcp_cmds,
    SHELL_CMD(config, &config_cmds, "Runtime benchmark configuration", NULL),
    SHELL_CMD(bench, &bench_cmds, "Transport benchmarks", NULL),
    SHELL_CMD(modem, &modem_cmds, "nRF modem status and radio diagnostics", NULL),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(stcp, &stcp_cmds, "STCP transport test bench", NULL);
