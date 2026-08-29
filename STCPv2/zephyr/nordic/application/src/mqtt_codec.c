#include "mqtt_codec.h"
#include <errno.h>
#include <string.h>
#include <zephyr/net/socket.h>

static int send_all(int fd, const void *buf, size_t len)
{
    const uint8_t *p = buf;

    while (len) {
        ssize_t n = zsock_send(fd, p, len, 0);
        if (n < 0) {
            return -errno;
        }
        if (n == 0) {
            return -ECONNRESET;
        }
        p += (size_t)n;
        len -= (size_t)n;
    }
    return 0;
}

static size_t enc_rem(uint8_t *out, size_t value)
{
    size_t n = 0;

    do {
        uint8_t d = (uint8_t)(value % 128U);
        value /= 128U;
        if (value) {
            d |= 0x80U;
        }
        out[n++] = d;
    } while (value);

    return n;
}

static int send_mqtt_header(int fd, uint8_t type_flags, size_t remaining)
{
    uint8_t hdr[5];
    size_t n;

    hdr[0] = type_flags;
    n = 1U + enc_rem(&hdr[1], remaining);
    return send_all(fd, hdr, n);
}

static int send_mqtt_str(int fd, const char *s)
{
    size_t len = strlen(s);
    uint8_t prefix[2];
    int rc;

    if (len > UINT16_MAX) {
        return -EMSGSIZE;
    }

    prefix[0] = (uint8_t)(len >> 8);
    prefix[1] = (uint8_t)len;

    rc = send_all(fd, prefix, sizeof(prefix));
    if (rc < 0) {
        return rc;
    }
    return send_all(fd, s, len);
}

int mqtt_send_connect(int fd, const char *id, uint16_t ka)
{
    static const uint8_t variable_header[] = {
        0x00, 0x04, 'M', 'Q', 'T', 'T',
        0x04, 0x02, 0x00, 0x00
    };
    uint8_t vh[sizeof(variable_header)];
    size_t id_len = strlen(id);
    size_t remaining;
    int rc;

    if (id_len > UINT16_MAX) {
        return -EMSGSIZE;
    }

    remaining = sizeof(vh) + 2U + id_len;
    rc = send_mqtt_header(fd, 0x10, remaining);
    if (rc < 0) {
        return rc;
    }

    memcpy(vh, variable_header, sizeof(vh));
    vh[8] = (uint8_t)(ka >> 8);
    vh[9] = (uint8_t)ka;

    rc = send_all(fd, vh, sizeof(vh));
    if (rc < 0) {
        return rc;
    }
    return send_mqtt_str(fd, id);
}

int mqtt_send_subscribe(int fd, const char *topic, uint16_t id)
{
    size_t topic_len = strlen(topic);
    size_t remaining;
    uint8_t packet_id[2] = {
        (uint8_t)(id >> 8),
        (uint8_t)id,
    };
    const uint8_t qos = 0;
    int rc;

    if (topic_len > UINT16_MAX) {
        return -EMSGSIZE;
    }

    remaining = sizeof(packet_id) + 2U + topic_len + sizeof(qos);
    rc = send_mqtt_header(fd, 0x82, remaining);
    if (rc < 0) {
        return rc;
    }
    rc = send_all(fd, packet_id, sizeof(packet_id));
    if (rc < 0) {
        return rc;
    }
    rc = send_mqtt_str(fd, topic);
    if (rc < 0) {
        return rc;
    }
    return send_all(fd, &qos, sizeof(qos));
}

int mqtt_send_publish_qos1(int fd, const char *topic, const void *payload,
                           size_t len, uint16_t id)
{
    size_t topic_len = strlen(topic);
    size_t remaining;
    uint8_t packet_id[2] = {
        (uint8_t)(id >> 8),
        (uint8_t)id,
    };
    int rc;

    if (topic_len > UINT16_MAX) {
        return -EMSGSIZE;
    }

    /*
     * KISS / low-stack path:
     * send MQTT's logical packet as header + topic + packet-id + payload.
     * TCP/STCP is a byte stream, so no 768+773 byte assembly buffers are
     * required. Wire bytes remain exactly the same.
     */
    remaining = 2U + topic_len + sizeof(packet_id) + len;

    rc = send_mqtt_header(fd, 0x32, remaining);
    if (rc < 0) {
        return rc;
    }
    rc = send_mqtt_str(fd, topic);
    if (rc < 0) {
        return rc;
    }
    rc = send_all(fd, packet_id, sizeof(packet_id));
    if (rc < 0) {
        return rc;
    }
    return send_all(fd, payload, len);
}
