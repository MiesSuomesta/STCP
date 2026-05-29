#include <zephyr/sys/atomic.h>
#include <stcp/stcp_api.h>

// Ilman debuggeja
#undef STCP_DEBUG

#include <stcp/debug.h>
#include <stcp/dns.h>

#include "stcp_mqtt.h"
#include "stcp_mqtt_fsm.h"
#include "mqtt_demo_utils.h"
#include <zephyr/kernel.h>
#include <zephyr/posix/posix_types.h>

#define MQTT_STACK_SIZE                 (7*1024)
#define MQTT_PUMP_STACK_SIZE            (3*1014)

#define MQTT_PUMP_VERBOSE               0

#define MQTT_PRIORITY                   5
#define MQTT_ZOMBIE_MAX_TIMEOUTS        30
// retry asetuksia
#define MQTT_RETRY_BASE_MS   50
#define MQTT_RETRY_MAX_MS    6000

static uint32_t retry_delay = MQTT_RETRY_BASE_MS;

extern int mqtt_connected;

void stcp_mqtt_task(void *p1, void *p2, void *p3);
void stcp_mqtt_pump_task(void *p1, void *p2, void *p3);

extern struct k_mutex client_lock;

K_THREAD_STACK_DEFINE(stcp_mqtt_stack, MQTT_STACK_SIZE);
struct k_thread stcp_mqtt_thread;

K_THREAD_STACK_DEFINE(stcp_mqtt_pump_stack, MQTT_PUMP_STACK_SIZE);
struct k_thread stcp_mqtt_pump_thread;

static void stcp_mqtt_mark_dead(struct stcp_api *api)
{
    LERR("MQTT: API Dead, Closing API %p (FD: %d)...",
        api, stcp_api_get_fd(api));
    stcp_api_close(api);
}

static const char *get_state_name(enum mqtt_state state) {
#if CONFIG_STCP_DEBUG
    //LDBG("Gettin string for state %d", state);
    switch (state)
    {
        case MQTT_STATE_INITIAL:    return "INITIAL";   
        case MQTT_STATE_IDLE:       return "IDLE";      
        case MQTT_STATE_CONNECT:    return "CONNECT";   
        case MQTT_STATE_SUBSCRIBE:  return "SUBSCRIBE"; 
        case MQTT_STATE_RUNNING:    return "RUNNING";   
        case MQTT_STATE_DISCONNECT: return "DISCONNECT";
        case MQTT_STATE_WAIT_RETRY: return "WAIT_RETRY";
        case MQTT_STATE_WAIT_API:   return "WAIT_API";
        default:                   
            stcp_dump_bt();
            return "UNKNOWN";   
    }
#else
    static char tmp[2];

    tmp[0] = "0";
    tmp[0] += (int)state;
    tmp[1] = 0;

    return tmp;
#endif
}

static void stcp_mqtt_fsm_set_state(struct mqtt_ctx *mqtt_ctx, enum mqtt_state newState) {
    
    if (!mqtt_ctx) return;
    MQTT_CTX_LOCK(mqtt_ctx);
        if (mqtt_ctx->state != newState) {
            LDBGBIG("MQTT: FSM State change from %s (%d) to %s (%d)", 
                get_state_name(mqtt_ctx->state), mqtt_ctx->state,
                get_state_name(newState), newState
            );
            stcp_dump_bt();
        }
        mqtt_ctx->last_state = mqtt_ctx->state;
        mqtt_ctx->state = newState;
        LDBG("MQTT: FSM State is now %s (%d)", get_state_name(newState), newState);
    MQTT_CTX_UNLOCK(mqtt_ctx);
}

static enum mqtt_state stcp_mqtt_fsm_get_state(struct mqtt_ctx *mqtt_ctx) {
    if (!mqtt_ctx) return -ENODEV;
    enum mqtt_state ret = MQTT_STATE_INITIAL;

    MQTT_CTX_LOCK(mqtt_ctx);
        ret = mqtt_ctx->state;
        LDBG("MQTT: FSM State is %s (%d)", get_state_name(ret), ret);
    MQTT_CTX_UNLOCK(mqtt_ctx);

    return ret;
}

static uint32_t mqtt_next_delay(void)
{
    uint32_t jitter = sys_rand32_get() % 1000; // 0–999 ms
    uint32_t delay = retry_delay + jitter;

    if (retry_delay < MQTT_RETRY_MAX_MS) {
        retry_delay *= 2;
        if (retry_delay > MQTT_RETRY_MAX_MS)
            retry_delay = MQTT_RETRY_MAX_MS;
    }

    return delay;
}

