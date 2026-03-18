#include <stcp_api.h>
#include <stcp/debug.h>
#include <stcp/stcp_socket.h>
#include <stcp/utils.h>

#include "stcp_mqtt.h"
#include "stcp_mqtt_fsm.h"
#include "mqtt_demo_utils.h"


#define MQTT_STACK_SIZE 7168
#define MQTT_PRIORITY 5

enum mqtt_state state = MQTT_STATE_IDLE;
extern struct mqtt_client client;
extern int mqtt_connected;

void stcp_mqtt_task(void *p1, void *p2, void *p3);
extern struct k_mutex client_lock;

K_THREAD_STACK_DEFINE(stpc_mqtt_stack, MQTT_STACK_SIZE);
struct k_thread stpc_mqtt_thread;

// Semaphore for connack

struct k_sem g_sem_connack_seen;

int stcp_mqtt_wait_for_connak_event(int timeout_ms) 
{
    if ( k_sem_count_get(&g_sem_connack_seen) == 0) {
        return 0;
    }
    MINFBIG("CONNACK take (%d ms)", timeout_ms);
    return k_sem_take(&g_sem_connack_seen, K_MSEC(timeout_ms));
}

void stcp_mqtt_set_connak_event_seen()
{
    MINFBIG("CONNACK give..");
    k_sem_give(&g_sem_connack_seen);
}

void stcp_mqtt_reset_connak_event_seen() 
{
    MINFBIG("CONNACK seen reset..");
    k_sem_reset(&g_sem_connack_seen);
}

void stpc_mqtt_init_fsm_thread() {
#if defined(CONFIG_STCP_TESTING) && (CONFIG_STCP_TEST_MODE != 3)
#define STCP_COMPILE_MQTT 0
#else
#define STCP_COMPILE_MQTT 1
#endif

#if STCP_COMPILE_MQTT
    MINFBIG("Initialising & staring MQTT FSM thread...");

    k_sem_init(&g_sem_connack_seen, 0, 1);

    k_thread_create(
        &stpc_mqtt_thread,                 // thread object
        stpc_mqtt_stack,                   // stack
        K_THREAD_STACK_SIZEOF(stpc_mqtt_stack),
        stcp_mqtt_task,               // entry function
        &client,                      // p1
        NULL,                         // p2
        NULL,                         // p3
        MQTT_PRIORITY,                // priority
        0,                            // options
        K_NO_WAIT                     // start delay
    );
#else
    MINFBIG("Testing enabled but not MQTT testing => MQTT FSM not started");
#endif
}

static int stcp_mqtt_do_run_event(struct mqtt_client *client, struct stcp_api *api)
{
    int rc;

    int fd = stcp_api_get_fd(api);

    struct zsock_pollfd fds[1];

    int poll_for = ZSOCK_POLLIN | ZSOCK_POLLERR | ZSOCK_POLLHUP;

    fds[0].fd = fd;
    fds[0].events = poll_for;

    int ret = zsock_poll(fds, 1, STCP_MQTT_POLL_TIMEOUT_MSEC);

    if (ret < 0) {
        LERR("poll error %d", errno);
        return -errno;
    }

    /* timeout -> keepalive */
    if (ret == 0) {
        MDBG("Calling mqtt_live...");
        rc = mqtt_live(client);
        MDBG("Return of mqtt_live, rc: %d / errno: %d", rc, errno);

        if (rc < 0) {
            LERR("mqtt_live error %d", rc);
            return rc;
        }
    }

    if (fds[0].revents & poll_for) {

        MDBG("Calling mqtt_input...");
        rc = mqtt_input(client);
        MDBG("Return of mqtt_input, rc: %d / errno: %d", rc, errno);

        if (rc < 0) {
            LERR("mqtt_input error %d", rc);
            return rc;
        }

    }

    return 0;
}

static const char *get_state_name(enum mqtt_state state) {
#if CONFIG_STCP_DEBUG
    //MDBG("Gettin string for state %d", state);
    switch (state)
    {
        case MQTT_STATE_INITIAL:    return "INITIAL";   
        case MQTT_STATE_IDLE:       return "IDLE";      
        case MQTT_STATE_CONNECT:    return "CONNECT";   
        case MQTT_STATE_SUBSCRIBE:  return "SUBSCRIBE"; 
        case MQTT_STATE_RUNNING:    return "RUNNING";   
        case MQTT_STATE_DISCONNECT: return "DISCONNECT";
        case MQTT_STATE_RECONNECT:  return "RECONNECT"; 
        case MQTT_STATE_WAIT_RETRY: return "WAIT_RETRY";
        default:                    return "UNKNOWN";   
    }
#else
    static char tmp[2];

    tmp[0] = "0";
    tmp[0] += (int)state;
    tmp[1] = 0;

    return tmp;
#endif
}

