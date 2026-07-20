#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/lte_lc.h>
LOG_MODULE_REGISTER(main,3);
int mqtt_proxy_run(void);
int main(void){LOG_INF("STCP2 MQTT proxy starting");int rc=lte_lc_connect();if(rc){LOG_ERR("LTE connect failed: %d",rc);return 0;}LOG_INF("LTE ready");return mqtt_proxy_run();}
