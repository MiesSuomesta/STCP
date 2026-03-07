#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(net_mqtt_sock_statistics, CONFIG_MQTT_LOG_LEVEL);

#include <errno.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>


#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>

#include <stcp/stcp_transport_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>

#include <stcp/debug.h>
#include <stcp/mqtt_stcp_stats.h>

struct mqtt_stcp_stats g_mqtt_stcp_stats = { 0 };

void mqtt_stats_dump(void)
{
    LDBGBIG("MQTT STCP STATS\n");
    LDBG(   "   TX bytes: %llu\n", g_mqtt_stcp_stats.tx_bytes);
    LDBG(   "   RX bytes: %llu\n", g_mqtt_stcp_stats.rx_bytes);
    LDBG(   "   TX calls: %u\n", g_mqtt_stcp_stats.tx_calls);
    LDBG(   "   RX calls: %u\n", g_mqtt_stcp_stats.rx_calls);
    LDBG(   "   TX EAGAIN: %u\n", g_mqtt_stcp_stats.tx_eagain);
    LDBG(   "   RX EAGAIN: %u\n", g_mqtt_stcp_stats.rx_eagain);
    LDBG(   "   Connects: %u\n", g_mqtt_stcp_stats.reconnects);
    LDBG(   "   Reconnects: %u\n", g_mqtt_stcp_stats.reconnects);
}
