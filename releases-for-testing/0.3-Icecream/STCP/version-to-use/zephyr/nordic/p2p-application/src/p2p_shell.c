#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/util.h>

#include "p2p_benchmark.h"
#include "p2p_noise_probe.h"

static struct p2p_bench_config p2p_cfg;
static bool p2p_cfg_ready;

enum p2p_job_kind {
    P2P_JOB_PING,
    P2P_JOB_UPLOAD,
    P2P_JOB_DOWNLOAD,
    P2P_JOB_FULL,
    P2P_JOB_ALL,
};

struct p2p_job {
    const struct shell *sh;
    struct p2p_bench_config cfg;
    enum p2p_job_kind kind;
    uint32_t ping_count;
};

K_THREAD_STACK_DEFINE(p2p_worker_stack, CONFIG_P2P_BENCH_WORKER_STACK_SIZE);
static struct k_thread p2p_worker_thread;
static struct k_sem p2p_job_sem;
static struct k_mutex p2p_job_lock;
static struct p2p_job p2p_pending_job;
static atomic_t p2p_worker_started;
static atomic_t p2p_job_pending;

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

static void emit_json_parts(const struct shell *sh, const char *json, int length)
{
    enum { PART = 88 };
    shell_print(sh, "STCP_BENCH_JSON_BEGIN %d", length);
    for (int off = 0; off < length; off += PART) {
        int n = MIN(PART, length - off);
        shell_print(sh, "STCP_BENCH_JSON_PART %.*s", n, json + off);
    }
    shell_print(sh, "STCP_BENCH_JSON_END");
}

static void ensure_p2p_config(void)
{
    if (!p2p_cfg_ready) {
        p2p_bench_config_defaults(&p2p_cfg);
        p2p_cfg_ready = true;
    }
}

static const char *p2p_job_name(enum p2p_job_kind kind)
{
    switch (kind) {
    case P2P_JOB_PING: return "ping";
    case P2P_JOB_UPLOAD: return "upload";
    case P2P_JOB_DOWNLOAD: return "download";
    case P2P_JOB_FULL: return "full";
    case P2P_JOB_ALL: return "all";
    default: return "unknown";
    }
}

static void p2p_print_result(const struct shell *sh, const char *name,
                             const struct p2p_bench_result *r)
{
    shell_print(sh,
        "P2P %-8s status=%d elapsed=%lld ms tx=%u rx=%u "
        "TX=%llu bit/s RX=%llu bit/s aggregate=%llu bit/s",
        name, r->status, r->elapsed_ms, r->bytes_tx, r->bytes_rx,
        r->tx_bps, r->rx_bps, r->aggregate_bps);
}

static void p2p_emit_machine_result(const struct shell *sh, const char *direction,
                                    const struct p2p_bench_config *cfg,
                                    const struct p2p_bench_result *r)
{
    static char json[768];
    int written = snprintk(json, sizeof(json),
        "{\"schema_version\":2,\"platform\":\"zephyr-nrf9151\"," 
        "\"transport\":\"stcp-p2p\",\"direction\":\"%s\"," 
        "\"payload_bytes\":%u,\"total_bytes\":%u,\"elapsed_ms\":%lld,"
        "\"status\":%d,\"errors\":%u,\"bytes_tx\":%u,\"bytes_rx\":%u,"
        "\"tx_bps\":%llu,\"rx_bps\":%llu,\"aggregate_bps\":%llu}",
        direction, cfg->payload_bytes, cfg->total_bytes, r->elapsed_ms,
        r->status, r->status < 0 ? 1U : 0U, r->bytes_tx, r->bytes_rx,
        r->tx_bps, r->rx_bps, r->aggregate_bps);

    if (written > 0 && written < (int)sizeof(json)) {
        emit_json_parts(sh, json, written);
    }
}

static int p2p_run_one(const struct p2p_job *job, enum p2p_job_kind kind)
{
    struct p2p_bench_result result;
    int rc;

    switch (kind) {
    case P2P_JOB_UPLOAD:
        rc = p2p_bench_upload(&job->cfg, &result);
        p2p_print_result(job->sh, "upload", &result);
        p2p_emit_machine_result(job->sh, "upload", &job->cfg, &result);
        return rc;
    case P2P_JOB_DOWNLOAD:
        rc = p2p_bench_download(&job->cfg, &result);
        p2p_print_result(job->sh, "download", &result);
        p2p_emit_machine_result(job->sh, "download", &job->cfg, &result);
        return rc;
    case P2P_JOB_FULL:
        rc = p2p_bench_full(&job->cfg, &result);
        p2p_print_result(job->sh, "full", &result);
        p2p_emit_machine_result(job->sh, "full", &job->cfg, &result);
        return rc;
    default:
        return -EINVAL;
    }
}

