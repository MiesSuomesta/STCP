#pragma once
#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>


struct stcp_mutex {
    struct k_mutex mutex;

    void *last_lr_lock;
    void *last_lr_unlock;

    k_tid_t owner;

    uint32_t last_lock_ts;
};

int stcp_mutex_lock(struct stcp_mutex *lock, int timeout_ms);
int stcp_mutex_unlock(struct stcp_mutex *lock);
int stcp_mutex_init(struct stcp_mutex *newMutex);
