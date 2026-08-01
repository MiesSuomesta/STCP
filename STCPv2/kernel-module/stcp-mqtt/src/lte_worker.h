#ifndef STCP_LTE_WORKER_H_
#define STCP_LTE_WORKER_H_

#include <zephyr/kernel.h>

int stcp_lte_worker_init(void);
int stcp_lte_worker_schedule(k_timeout_t delay);
void stcp_lte_worker_cancel(void);
bool stcp_lte_worker_pending(void);

#endif /* STCP_LTE_WORKER_H_ */