static int p2p_run_job(const struct p2p_job *job)
{
    if (!p2p_bench_backend_available()) {
        shell_error(job->sh,
            "STCP_P2P native backend is not linked yet; "
            "libp2p golden reference remains userspace-only");
        return -ENOSYS;
    }

    if (job->kind == P2P_JOB_PING) {
        uint32_t ok = 0U;
        uint64_t sum = 0U, min = 0U, max = 0U;
        int rc = p2p_bench_ping(&job->cfg, job->ping_count, &ok,
                                &sum, &min, &max);
        if (rc == 0 && ok > 0U) {
            shell_print(job->sh,
                "P2P PING peer=%s:%s ok=%u/%u min=%llu us mean=%llu us max=%llu us",
                job->cfg.host, job->cfg.port, ok, job->ping_count,
                min, sum / ok, max);
        }
        return rc;
    }

    if (job->kind == P2P_JOB_ALL) {
        int rc = p2p_run_one(job, P2P_JOB_UPLOAD);
        if (rc < 0) return rc;
        rc = p2p_run_one(job, P2P_JOB_DOWNLOAD);
        if (rc < 0) return rc;
        return p2p_run_one(job, P2P_JOB_FULL);
    }

    return p2p_run_one(job, job->kind);
}

static void p2p_worker_main(void *a, void *b, void *c)
{
    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    for (;;) {
        struct p2p_job job;
        k_sem_take(&p2p_job_sem, K_FOREVER);

        k_mutex_lock(&p2p_job_lock, K_FOREVER);
        job = p2p_pending_job;
        k_mutex_unlock(&p2p_job_lock);

        shell_print(job.sh,
            "P2P %s: stcp-p2p://%s:%s payload=%u total=%u timeout=%u",
            p2p_job_name(job.kind), job.cfg.host, job.cfg.port,
            job.cfg.payload_bytes, job.cfg.total_bytes, job.cfg.timeout_ms);

        int rc = p2p_run_job(&job);
        if (rc < 0) {
            shell_error(job.sh, "P2P %s failed: %d",
                        p2p_job_name(job.kind), rc);
        } else {
            shell_print(job.sh, "P2P %s completed successfully",
                        p2p_job_name(job.kind));
        }

        atomic_clear(&p2p_job_pending);
    }
}

static void ensure_p2p_worker(void)
{
    if (atomic_cas(&p2p_worker_started, 0, 1)) {
        k_sem_init(&p2p_job_sem, 0, 1);
        k_mutex_init(&p2p_job_lock);
        k_thread_create(&p2p_worker_thread,
                        p2p_worker_stack,
                        K_THREAD_STACK_SIZEOF(p2p_worker_stack),
                        p2p_worker_main,
                        NULL, NULL, NULL,
                        CONFIG_P2P_BENCH_WORKER_PRIORITY,
                        0, K_NO_WAIT);
        k_thread_name_set(&p2p_worker_thread, "stcp_p2p_bench");
    }
}

static int queue_p2p_job(const struct shell *sh, enum p2p_job_kind kind,
                         uint32_t ping_count)
{
    ensure_p2p_config();
    ensure_p2p_worker();

    if (!atomic_cas(&p2p_job_pending, 0, 1)) {
        shell_error(sh, "P2P benchmark already running");
        return -EBUSY;
    }

    k_mutex_lock(&p2p_job_lock, K_FOREVER);
    p2p_pending_job.sh = sh;
    p2p_pending_job.cfg = p2p_cfg;
    p2p_pending_job.kind = kind;
    p2p_pending_job.ping_count = ping_count;
    k_mutex_unlock(&p2p_job_lock);

    shell_print(sh, "P2P %s queued: stcp-p2p://%s:%s",
                p2p_job_name(kind), p2p_cfg.host, p2p_cfg.port);
    k_sem_give(&p2p_job_sem);
    return 0;
}

static int cmd_p2p_show(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    ensure_p2p_config();
    uint32_t core_abi = p2p_bench_core_abi();
    int core_test = p2p_bench_core_selftest();
    shell_print(sh, "P2P core    : linked ABI=%u selftest=%s (%d)",
                core_abi, core_test == 0 ? "PASS" : "FAIL", core_test);
    int noise_test = p2p_bench_noise_selftest();
    shell_print(sh, "Noise core  : %s selftest=%s (%d)",
                p2p_bench_noise_core_ready() ? "XX+identity linked" : "not linked",
                noise_test == 0 ? "PASS" : "FAIL", noise_test);
    shell_print(sh, "P2P backend : %s",
                p2p_bench_protocol_ready() ? "native STCP_P2P ready" :
                                             "protocol not ready (Yamux/ping pending)");
    shell_print(sh, "Peer        : %s:%s", p2p_cfg.host, p2p_cfg.port);
    shell_print(sh, "Payload     : %u bytes", p2p_cfg.payload_bytes);
    shell_print(sh, "Total       : %u bytes", p2p_cfg.total_bytes);
    shell_print(sh, "Timeout     : %u ms", p2p_cfg.timeout_ms);
    shell_print(sh, "Reference   : rust-libp2p + Noise + Yamux over STCP (golden)");
    return 0;
}