static void mqtt_reset_backoff(void)
{
    retry_delay = MQTT_RETRY_BASE_MS;
}

static void mqtt_destroy_api(struct mqtt_client *client, int dropRef)
{
    struct stcp_api *api = API_FROM_CLIENT(client);
    if (!api) return;

    LDBG("MQTT: Setting connection in progress for API %p...", api);
    atomic_set(&api->connect_in_progress, 0);

    LINF("Destroying API %p", api);
    
    if (stcp_api_is_alive(api)) {
        stcp_api_close(api);
    }

    LDBG("MQTT: Dropping API %p MQTT REF now....", api);
    stcp_api_release(api);   // drop MQTT ref
    LDBG("MQTT: Nulling client %p API....", client);
    API_SET_TO_CLIENT(client, NULL);
}

// Semaphore for connack

struct k_sem g_connack_seen;

int stcp_mqtt_wait_for_connak_event(struct mqtt_client *client, struct stcp_api *api, int timeout_ms) 
{
    if (!api) {
        return -ENODEV;
    }

    API_CONTEXT_LOCK(api);
 
       if (api->connack_seen) {
            LDBGBIG("MQTT: Already got CONNACK...");
            API_CONTEXT_UNLOCK(api);
            return 0;
        }
 
    API_CONTEXT_UNLOCK(api);
 
    int rc = k_sem_take(&api->connack_sem, K_MSEC(timeout_ms));

    API_CONTEXT_LOCK(api);
        if (rc < 0) {
            LWRNBIG("MQTT: CONNACK NOT received within timeout %d ms, with jitter", timeout_ms);
            api->connack_seen = 0;
        } else {
            LDBGBIG("MQTT: Connection ACK seen!");
            api->connack_seen = 1;
        }
    API_CONTEXT_UNLOCK(api);

    return rc;
}

int stcp_mqtt_get_connak_event_seen(struct stcp_api *api) {
    if (!api) return -EBADFD;
    return api->connack_seen;
}

void stcp_mqtt_set_connak_event_seen(struct stcp_api *api)
{
    LINFBIG("API %p CONNACK sema give..", api);
    API_CONTEXT_LOCK(api);
        k_sem_give(&api->connack_sem);
        api->connack_seen = 1;
    API_CONTEXT_UNLOCK(api);
}

void stcp_mqtt_reset_connak_event_seen(struct stcp_api *api)
{
    LINFBIG("API %p CONNACK sema reset..", api);
    if (api) {
        API_CONTEXT_LOCK(api);
            k_sem_reset(&api->connack_sem);
            api->connack_seen = 0;
        API_CONTEXT_UNLOCK(api);
    }
}

void stpc_mqtt_init_fsm_thread() {
#if defined(CONFIG_STCP_TESTING) && (CONFIG_STCP_TEST_MODE != 3)
#define STCP_COMPILE_MQTT 0
#else
#define STCP_COMPILE_MQTT 1
#endif

#if STCP_COMPILE_MQTT
    LINFBIG("Initialising & staring MQTT FSM thread...");

    k_sem_init(&g_connack_seen, 0, 1);


    struct mqtt_ctx *mqtt_ctx = k_malloc(sizeof(struct mqtt_ctx));
    memset(mqtt_ctx, 0, sizeof(struct mqtt_ctx));

    struct mqtt_client* client = k_malloc(sizeof(struct mqtt_client));
    LINFBIG("MQTT: Created client %p", client);
    if (!client) {
        LERRBIG("MQTT: client: Out of Mana!");
        return -ENOBUFS;
    }

    memset(client, 0, sizeof(struct mqtt_client));

    k_mutex_init(&mqtt_ctx->lock);

    struct sockaddr_storage* broker = k_malloc(sizeof(struct sockaddr_storage));
    LINFBIG("MQTT: Created broker %p", broker);
    if (!broker) {
        LERRBIG("MQTT: broker: Out of Mana!");
        k_free(client);
        return -ENOBUFS;
    }

    memset(broker, 0, sizeof(struct sockaddr_storage));

    mqtt_ctx->client_ptr = client;
    mqtt_ctx->broker_ptr = broker;

    LINFBIG("MQTT: Thread starting, client memset ok");

    k_thread_create(
        &stcp_mqtt_thread,            // thread object
        stcp_mqtt_stack,              // stack
        K_THREAD_STACK_SIZEOF(stcp_mqtt_stack),
        stcp_mqtt_task,               // entry function
        mqtt_ctx,                     // p1
        NULL,                         // p2
        NULL,                         // p3
        MQTT_PRIORITY,                // priority
        0,                            // options
        K_NO_WAIT                     // start delay
    );
    LINFBIG("MQTT: Thread starting, client memset ok");

    k_thread_create(
        &stcp_mqtt_pump_thread,       // thread object
        stcp_mqtt_pump_stack,         // stack
        K_THREAD_STACK_SIZEOF(stcp_mqtt_pump_stack),
        stcp_mqtt_pump_task,          // entry function
        mqtt_ctx,                     // p1
        NULL,                         // p2
        NULL,                         // p3
        MQTT_PRIORITY,                // priority
        0,                            // options
        K_NO_WAIT                     // start delay
    );

#else
    LINFBIG("Testing enabled but not MQTT testing => MQTT FSM not started");
#endif
}



