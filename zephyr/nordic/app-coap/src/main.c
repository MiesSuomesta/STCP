#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/coap.h>
#include <zephyr/random/random.h>

#include "coap_stcp_transport.h"

LOG_MODULE_REGISTER(stcp_coap_app, LOG_LEVEL_INF);

static uint8_t tx_buf[512];
static uint8_t rx_buf[1024];

static int build_get_request(uint8_t *buf,
                             size_t buf_len,
                             uint16_t message_id,
                             uint8_t token[4])
{
    struct coap_packet request;
    int rc;

    rc = coap_packet_init(&request,
                          buf,
                          buf_len,
                          1,
                          COAP_TYPE_CON,
                          4,
                          token,
                          COAP_METHOD_GET,
                          message_id);
    if (rc < 0) {
        return rc;
    }

    rc = coap_packet_append_option(&request,
                                   COAP_OPTION_URI_PATH,
                                   CONFIG_STCP_COAP_PATH,
                                   strlen(CONFIG_STCP_COAP_PATH));
    if (rc < 0) {
        return rc;
    }

    return (int)request.offset;
}

static int parse_response(const uint8_t *buf, size_t len)
{
    struct coap_packet response;
    const uint8_t *payload;
    uint16_t payload_len = 0U;
    uint8_t code;
    int rc;

    rc = coap_packet_parse(&response,
                           (uint8_t *)buf,
                           len,
                           NULL,
                           0);
    if (rc < 0) {
        LOG_ERR("coap_packet_parse failed: %d", rc);
        return rc;
    }

    code = coap_header_get_code(&response);

    payload = coap_packet_get_payload(&response, &payload_len);

    LOG_INF("CoAP response code=%u.%02u payload_len=%u",
            (unsigned int)(code >> 5),
            (unsigned int)(code & 0x1f),
            (unsigned int)payload_len);

    if (payload != NULL && payload_len > 0U) {
        LOG_INF("CoAP payload: %.*s",
                (int)payload_len,
                (const char *)payload);
    }

    return 0;
}

int main(void)
{
    uint32_t sequence = 1U;

    LOG_INF("Zephyr CoAP over STCP starting");
    LOG_INF("gateway=%s:%d path=/%s",
            CONFIG_STCP_COAP_SERVER_IPV4,
            CONFIG_STCP_COAP_SERVER_PORT,
            CONFIG_STCP_COAP_PATH);

    while (true) {
        uint8_t token[4];
        uint16_t message_id;
        int request_len;
        int rc;

        rc = coap_stcp_connect();
        if (rc < 0) {
            LOG_ERR("coap_stcp_connect failed: %d", rc);
            goto reconnect;
        }

        message_id = (uint16_t)(sys_rand32_get() & 0xffffU);
        token[0] = 'S';
        token[1] = 'T';
        token[2] = (uint8_t)((sequence >> 8) & 0xffU);
        token[3] = (uint8_t)(sequence & 0xffU);

        request_len = build_get_request(tx_buf,
                                        sizeof(tx_buf),
                                        message_id,
                                        token);
        if (request_len < 0) {
            LOG_ERR("CoAP request build failed: %d", request_len);
            coap_stcp_close();
            goto reconnect;
        }

        LOG_INF("CoAP GET /%s id=%u bytes=%d",
                CONFIG_STCP_COAP_PATH,
                message_id,
                request_len);

        rc = coap_stcp_send(tx_buf, (size_t)request_len);
        if (rc < 0) {
            LOG_ERR("CoAP send failed: %d", rc);
            coap_stcp_close();
            goto reconnect;
        }

        rc = coap_stcp_recv(rx_buf, sizeof(rx_buf));
        if (rc <= 0) {
            LOG_ERR("CoAP recv failed: %d", rc);
            coap_stcp_close();
            goto reconnect;
        }

        LOG_INF("CoAP response bytes=%d", rc);

        rc = parse_response(rx_buf, (size_t)rc);
        if (rc < 0) {
            coap_stcp_close();
            goto reconnect;
        }

        sequence++;
        k_sleep(K_MSEC(CONFIG_STCP_COAP_INTERVAL_MS));
        continue;

reconnect:
        LOG_WRN("CoAP reconnect in %d ms",
                CONFIG_STCP_COAP_RECONNECT_DELAY_MS);
        coap_stcp_close();
        k_sleep(K_MSEC(CONFIG_STCP_COAP_RECONNECT_DELAY_MS));
    }

    return 0;
}
