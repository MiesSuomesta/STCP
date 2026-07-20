#ifndef MODEM_STATUS_H_
#define MODEM_STATUS_H_

#include <zephyr/shell/shell.h>

int modem_status_system(const struct shell *sh);
int modem_status_health(const struct shell *sh);
int modem_status_signal(const struct shell *sh);
int modem_status_network(const struct shell *sh);
int modem_status_band(const struct shell *sh);
int modem_status_packet(const struct shell *sh);
int modem_status_sleep(const struct shell *sh);
int modem_status_apn(const struct shell *sh);
int modem_status_all(const struct shell *sh);

#endif /* MODEM_STATUS_H_ */