#define ENABLE_API_AQUIRE  1

void stcp_mqtt_ctx_dump(const char* prefix,  struct mqtt_ctx *mqtt_ctx, int lock, int hexdumps) {

    if (lock) {
        MQTT_CTX_LOCK(mqtt_ctx);
    }
        LDBG("MQTT: [%s] Context %p dump", prefix, mqtt_ctx);

        LDBG("MQTT: [%s] Client ptr     : %p", prefix, mqtt_ctx->client_ptr);
        MQTT_API_CLIENT_DEBUG(mqtt_ctx->client_ptr);

        LDBG("MQTT: [%s] Broker ptr     : %p", prefix, mqtt_ctx->broker_ptr);

        LDBG("MQTT: [%s] Current state  : %s (%d)", prefix,
            get_state_name(mqtt_ctx->state),
            mqtt_ctx->state);
        
        LDBG("MQTT: [%s] Last state     : %s (%d)", prefix,
            get_state_name(mqtt_ctx->last_state),
            mqtt_ctx->last_state);

        LDBG("MQTT: [%s] Last progress  : %d", prefix,
            mqtt_ctx->last_progress);
        
        LDBG("MQTT: [%s] Dead count     : %d", prefix,
            mqtt_ctx->dead_counter);
        
        LDBG("MQTT: [%s] EAGAIN count   : %d", prefix,
            mqtt_ctx->eagain_counter);
        
        LDBG("MQTT: [%s] Loop count     : %d", prefix,
            mqtt_ctx->loop_count);

        void *pRX = mqtt_ctx->rx_buffer;
        void *pTX = mqtt_ctx->tx_buffer;

    if (lock) {
        MQTT_CTX_UNLOCK(mqtt_ctx);
    }

    if (hexdumps) {
        LDBG("MQTT: [%s] RX buffer      : %p", prefix, pRX);
        stcp_hexdump_ascii("RX buffer", pRX, RX_BUF_SIZE);

        LDBG("MQTT: [%s] TX buffer      : %p", prefix, pTX);
        stcp_hexdump_ascii("RX buffer", pTX, TX_BUF_SIZE);
    }
}

int stcp_mqtt_do_single_input_event(struct mqtt_client *client, int run_input, int run_live, int *input_ret, int *live_ret)
{
    if (client) {
        int rc = -1;
        
        struct stcp_api *api = API_FROM_CLIENT(client);
#if MQTT_PUMP_VERBOSE
        LDBG("MQTT: Singled event API %p",
            api
        );
#endif

        if (input_ret) { *input_ret = 0; }
        if (live_ret) { *live_ret = 0; }

        if (run_input) {
            rc = mqtt_input(client);
            if (input_ret) { *input_ret = rc; }
#if MQTT_PUMP_VERBOSE
            LDBG("MQTT: Single mqtt_input rc=%d", rc);
#endif
        }

        if (run_live) {
            rc = mqtt_live(client);
            if (live_ret) { *live_ret = rc; }
#if MQTT_PUMP_VERBOSE
            LDBG("MQTT: Single live_input rc=%d", rc);
#endif
        }

        return 0;
    }
    return -ENODEV;
}


#define MQTT_POLLING_SLEEP_MS   50