void stcp_mqtt_task(void *p1, void *p2, void *p3)
{
    int rc;
    struct mqtt_client *clientPtr = p1;
    struct stcp_api *api = clientPtr->transport.stcp.stcp_api_instance;
    static uint32_t loop_cnt = 0;

#if CONFIG_STCP_DEBUG
    static enum mqtt_state last_state = MQTT_STATE_INITIAL;
#endif

    while (1) {

        loop_cnt++;

        //if (loop_cnt % 30 == 0) {
        {
#if CONFIG_STCP_DEBUG
            const char *msg = get_state_name(state);
            MINF("MQTT alive, uptime %u sec, current state %s (%d)", 
                k_uptime_get_32()/1000,
                msg,
                state
            );
#else
            MINF("MQTT alive, uptime %u sec, current state %d", 
                k_uptime_get()/1000,
                state
            );
#endif
        }

#if CONFIG_STCP_DEBUG
        if (state != last_state) {
            MINF("MQTT FSM State change: %s => %s", 
                get_state_name(last_state), 
                get_state_name(state));

            last_state = state;
        } else {
            MWRN("MQTT FSM State: %s", get_state_name(state));
        }
#endif
        switch (state) {

            case MQTT_STATE_INITIAL: 
                MINF("MQTT: @ %s state handler", get_state_name(state));
                state = MQTT_STATE_INITIAL;
                break;

            case MQTT_STATE_IDLE:
                MINF("MQTT: @ %s state handler", get_state_name(state));

                memset(clientPtr, 0, sizeof(*clientPtr));

                stcp_mqtt_reset_connak_event_seen();

                mqtt_client_init(clientPtr);

                state = MQTT_STATE_CONNECT;
                break;

            case MQTT_STATE_CONNECT:

                MINF("MQTT: @ %s state handler", get_state_name(state));

                rc = mqtt_connect_via_stcp("lja.fi", "7777", NULL);

                mqtt_connected = rc == 0;

                if (rc == 0) {
                    MINF("MQTT: connected!");
                    state = MQTT_STATE_SUBSCRIBE;
                } else {
                    MERR("MQTT: connect failed rc=%d errno=%d", rc, errno);
                    state = MQTT_STATE_WAIT_RETRY;
                }


                break;

            case MQTT_STATE_SUBSCRIBE:
                MINF("MQTT: @ %s state handler", get_state_name(state));

                MINF("MQTT: Waiting CONNACK event for %d ms",
                    STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC);
                rc = stcp_mqtt_wait_for_connak_event(
                        STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC
                    );

                if (rc != 0) {
                    // error while waiting for connect => reinit
                    MWRN("Wait for CONNACK timeout => reinit..");
                    state = MQTT_STATE_WAIT_RETRY;
                    break;
                }

                MINF("Starting subscription .....");
                rc = stpc_mqtt_subscribe(clientPtr);
                if (rc < 0) {
                    MWRN("Subscribe failed: %d", rc);
                }

                MINFBIG("Subscription OK");
                state = MQTT_STATE_RUNNING;
                break;


            case MQTT_STATE_RUNNING: {
                static uint32_t last_pub = 0;
                uint32_t now = k_uptime_get_32();
                MINF("MQTT: @ %s state handler", get_state_name(state));

                if (!mqtt_connected) {
                    MWRN("MQTT: connection lost @ running state");
                    state = MQTT_STATE_DISCONNECT;
                    break;
                }

                MINF("MQTT: Doign poll event....");
                rc = stcp_mqtt_do_run_event(clientPtr, api);
                MINF("MQTT: Event ret: %d", rc);

                if (rc < 0) {
                    int isNotError = 0;
                    if (rc == -EINPROGRESS) {
                        MWRN("Client processing ... (EINPROGRESS)");
                        SLEEP_MSEC(100);

                        isNotError = 1;
                    }

                    if (rc == -EAGAIN) {
                        MWRN("Client processing ... (EAGAIN)");
                        SLEEP_MSEC(10);
                        isNotError = 1;
                    }

                    if(!isNotError) {
                        MWRN("Poller ret: %d => State update: Disconnected", rc);
                        state = MQTT_STATE_DISCONNECT;
                        mqtt_connected = 0;
                        break;
                    }

                    /* tarkoituksella, jatkaa debug viestiin.. */
                }
                
                uint32_t diffPub = now - last_pub;
                int publish  = (last_pub == 0);
                    publish |= (diffPub > STCP_MQTT_PUBLISH_INTERVAL_MS);

                if (publish)
                {
                    char timestamp[128];

                    last_pub = now;

                    make_timestamp(timestamp, sizeof(timestamp));

                    CLIENT_LOCK(&client_lock);

                    MDBG("Publishing: %s", timestamp);
                    mqtt_publish_message(
                        "testi",
                        (uint8_t *)timestamp,
                        strlen(timestamp)
                    );
                    
                    rc = mqtt_live(clientPtr);

                    CLIENT_UNLOCK(&client_lock);
                }

                if (rc < 0 && rc != -EAGAIN) {
                    MERR("mqtt_live error @ publish %d", rc);
                    state = MQTT_STATE_DISCONNECT;
                    mqtt_connected = 0;
                }
                break;
            }

            case MQTT_STATE_DISCONNECT:
                MINF("MQTT: @ %s state handler", get_state_name(state));

                mqtt_abort(clientPtr);

                mqtt_connected = 0;
                state = MQTT_STATE_RECONNECT;

                break;


            case MQTT_STATE_RECONNECT:
                MINF("MQTT: @ %s state handler", get_state_name(state));

                rc = stcp_mqtt_reconnect(clientPtr);

                if (rc != 0) {
                    state = MQTT_STATE_WAIT_RETRY;
                    break;
                }

                mqtt_connected = 1;
                state = MQTT_STATE_RUNNING;
                break;

            case MQTT_STATE_WAIT_RETRY:
                MINF("MQTT: @ %s state handler", get_state_name(state));

                MINF("MQTT: retry in 2 seconds");
                SLEEP_SEC(2);

                state = MQTT_STATE_RECONNECT;
                break;
        }
    }
}


