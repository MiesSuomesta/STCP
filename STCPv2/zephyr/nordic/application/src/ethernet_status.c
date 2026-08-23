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

static const struct net_in_addr *current_ipv4_addr(struct net_if *iface)
{
    return net_if_ipv4_get_global_addr(iface, NET_ADDR_PREFERRED);
}

static void format_ipv4_runtime(struct net_if *iface,
                                char *address_buffer,
                                size_t address_buffer_size,
                                char *netmask_buffer,
                                size_t netmask_buffer_size,
                                char *gateway_buffer,
                                size_t gateway_buffer_size)
{
    const struct net_in_addr *address;
    struct net_in_addr netmask;
    struct net_in_addr gateway;

    snprintf(address_buffer, address_buffer_size, "unassigned");
    snprintf(netmask_buffer, netmask_buffer_size, "unavailable");
    snprintf(gateway_buffer, gateway_buffer_size, "unavailable");

    address = current_ipv4_addr(iface);
    if (address != NULL) {
        if (net_addr_ntop(AF_INET, address,
                          address_buffer, address_buffer_size) == NULL) {
            snprintf(address_buffer, address_buffer_size, "unassigned");
        }

        netmask = net_if_ipv4_get_netmask_by_addr(iface, address);
        if (net_addr_ntop(AF_INET, &netmask,
                          netmask_buffer, netmask_buffer_size) == NULL) {
            snprintf(netmask_buffer, netmask_buffer_size, "unavailable");
        }
    }

    gateway = net_if_ipv4_get_gw(iface);
    if (net_addr_ntop(AF_INET, &gateway,
                      gateway_buffer, gateway_buffer_size) == NULL) {
        snprintf(gateway_buffer, gateway_buffer_size, "unavailable");
    }
}

int ethernet_status_show(const struct shell *shell)
{
    struct net_if *iface = net_if_get_default();
    char mac[24];
    char ipv4[NET_IPV4_ADDR_LEN];
    char netmask[NET_IPV4_ADDR_LEN];
    char gateway[NET_IPV4_ADDR_LEN];

    if (iface == NULL) {
        shell_error(shell, "No default network interface");
        return -ENODEV;
    }

    format_mac(net_if_get_link_addr(iface), mac, sizeof(mac));
    format_ipv4_runtime(iface,
                        ipv4, sizeof(ipv4),
                        netmask, sizeof(netmask),
                        gateway, sizeof(gateway));

    shell_print(shell, "=== ETHERNET STATUS ===");
    shell_print(shell, "Interface : %d", net_if_get_by_iface(iface));
    shell_print(shell, "Admin up  : %s", net_if_is_up(iface) ? "yes" : "no");
    shell_print(shell, "Carrier   : %s", net_if_is_carrier_ok(iface) ? "up" : "down");
    shell_print(shell, "MAC       : %s", mac);
    shell_print(shell, "IPv4      : %s", ipv4);
    shell_print(shell, "Netmask   : %s", netmask);
    shell_print(shell, "Gateway   : %s", gateway);
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
    char netmask[NET_IPV4_ADDR_LEN];
    char gateway[NET_IPV4_ADDR_LEN];

    if (iface == NULL) {
        LOG_ERR("No default Ethernet interface");
        return;
    }

    format_mac(net_if_get_link_addr(iface), mac, sizeof(mac));
    format_ipv4_runtime(iface,
                        ipv4, sizeof(ipv4),
                        netmask, sizeof(netmask),
                        gateway, sizeof(gateway));

    LOG_INF("Ethernet iface=%d admin=%s carrier=%s mac=%s",
            net_if_get_by_iface(iface),
            net_if_is_up(iface) ? "up" : "down",
            net_if_is_carrier_ok(iface) ? "up" : "down",
            mac);
    LOG_INF("Ethernet IPv4=%s netmask=%s gateway=%s DHCP=off SPI=8MHz",
            ipv4, netmask, gateway);
}