void stcp_mqtt_pump_task(void *p1, void *p2, void *p3)
{
    struct mqtt_ctx *mqtt_ctx = p1;
    LDBG("MQTT: Pump executing.....");
    LDBG("MQTT: Context: %p", mqtt_ctx);

    while ( 1 ) {

        int input_ret = -1;
        int live_ret = -1;
        
        int run_input = 1;
        int run_live = 1;
        int pump_ret = 0;
        int run_pump = 0;
        int api_ok = 0;
        int hs_ok = 0;
        struct mqtt_client *clientPtr = NULL;
        struct stcp_api *api = NULL;
        int api_aquired = 0;

        MQTT_CTX_LOCK(mqtt_ctx);

            stcp_mqtt_ctx_dump("PUMP: at begin of while loop", mqtt_ctx, 0, 0);

            clientPtr = mqtt_ctx->client_ptr;

#if MQTT_PUMP_VERBOSE
            LDBG("MQTT: Pump got client: %p & state %d", clientPtr, mqtt_ctx->state);
            MQTT_API_CLIENT_DEBUG(clientPtr);
#endif

            if (clientPtr != NULL) {
                api = API_FROM_CLIENT(clientPtr);
                api_aquired = stcp_api_acquire(api) == 0;
                
                if (api_aquired) {
                    hs_ok = stcp_api_get_handshake_status(api);
                }

#if MQTT_PUMP_VERBOSE
                LDBG("MQTT: Pump got API %p from client %p .. HS status: %d", 
                    api, clientPtr, hs_ok);
#endif

                if (hs_ok > 0) {

                        // Millon pumpataan?
                        run_pump |= mqtt_ctx->state == MQTT_STATE_CONNACK;
                        run_pump |= mqtt_ctx->state == MQTT_STATE_CONNECT;
                        run_pump |= mqtt_ctx->state == MQTT_STATE_RUNNING;
                        run_pump |= mqtt_ctx->state == MQTT_STATE_SUBSCRIBE;

                    // Connect statessa ei liveä
                    if (mqtt_ctx->state == MQTT_STATE_CONNECT) {
                        run_live = 0;
                    }

                    api_ok = api_aquired && stcp_api_is_usable(api);

                } 
            }


        MQTT_CTX_UNLOCK(mqtt_ctx);
        
        if (run_pump && api_ok) {

#if MQTT_PUMP_VERBOSE
            LDBG("[MQTT PUMP] Running input: %s live: %s... now at state %s (%d)", 
                GET_YES_NO_STR(run_input),
                GET_YES_NO_STR(run_live),
                get_state_name(mqtt_ctx->state),
                mqtt_ctx->state);
#endif

            pump_ret = stcp_mqtt_do_single_input_event(
                clientPtr, run_input, run_live, &input_ret, &live_ret);

#if MQTT_PUMP_VERBOSE
            LDBGBIG(
                "MQTT PUMP client=%p input=%d live=%d ret=%d errno=%d",
                    clientPtr,
                    input_ret,
                    live_ret,
                    pump_ret,
                    errno
                );
#endif
        }

        if (api_aquired) {
            stcp_api_release(api);
        }

        sleep_ms_jitter(MQTT_POLLING_SLEEP_MS, 50);
    }
}