static int cmd_p2p_host(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_p2p_config();
    if (strlen(argv[1]) >= sizeof(p2p_cfg.host)) {
        shell_error(sh, "P2P host is too long");
        return -ENAMETOOLONG;
    }
    strcpy(p2p_cfg.host, argv[1]);
    shell_print(sh, "P2P peer host = %s", p2p_cfg.host);
    return 0;
}

static int cmd_p2p_port(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t port;
    ARG_UNUSED(argc);
    ensure_p2p_config();
    if (parse_u32(sh, argv[1], 1, 65535, &port) < 0) return -EINVAL;
    snprintk(p2p_cfg.port, sizeof(p2p_cfg.port), "%u", port);
    shell_print(sh, "P2P peer port = %s", p2p_cfg.port);
    return 0;
}

static int cmd_p2p_payload(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_p2p_config();
    if (parse_u32(sh, argv[1], 1, 65536, &p2p_cfg.payload_bytes) < 0)
        return -EINVAL;
    shell_print(sh, "P2P payload = %u", p2p_cfg.payload_bytes);
    return 0;
}

static int cmd_p2p_total(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_p2p_config();
    if (parse_u32(sh, argv[1], 1, 2147483647U, &p2p_cfg.total_bytes) < 0)
        return -EINVAL;
    shell_print(sh, "P2P total = %u", p2p_cfg.total_bytes);
    return 0;
}

static int cmd_p2p_timeout(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ensure_p2p_config();
    if (parse_u32(sh, argv[1], 1000, 600000, &p2p_cfg.timeout_ms) < 0)
        return -EINVAL;
    shell_print(sh, "P2P timeout = %u ms", p2p_cfg.timeout_ms);
    return 0;
}

static int cmd_p2p_noise(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    ensure_p2p_config();
    return p2p_noise_probe_run(sh, &p2p_cfg);
}

static int cmd_p2p_ping(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t count = 4U;
    if (argc > 1 && parse_u32(sh, argv[1], 1, 100000, &count) < 0)
        return -EINVAL;
    return queue_p2p_job(sh, P2P_JOB_PING, count);
}

static int cmd_p2p_upload(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return queue_p2p_job(sh, P2P_JOB_UPLOAD, 0U); }
static int cmd_p2p_download(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return queue_p2p_job(sh, P2P_JOB_DOWNLOAD, 0U); }
static int cmd_p2p_full(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return queue_p2p_job(sh, P2P_JOB_FULL, 0U); }
static int cmd_p2p_all(const struct shell *sh, size_t argc, char **argv)
{ ARG_UNUSED(argc); ARG_UNUSED(argv); return queue_p2p_job(sh, P2P_JOB_ALL, 0U); }

SHELL_STATIC_SUBCMD_SET_CREATE(p2p_bench_cmds,
    SHELL_CMD(upload, NULL, "Run native STCP_P2P upload benchmark", cmd_p2p_upload),
    SHELL_CMD(download, NULL, "Run native STCP_P2P download benchmark", cmd_p2p_download),
    SHELL_CMD(full, NULL, "Run native STCP_P2P full-duplex benchmark", cmd_p2p_full),
    SHELL_CMD(all, NULL, "Run all native STCP_P2P throughput benchmarks", cmd_p2p_all),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(p2p_cmds,
    SHELL_CMD(show, NULL, "Show STCP_P2P benchmark configuration/backend", cmd_p2p_show),
    SHELL_CMD_ARG(host, NULL, "Set peer host: stcp p2p host <ip|name>", cmd_p2p_host, 2, 0),
    SHELL_CMD_ARG(port, NULL, "Set peer port: stcp p2p port <1..65535>", cmd_p2p_port, 2, 0),
    SHELL_CMD_ARG(payload, NULL, "Set payload bytes", cmd_p2p_payload, 2, 0),
    SHELL_CMD_ARG(total, NULL, "Set total benchmark bytes", cmd_p2p_total, 2, 0),
    SHELL_CMD_ARG(timeout, NULL, "Set operation timeout ms", cmd_p2p_timeout, 2, 0),
    SHELL_CMD(noise, NULL, "Run golden-compatible multistream + Noise XX probe", cmd_p2p_noise),
    SHELL_CMD_ARG(ping, NULL, "Run peer ping: stcp p2p ping [count]", cmd_p2p_ping, 1, 1),
    SHELL_CMD(bench, &p2p_bench_cmds, "Native STCP_P2P throughput benchmarks", NULL),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(stcp_cmds,
    SHELL_CMD(p2p, &p2p_cmds, "STCP P2P benchmark and diagnostics", NULL),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(stcp, &stcp_cmds, "STCP native P2P", NULL);
