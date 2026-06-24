#pragma once
#include <zephyr/kernel.h>
#include <stdbool.h>

#define STCP_SOCKET_INTERNAL 1
#include <stcp/settings.h>
#include <stcp/debug.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>
#include <stcp/utils.h>
#include <stcp/stcp_net.h>
#include <stcp/fsm.h>
#include <stcp/workers.h>
#include <stcp/stcp_operations_zephyr.h>
#include <stcp/stcp_rx_transmission.h>
#include <stcp/low_level_socks.h>

#include <stcp/stcp_mutex.h>

// 3kb
#define STCP_FSM_THREAD_STACK_SIZE      (3*1024)
/*

#define STCP_CTX_MAGIC_ALIVE            0x53544350
#define STCP_CTX_MAGIC_ALIVE_FOOTER     0x50433553
#define STCP_CTX_MAGIC_POISON           0xDEADBEEF

*/
struct stcp_api {
    uint32_t magic; // Conteksti magikki

    struct stcp_mutex lock;
    atomic_t alive;
    struct k_sem connected_sem;
    atomic_t connected;
    int    nonblocking;
    struct stcp_ctx *ctx;
    struct stcp_fsm *fsm;
    int    connection_timeouts;

    int    connack_reset_done;

    int    connack_seen;
    struct k_sem connack_sem;
    
    K_KERNEL_STACK_MEMBER(
        thread_stack,
        STCP_FSM_THREAD_STACK_SIZE
    );

    atomic_t thread_running;
    struct k_thread thread_data;
    k_tid_t thread_tid;

    atomic_t connect_in_progress;
    uint32_t magic_footer; // Conteksti magikki
} __aligned(16);

