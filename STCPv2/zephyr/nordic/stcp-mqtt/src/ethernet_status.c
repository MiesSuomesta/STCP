#include "ethernet_status.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(ethernet_status, LOG_LEVEL_INF);

static const char *format_mac(const struct net_linkaddr *link, char *buffer,
                              size_t buffer_size)
{
    if (link == NULL || link->addr == NULL || link->len < 6U) {
        snprintf(buffer, buffer_size, "unavailable");
        return buffer;
    }

    snprintf(buffer, buffer_size, "%02x:%02x:%02x:%02x:%02x:%02x",
             link->addr[0], link->addr[1], link->addr[2],
             link->addr[3], link->addr[4], link->addr[5]);
    return buffer;
}

static const char *current_ipv4(struct net_if *iface, char *buffer,
                                size_t buffer_size)
{
    const struct in_addr *address;

    address = net_if_ipv4_get_global_addr(iface, NET_ADDR_PREFERRED);
    if (address == NULL || net_addr_ntop(AF_INET, address, buffer, buffer_size) == NULL) {
        snprintf(buffer, buffer_size, "unassigned");
    }

    return buffer;
}

int ethernet_status_show(const struct shell *shell)
{
    struct net_if *iface = net_if_get_default();
    char mac[24];
    char ipv4[NET_IPV4_ADDR_LEN];

    if (iface == NULL) {
        shell_error(shell, "No default network interface");
        return -ENODEV;
    }

    shell_print(shell, "=== ETHERNET STATUS ===");
    shell_print(shell, "Interface : %d", net_if_get_by_iface(iface));
    shell_print(shell, "Admin up  : %s", net_if_is_up(iface) ? "yes" : "no");
    shell_print(shell, "Carrier   : %s", net_if_is_carrier_ok(iface) ? "up" : "down");
    shell_print(shell, "MAC       : %s",
                format_mac(net_if_get_link_addr(iface), mac, sizeof(mac)));
    shell_print(shell, "IPv4      : %s", current_ipv4(iface, ipv4, sizeof(ipv4)));
    shell_print(shell, "Netmask   : %s", CONFIG_NET_CONFIG_MY_IPV4_NETMASK);
    shell_print(shell, "Gateway   : %s", CONFIG_NET_CONFIG_MY_IPV4_GW);
    shell_print(shell, "DHCP      : disabled");
    shell_print(shell, "SPI limit : 8 MHz");
    shell_print(shell, "=======================");
    return 0;
}

void ethernet_status_log_startup(void)
{
    struct net_if *iface = net_if_get_default();
    char mac[24];
    char ipv4[NET_IPV4_ADDR_LEN];

    if (iface == NULL) {
        LOG_ERR("No default Ethernet interface");
        return;
    }

    LOG_INF("Ethernet iface=%d admin=%s carrier=%s mac=%s",
            net_if_get_by_iface(iface),
            net_if_is_up(iface) ? "up" : "down",
            net_if_is_carrier_ok(iface) ? "up" : "down",
            format_mac(net_if_get_link_addr(iface), mac, sizeof(mac)));
    LOG_INF("Ethernet IPv4=%s netmask=%s gateway=%s DHCP=off SPI=8MHz",
            current_ipv4(iface, ipv4, sizeof(ipv4)),
            CONFIG_NET_CONFIG_MY_IPV4_NETMASK,
            CONFIG_NET_CONFIG_MY_IPV4_GW);
}