void stcp_mqtt_task(void *p1, void *p2, void *p3)
{
    int rc;

    struct mqtt_ctx *mqtt_ctx = p1;

    struct mqtt_client *clientPtr = mqtt_ctx->client_ptr;

    //static uint32_t loop_cnt = 0;
//    static uint32_t last_progress = 0;
//    static int dead_counter = 0;
//    static enum mqtt_state last_state = MQTT_STATE_INITIAL;
    // API initti, API on oltava elossa ENNEN whileä
    // connecti ottaa reffin

    LDBG("MQTT: Thread got client: %p", clientPtr);


#if ENABLE_API_AQUIRE
    int alive = 0;
    int api_aquired = 0;
#endif

    while (1) {

        struct stcp_api *api = NULL;
        struct stcp_fsm *fsm = NULL;
        int aquired_ret = -EINVAL;

        MQTT_CTX_LOCK(mqtt_ctx);
            api = API_FROM_CLIENT(clientPtr);
            LDBG("MQTT: Got api at top of while %p", api);
            if (api) {
                aquired_ret = stcp_api_acquire(api);
            }
        MQTT_CTX_UNLOCK(mqtt_ctx);
        
        stcp_mqtt_ctx_dump("FSM: at begin of while loop", mqtt_ctx, 1, 0);

        LDBG("MQTT: Aquired API %p, ret %d", api, aquired_ret);

        int connect_in_progress = stcp_api_get_connect_in_progress(api);
        int handshake_done = stcp_api_get_handshake_status(api);

        LDBG("MQTT: Connect in progress: %s (%d)", 
            GET_YES_NO_STR(connect_in_progress == 1),
            connect_in_progress
        );
        
        LDBG("MQTT: Connect hanshake done: %s (%d)", 
            GET_YES_NO_STR(handshake_done == 1),
            handshake_done
        );
        
#if ENABLE_API_AQUIRE
        if (api && aquired_ret == 0) {
            LINFBIG("MQTT: API aquired!");
            api_aquired = 1;
        } else {
            api = NULL;
            api_aquired = 0;
            LWRNBIG("MQTT: API Aquire failed @ %s state", get_state_name(mqtt_ctx->state));
            SLEEP_MSEC(100);
        }
#endif

        LDBG("MQTT: Got MQTT CTX %p with at state %d", mqtt_ctx, mqtt_ctx->state);
        
        mqtt_ctx->loop_count++;

        if (api) {
            alive = stcp_api_is_alive(api);
            LDBG("MQTT: API %p check, alive: %s, state: %d, dead count: %d",
                    api, 
                    GET_YES_NO_STR(alive == 1), 
                    mqtt_ctx->state, 
                    mqtt_ctx->dead_counter
                );
        
            int skip_and_reset_dead_counter = 0;

            skip_and_reset_dead_counter |= mqtt_ctx->state == MQTT_STATE_CONNECT;
            skip_and_reset_dead_counter |= mqtt_ctx->state == MQTT_STATE_RUNNING;
            skip_and_reset_dead_counter |= mqtt_ctx->state == MQTT_STATE_CONNACK;
            skip_and_reset_dead_counter |= mqtt_ctx->state == MQTT_STATE_WAIT_API;
            skip_and_reset_dead_counter |= mqtt_ctx->state == MQTT_STATE_WAIT_RETRY;

            LDBG("Skip dead API check? %s",
                    GET_YES_NO_STR(skip_and_reset_dead_counter)
                );

            if (!skip_and_reset_dead_counter) {
                if (!alive) {
                    mqtt_ctx->dead_counter++;
                    LWRN("API not alive count=%d", mqtt_ctx->dead_counter);

                    if (mqtt_ctx->dead_counter > 10) {
                        LERR("API dead too long -> disconnect");
                        stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_DISCONNECT);
                    }
                } else {
                    LWRN("API %p dead counter reset (not alive)", api);
                    mqtt_ctx->dead_counter = 0;
                } 
            } else {
                LWRN("API %p dead counter reset (by state)", api);
                mqtt_ctx->dead_counter = 0;
            }
        }

        LDBG("MQTT: Client API %p, alive: %s, state: %d", 
                api, GET_YES_NO_STR(alive == 1),
                mqtt_ctx->state
        );

        if (mqtt_ctx->last_state != mqtt_ctx->state) {
            mqtt_ctx->last_progress = k_uptime_get_32();
        }

        int skip_retry = 0;

        // Statet joita ei katsella
        skip_retry |= mqtt_ctx->state == MQTT_STATE_WAIT_RETRY;
        skip_retry |= mqtt_ctx->state == MQTT_STATE_RUNNING;
        skip_retry |= mqtt_ctx->state == MQTT_STATE_CONNECT;
        skip_retry |= mqtt_ctx->state == MQTT_STATE_INITIAL;
        skip_retry |= mqtt_ctx->state == MQTT_STATE_IDLE;
        skip_retry |= mqtt_ctx->state == MQTT_STATE_SUBSCRIBE;

        LDBG("Running stuck skipped: %d", skip_retry);

        if (!skip_retry) {
            if ((k_uptime_get_32() - mqtt_ctx->last_progress) > 60000) {
                LERR("MQTT FSM stuck → forcing reset");
                mqtt_ctx->state =  MQTT_STATE_IDLE;
            }
        }

        if (mqtt_ctx->loop_count % 5 == 0)
        {
#if CONFIG_STCP_DEBUG
            const char *msg = get_state_name(mqtt_ctx->state);
            MINF("MQTT alive, uptime %u sec, current state %s (%d)", 
                mqtt_get_timestamp(),
                msg,
                mqtt_ctx->state
            );
#else
            LINF("MQTT alive, uptime %u sec, current state %d", 
                mqtt_get_timestamp(),
                state
            );
#endif
        }

#if CONFIG_STCP_DEBUG
        if (stcp_mqtt_fsm_get_state(mqtt_ctx) != mqtt_ctx->last_state) {
            LINF("MQTT FSM State change: %s => %s", 
                get_state_name(mqtt_ctx->last_state), 
                get_state_name(mqtt_ctx->state));

            mqtt_ctx->last_state = mqtt_ctx->state;
        } else {
            MWRN("MQTT FSM State still: %s", get_state_name(mqtt_ctx->state));
        }
#endif

        MDBG("MQTT: Got API: %p at state machine", api);
        if (!api) {
            int api_null_allowed = 0;

            api_null_allowed |= mqtt_ctx->state == MQTT_STATE_INITIAL;
            api_null_allowed |= mqtt_ctx->state == MQTT_STATE_CONNECT;
            api_null_allowed |= mqtt_ctx->state == MQTT_STATE_CONNACK;
            api_null_allowed |= mqtt_ctx->state == MQTT_STATE_IDLE;
            api_null_allowed |= mqtt_ctx->state == MQTT_STATE_WAIT_API;
            api_null_allowed |= mqtt_ctx->state == MQTT_STATE_WAIT_RETRY;

            if ( api_null_allowed ) {
                MWRN("MQTT: Null allowed for %s state", get_state_name(mqtt_ctx->state));
            } else {
                MWRN("MQTT: API IS NULL and not allowed for %s state", get_state_name(mqtt_ctx->state));
                SLEEP_MSEC(250);
                continue;
            }
        }

        //API_CONTEXT_LOCK(api);
jump_to_state_switch:

        LDBG("MQTT: At FSM switch: got API %p at state %s (%d)", 
            api, 
            get_state_name(mqtt_ctx->state),
            mqtt_ctx->state);

        stcp_mqtt_ctx_dump(get_state_name(mqtt_ctx->state), mqtt_ctx, 1, 1);

        switch (mqtt_ctx->state) {

            case MQTT_STATE_INITIAL: 
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));
                stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_IDLE);
                break;

            case MQTT_STATE_IDLE:
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));
                stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_CONNECT);
                break;

            case MQTT_STATE_CONNACK:
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));
                int hs_done_rc = -EAGAIN;
                
                MDBG("Starting to wait handshake to complete....");
                hs_done_rc = stcp_api_wait_until_stcp_handshake_is_done(api, 60*1000);
                MDBG("Wait handshake to complete....Done: %d", hs_done_rc);

                LINF("MQTT: Waiting CONNACK event for %d ms",
                    STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC);

                MQTT_API_CLIENT_DEBUG(clientPtr);
                rc = stcp_mqtt_wait_for_connak_event(
                        clientPtr,
                        api,
                        STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC
                    );

                LDBGBIG("MQTT: CONNACK wait ret: %d HS: %d", rc, hs_done_rc);

                if (rc == 0) {
                    LDBG("Got CONNACK .. HS status: %d", hs_done_rc);
                    if (hs_done_rc == 0) {
                        LDBGBIG("Got CONNACK, subscribing!");
                        stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_SUBSCRIBE);
                        LDBG("MQTT: Setting connection in progress false for API %p...", api);
                        atomic_set(&api->connect_in_progress, 0);
                    }
                } else {
                    // error while waiting for connect => reinit
                    LDBG("Wait for CONNACK timeout..");
                    if (
                        rc == -EAGAIN ||
                        rc == -EINPROGRESS ||
                        (rc == -1 && errno == EAGAIN)
                    ) {
                        LINF("MQTT: connect in progress...");
                        SLEEP_MSEC(100);
                        break;
                    }

                    LERR("MQTT: CONNACK wait failed rc=%d errno=%d", rc, errno);

                    stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_WAIT_RETRY);
                break;
                
            case MQTT_STATE_CONNECT:

#define MQTT_STCP_WAIT_FOR_RADIO_CONNECTION_SECONDS   (60)
#define MQTT_STCP_WAIT_FOR_RADIO_CONNECTION_TRIES     (6)

                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));
 
                int tries = MQTT_STCP_WAIT_FOR_RADIO_CONNECTION_TRIES;
                stcp_api_try_to_wakeup_radio();
                int timeout = 0;
                do {
                    timeout = stcp_api_wait_for_radio_connected(
                            MQTT_STCP_WAIT_FOR_RADIO_CONNECTION_SECONDS);
                    
                    if (timeout == 0) {
                        LDBG("Radio is now connected!");
                    } 

                    if (timeout == -EAGAIN) {
                        LDBG("Timeout..tries left: %d", tries);
                        stcp_api_try_to_wakeup_radio();
                        SLEEP_MSEC(250);
                    } 

                } while( (timeout < 0) && (tries-- > 0));
                LDBGBIG("Radio loop passed!");                

                int radio = 0;
                int connection_ok = 0;

                stcp_api_get_modem_state(NULL, NULL, NULL, &radio, &connection_ok);
                LDBG("Got modem state ok? Radio: %s, Connection: %s", 
                        GET_YES_NO_STR(radio), 
                        GET_YES_NO_STR(connection_ok)
                    );
                
                if (!connection_ok) {
                    LWRN("Connection not ok => retry...");
                    sleep_ms_jitter(500, 500);
                    break;
                } 

                LDBGBIG("MQTT: Starting to connect...........");

                // PITÄÄ OLLA NULL!
                struct stcp_api *newApi = NULL;

                rc = mqtt_connect_via_stcp(
                    CONFIG_STCP_CONNECT_TO_HOST,
                    CONFIG_STCP_CONNECT_TO_PORT,
                    mqtt_ctx,
                    &newApi);

                LDBGBIG("MQTT: Got connected API: %p", newApi);
                MQTT_API_CLIENT_DEBUG(mqtt_ctx->client_ptr);

                if (!stcp_api_is_alive(newApi)) {
                    LERR("MQTT: Got no alive API from mqtt connect => IDLE");
                    stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_IDLE);
                    break;
                }
                // NEW api ok
                LDBG("Starting update client %p.....", clientPtr);
                API_CONTEXT_LOCK(newApi);

                    stcp_mqtt_reset_connak_event_seen(newApi);
                    MQTT_CTX_LOCK(mqtt_ctx);

                        MQTT_API_CLIENT_DEBUG(clientPtr);
                        API_SET_TO_CLIENT(clientPtr, newApi);
                        mqtt_ctx->client_ptr = clientPtr;
                        api = API_FROM_CLIENT(clientPtr);
                        MQTT_API_CLIENT_DEBUG(mqtt_ctx->client_ptr);

                    MQTT_CTX_UNLOCK(mqtt_ctx);

                API_CONTEXT_UNLOCK(newApi);

                // Start to wait CONNACK
                LDBGBIG("Starting CONNACK state!");
                stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_CONNACK);
                break;
            }

            case MQTT_STATE_SUBSCRIBE:
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));

                LINF("Starting subscription .....");
                MQTT_API_CLIENT_DEBUG(clientPtr);
                rc = stpc_mqtt_subscribe(clientPtr);
                if (rc == 0) {

                    LINFBIG("Subscription OK");

                    stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_RUNNING);

                } else if (
                    rc == -EAGAIN ||
                    (rc == -1 && errno == EAGAIN)
                ) {

                    LDBGBIG(
                        "Subscription pending (EAGAIN)"
                    );

                    /* jää subscribe-stateen */
                    //stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_SUBSCRIBE);

                } else {

                    LERR(
                        "Subscription FAILED rc=%d errno=%d",
                        rc,
                        errno
                    );
                    stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_IDLE);
                }                
                break;


            case MQTT_STATE_RUNNING: {
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));
                static uint32_t last_pub = 0;
                uint32_t now = mqtt_get_timestamp();
                rc = 0;
                
                // Wait until API connects
                if ( !stcp_api_is_usable(api) ) {
                    LERRBIG("MQTT: API %p not usable anymore @ %s state...", 
                        api, get_state_name(mqtt_ctx->state));

                    stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_IDLE);
                    SLEEP_MSEC(10);
                    break;
                }

                LDBGBIG("ENTER TO PUBLISH...");
                uint32_t diffPub = now - last_pub;
                int publish  = 1; // (last_pub == 0);
                    publish |= (diffPub > STCP_MQTT_PUBLISH_INTERVAL_MS);
                LDBG("MQTT: Publish: %d", publish);
                if (publish)
                {
#define TIMESTAMP_SIZE 128
                    char timestamp[TIMESTAMP_SIZE];

                    make_timestamp(timestamp, TIMESTAMP_SIZE);

                    LDBG("Publishing: %s", timestamp);
                    MQTT_API_CLIENT_DEBUG(clientPtr);
                    rc = mqtt_publish_message(
                        mqtt_ctx,
                        "testi",
                        (uint8_t *)timestamp,
                        strlen(timestamp)
                    );

                    if (rc == 0) {
                        last_pub = now;
                    }                  
                }
                break;
            }

            case MQTT_STATE_DISCONNECT:
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));

                MQTT_CTX_LOCK(mqtt_ctx);
                    if (clientPtr) {

                        MQTT_API_CLIENT_DEBUG(clientPtr);

                        struct stcp_api *old_api = API_FROM_CLIENT(clientPtr);

                        LINF("MQTT/Disconnect: Aborting ....");
                        MQTT_API_CLIENT_DEBUG(clientPtr);
                        mqtt_abort(clientPtr);

                        if (stcp_api_is_alive(old_api)) {
                            LINF("MQTT/Disconnect: Disconnecting ....");
                            MQTT_API_CLIENT_DEBUG(clientPtr);
                            int dc_ret = mqtt_disconnect(clientPtr, NULL);
                            LINF("MQTT/Disconnect: Disconnect rc=%d", dc_ret);
                            LDBG("MQTT: Setting connection in progress false for API %p...", api);
                            atomic_set(&api->connect_in_progress, 0);
                            stcp_api_close(old_api);
                        }
                    }
                    MQTT_CTX_UNLOCK(mqtt_ctx);

                LINF("MQTT/Disconnect: API handled.... retrying...");
                stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_WAIT_RETRY);
                break;

            case MQTT_STATE_WAIT_RETRY: {
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));
                    int api_usable = api && stcp_api_is_usable(api);

                    if ( !api_usable ) {
                        LDBGBIG(
                            "MQTT: Started WAIT_RETRY with unusable API %p",
                            api
                        );
                    } else {
                        LDBG("MQTT: Got api %p, closing it....", api);
                        if (api) {
                            stcp_api_close(api);
                            api = NULL;
                        }
                    }

                    uint32_t delay = mqtt_next_delay();
                    delay += 50;
                    LWRN("MQTT retry in %u ms", delay);
                
                    SLEEP_MSEC(delay);

                    LINF("MQTT/RETRY: Creating STCP API...");
                    struct stcp_api *newApi = NULL;
                    
                    rc = mqtt_connect_via_stcp(
                        CONFIG_STCP_CONNECT_TO_HOST,
                        CONFIG_STCP_CONNECT_TO_PORT,
                        mqtt_ctx,
                        &newApi);

                    MQTT_API_CLIENT_DEBUG(mqtt_ctx->client_ptr);
                    LDBG("MQTT/RETRY: Connect done, rc: %d", rc);

                    if (!newApi) {
                        LERR("MQTT/RETRY: Failed to recreate API => State IDLE");
                        stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_IDLE);
                        break;
                    }

                    api = newApi;
                    API_SET_TO_CLIENT(clientPtr, newApi);
                    MQTT_API_CLIENT_DEBUG(clientPtr);

                    LINF("MQTT/RETRY: Resetting CONNACK seen ....");
                    stcp_mqtt_reset_connak_event_seen(api);

                    api_usable = api && stcp_api_is_usable(api);

                    if (!api_usable) {

                        LDBGBIG(
                            "MQTT: Not usable API after STCP Connect"
                        );
                        stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_IDLE);
                    } else {

                        LDBGBIG(
                            "MQTT: API %p usable => Setting Connected state",
                            api
                        );
                        stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_CONNECT);
                    }
                    
                    break;
                }

            case MQTT_STATE_WAIT_API: {
                LINF("MQTT: @ %s state handler", get_state_name(mqtt_ctx->state));
                MQTT_CTX_LOCK(mqtt_ctx);

                    api = API_FROM_CLIENT(clientPtr);
                    MQTT_API_CLIENT_DEBUG(clientPtr);

                MQTT_CTX_UNLOCK(mqtt_ctx);

                if (!api) {
                    stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_CONNECT);
                    LERR("MQTT/Wait API: Got NO API => Connecting");
                } else {
                    stcp_mqtt_fsm_set_state(mqtt_ctx, MQTT_STATE_WAIT_RETRY);
                    LERR("MQTT/Wait API: Got API => Retrying");
                }
            }

        }

        LDBG("MQTT: After switch: FSM API: %p State: %s", 
            api,
            get_state_name(mqtt_ctx->state)
        );
        MQTT_API_CLIENT_DEBUG(mqtt_ctx->client_ptr);

#if ENABLE_API_AQUIRE
        if (api) {
            if (api_aquired) {
                LDBG("MQTT: Aquired API Release .....");
                stcp_api_release(api);
            }
        }
#endif
        //API_CONTEXT_UNLOCK(api);
    }

    LDBG("MQTT: Freeing MQTT Context...");

    k_free(mqtt_ctx->client_ptr);
    k_free(mqtt_ctx->broker_ptr);
    k_free(mqtt_ctx);

    mqtt_dns_info_free();
}


