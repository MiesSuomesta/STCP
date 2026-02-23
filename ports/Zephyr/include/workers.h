#pragma once
#include <zephyr/kernel.h>

void worker_cleanup_work_handler(struct k_work *work);
void worker_context_init(struct stcp_ctx *ctx);

void worker_schedule_cleanup(struct stcp_ctx * ctx);
void worker_cleanup_work_handler(struct k_work *work);