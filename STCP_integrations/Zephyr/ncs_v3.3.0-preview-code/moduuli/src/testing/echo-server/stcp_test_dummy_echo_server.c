#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <errno.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/debug.h>

#define LOGTAG     "[STCP/Test/DummyEcho] "
#include "stcp_testing_bplate.h"
#include "status-monitor.h"

#define STCP_MASTER_SERVER_WORKER_STACK 2048
#define STCP_MASTER_SERVER_WORKER_PRIO  6
#define STCP_MASTER_SERVER_MAX_WORKERS  8

// 1kb
#define STCP_MASTER_SERVER_WORKER_BUF_SIZE (1*1024)

struct stcp_master_worker {
    struct k_thread thread;
    struct k_mutex mutex;
    int client_fd;
    int worker_nr;
    bool used;
};

static struct stcp_master_worker work_slots[STCP_MASTER_SERVER_MAX_WORKERS];

int stcp_worker_get_free_slot() {
    int slot = 0;
    while (slot < STCP_MASTER_SERVER_MAX_WORKERS) {
        if (! work_slots[slot].used ) {
            return slot;
        }
        slot++;
    }
    return -1; // All slots taken
}

K_THREAD_STACK_ARRAY_DEFINE(
    stcp_master_server_worker_stacks,
    STCP_MASTER_SERVER_MAX_WORKERS,
    STCP_MASTER_SERVER_WORKER_STACK
);

static struct k_thread stcp_master_server_workers[STCP_MASTER_SERVER_MAX_WORKERS];
static atomic_t worker_index;

static void stcp_echo_server_worker(void *p1, void *p2, void *p3) {
    struct stcp_master_worker *workerData = p1;
    int32_t client_fd = workerData->client_fd;
    uint8_t buf[STCP_MASTER_SERVER_WORKER_BUF_SIZE];
    int ret = 0;
    int wid = workerData->worker_nr;
    
    struct stcp_server_stats *pStatistics = 
        stcp_server_statistics_get_ptr();
    
    TINF("Worker[%d] Client connected fd=%d", wid, client_fd);
    k_mutex_init(&workerData->mutex);

    while (1) {
        ret = stcp_recv(client_fd, buf, sizeof(buf), 0);

        if (ret > 0) {
            bool statistic_request = stcp_testing_server_check_for_command(
                buf,
                STCP_MASTER_SERVER_STATISTIC_CMD,
                sizeof(STCP_MASTER_SERVER_STATISTIC_CMD)
            );
            if (statistic_request) {
                TINFBIG("Doing statistic report....");
                stcp_testing_statistics_send_to(client_fd);
                continue;
            }
        }

        if (ret > 0) {
            int sent = stcp_send(client_fd, buf, ret, 0);
            if (sent < 0) {
                TERR("Worker[%d] send error (%d)", wid, sent);
                break;
            }
            stcp_testing_statistics_lock();
                pStatistics->rx_bytes += ret;
                pStatistics->tx_bytes += sent;
                pStatistics->messages++;
            stcp_testing_statistics_unlock();
        }
        else if (ret == 0) {
            TINF("Worker[%d] Client disconnected", wid);
            break;
        } else {
            stcp_testing_statistics_lock();
                pStatistics->errors++;
            stcp_testing_statistics_unlock();
            TERR("Worker[%d] recv error (%d) count=%llu", wid, ret, pStatistics->errors);
            break;
        }
    }


    stcp_testing_statistics_lock();
        if (pStatistics->running > 0)
            pStatistics->running--;

        workerData->used = false;
    stcp_testing_statistics_unlock();

    stcp_close(client_fd);
    TINF("Worker[%d] Session thread closed", wid);
}

void stcp_echo_server_start(void)
{
    int server_fd;
    int client_fd;
    int ret;

    struct stcp_server_stats *statistics = 
        stcp_server_statistics_get_ptr();

    struct sockaddr_in addr;
    TINF("STCP master echo server starting (port=%d, uptime: %llu)",
        CONFIG_STCP_TESTING_PEER_PORT_TO_CONNECT,
        statistics->start_time
    );
    
    server_fd = stcp_socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        TERR("socket failed (%d)", server_fd);
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONFIG_STCP_MODEM_ECHO_SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    ret = stcp_bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        TERR("bind failed (%d)", ret);
        return;
    }

    ret = stcp_listen(server_fd, 1);
    if (ret < 0) {
        TERR("listen failed (%d)", ret);
        return;
    }

    TINF("Listening on port %d", CONFIG_STCP_MODEM_ECHO_SERVER_PORT);

    while (1) {
        client_fd = stcp_accept(server_fd, NULL, NULL, NULL);
        if (client_fd < 0) {
            TERR("accept failed (%d)", client_fd);
            continue;
        }

        bool rejected = 0;
        stcp_testing_statistics_lock();
            rejected = (statistics->running >= STCP_MASTER_SERVER_MAX_WORKERS);
            if (rejected) {
                statistics->rejected++;
            } else {
                statistics->connections++;
                statistics->running++;
            }

            if (statistics->max_running < statistics->running)
                statistics->max_running = statistics->running;
        stcp_testing_statistics_unlock();

        if (rejected) {
            TWRN("Running rejected, all workers running... (Client: %d)", client_fd);
            stcp_close(client_fd);
            continue;
        }
        // Liipasta workkeri
        // Liipasta workkeri lukon sisällä => VARMASTI menee oikein
        stcp_testing_statistics_lock();
            int32_t worker_idx = stcp_worker_get_free_slot();
            TINF("Got free slot %d", worker_idx);
            if (worker_idx < 0) {
                TWRN("No free slots...");
            } else {
                
                work_slots[worker_idx].used = true;
                work_slots[worker_idx].worker_nr = worker_idx;

                k_thread_create(
                    &stcp_master_server_workers[worker_idx],
                    stcp_master_server_worker_stacks[worker_idx],
                    STCP_MASTER_SERVER_WORKER_STACK,
                    stcp_echo_server_worker,
                    (void *)&work_slots[worker_idx],
                    NULL,
                    NULL,
                    STCP_MASTER_SERVER_WORKER_PRIO,
                    0,
                    K_NO_WAIT
                );
            }
        stcp_testing_statistics_unlock();
        TINF("Worker[%u] started at %llu ..", worker_idx, k_uptime_get());

    }
}
