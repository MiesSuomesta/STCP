#ifndef STCP_DEBUG_H
#define STCP_DEBUG_H

#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

/*
 * KISS debug policy:
 * - real errors stay visible with LOG_ERR()
 * - verbose diagnostics compile out unless both the master and category
 *   switches are enabled
 * - no runtime framework, buffers or branches are added
 */
#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_POLL)
#define STCP_POLL_INF(...)  LOG_INF(__VA_ARGS__)
#define STCP_POLL_WRN(...)  LOG_WRN(__VA_ARGS__)
#define STCP_POLL_DBG(...)  LOG_DBG(__VA_ARGS__)
#define STCP_POLL_PRINTK(...) printk(__VA_ARGS__)
#else
#define STCP_POLL_INF(...)  do { } while (0)
#define STCP_POLL_WRN(...)  do { } while (0)
#define STCP_POLL_DBG(...)  do { } while (0)
#define STCP_POLL_PRINTK(...) do { } while (0)
#endif

#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_RX)
#define STCP_RX_INF(...)    LOG_INF(__VA_ARGS__)
#define STCP_RX_WRN(...)    LOG_WRN(__VA_ARGS__)
#define STCP_RX_DBG(...)    LOG_DBG(__VA_ARGS__)
#define STCP_RX_HEXDUMP(data, len, text) LOG_HEXDUMP_DBG(data, len, text)
#else
#define STCP_RX_INF(...)    do { } while (0)
#define STCP_RX_WRN(...)    do { } while (0)
#define STCP_RX_DBG(...)    do { } while (0)
#define STCP_RX_HEXDUMP(data, len, text) do { } while (0)
#endif

#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_TX)
#define STCP_TX_INF(...)    LOG_INF(__VA_ARGS__)
#define STCP_TX_DBG(...)    LOG_DBG(__VA_ARGS__)
#define STCP_TX_HEXDUMP(data, len, text) LOG_HEXDUMP_DBG(data, len, text)
#else
#define STCP_TX_INF(...)    do { } while (0)
#define STCP_TX_DBG(...)    do { } while (0)
#define STCP_TX_HEXDUMP(data, len, text) do { } while (0)
#endif

#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_CARRIER)
#define STCP_CARRIER_INF(...) LOG_INF(__VA_ARGS__)
#define STCP_CARRIER_WRN(...) LOG_WRN(__VA_ARGS__)
#define STCP_CARRIER_DBG(...) LOG_DBG(__VA_ARGS__)
#else
#define STCP_CARRIER_INF(...) do { } while (0)
#define STCP_CARRIER_WRN(...) do { } while (0)
#define STCP_CARRIER_DBG(...) do { } while (0)
#endif

#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_SOCKET)
#define STCP_SOCKET_INF(...) LOG_INF(__VA_ARGS__)
#define STCP_SOCKET_DBG(...) LOG_DBG(__VA_ARGS__)
#else
#define STCP_SOCKET_INF(...) do { } while (0)
#define STCP_SOCKET_DBG(...) do { } while (0)
#endif

#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_HANDSHAKE)
#define STCP_HANDSHAKE_INF(...) LOG_INF(__VA_ARGS__)
#define STCP_HANDSHAKE_DBG(...) LOG_DBG(__VA_ARGS__)
#else
#define STCP_HANDSHAKE_INF(...) do { } while (0)
#define STCP_HANDSHAKE_DBG(...) do { } while (0)
#endif

#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_CRYPTO)
#define STCP_CRYPTO_INF(...) LOG_INF(__VA_ARGS__)
#define STCP_CRYPTO_WRN(...) LOG_WRN(__VA_ARGS__)
#define STCP_CRYPTO_DBG(...) LOG_DBG(__VA_ARGS__)
#define STCP_CRYPTO_PRINTK(...) printk(__VA_ARGS__)
#define STCP_CRYPTO_HEXDUMP(data, len, text) LOG_HEXDUMP_INF(data, len, text)
#else
#define STCP_CRYPTO_INF(...) do { } while (0)
#define STCP_CRYPTO_WRN(...) do { } while (0)
#define STCP_CRYPTO_DBG(...) do { } while (0)
#define STCP_CRYPTO_PRINTK(...) do { } while (0)
#define STCP_CRYPTO_HEXDUMP(data, len, text) do { } while (0)
#endif

#if defined(CONFIG_STCP_DEBUG) && defined(CONFIG_STCP_DEBUG_MQTT)
#define STCP_MQTT_INF(...)  LOG_INF(__VA_ARGS__)
#define STCP_MQTT_WRN(...)  LOG_WRN(__VA_ARGS__)
#define STCP_MQTT_DBG(...)  LOG_DBG(__VA_ARGS__)
#define STCP_MQTT_HEXDUMP(data, len, text) LOG_HEXDUMP_DBG(data, len, text)
#else
#define STCP_MQTT_INF(...)  do { } while (0)
#define STCP_MQTT_WRN(...)  do { } while (0)
#define STCP_MQTT_DBG(...)  do { } while (0)
#define STCP_MQTT_HEXDUMP(data, len, text) do { } while (0)
#endif

#endif /* STCP_DEBUG_H */
