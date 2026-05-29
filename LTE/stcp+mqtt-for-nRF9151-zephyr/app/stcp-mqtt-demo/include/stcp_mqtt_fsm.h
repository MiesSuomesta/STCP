#pragma once

enum mqtt_state {
    MQTT_STATE_INITIAL,
    MQTT_STATE_IDLE,
    MQTT_STATE_CONNACK,
    MQTT_STATE_CONNECT,
    MQTT_STATE_SUBSCRIBE,
    MQTT_STATE_RUNNING,
    MQTT_STATE_DISCONNECT,
    //MQTT_STATE_RECONNECT,
    MQTT_STATE_WAIT_RETRY,
    MQTT_STATE_WAIT_API
};

enum mqtt_connection_state {
    MQTT_CONNECTION_STATE_CONNECT,
    MQTT_CONNECTION_STATE_CONNECTING,
    MQTT_CONNECTION_STATE_DISCONNECTED
};

int stcp_mqtt_do_single_input_event(struct mqtt_client *client, int run_input, int run_live, int *input_ret, int *live_ret);
int stcp_mqtt_get_connak_event_seen(struct stcp_api *api);
