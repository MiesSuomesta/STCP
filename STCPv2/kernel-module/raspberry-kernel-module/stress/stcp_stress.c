#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifndef AF_STCP
#define AF_STCP 45
#endif

#define DEFAULT_PORT 7777
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_CLIENTS 8
#define DEFAULT_DURATION 30
#define DEFAULT_ITERATIONS 1000
#define DEFAULT_PAYLOAD 4096
#define DEFAULT_TCP_PROTO 253
#define DEFAULT_UDP_PROTO 254
#define MAX_PAYLOAD (1024 * 1024)

enum mode {
    MODE_SERVER,
    MODE_CHURN,
    MODE_STEADY,
};

struct config {
    enum mode mode;
    const char *host;
    int port;
    int protocol;
    int clients;
    int duration;
    int iterations;
    size_t payload_size;
    bool random_payload;
    bool verify;
    bool echo;
};

struct stats {
    atomic_ullong connections_ok;
    atomic_ullong connections_failed;
    atomic_ullong sends_ok;
    atomic_ullong sends_failed;
    atomic_ullong recvs_ok;
    atomic_ullong recvs_failed;
    atomic_ullong bytes_sent;
    atomic_ullong bytes_received;
    atomic_ullong verify_failed;
};

struct worker_arg {
    const struct config *cfg;
    struct stats *stats;
    int id;
};

static volatile sig_atomic_t stop_requested;

static void on_signal(int signo)
{
    (void)signo;
    stop_requested = 1;
}

static uint64_t now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000ULL +
           (uint64_t)ts.tv_nsec / 1000000ULL;
}

static void fill_payload(uint8_t *buffer, size_t length, uint64_t seed)
{
    uint64_t value = seed ^ 0x9e3779b97f4a7c15ULL;
    size_t i;

    for (i = 0; i < length; ++i) {
        value ^= value << 13;
        value ^= value >> 7;
        value ^= value << 17;
        buffer[i] = (uint8_t)(value & 0xff);
    }
}

static int make_addr(const struct config *cfg, struct sockaddr_in *addr)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons((uint16_t)cfg->port);

    if (inet_pton(AF_INET, cfg->host, &addr->sin_addr) != 1)
        return -1;

    return 0;
}

static int create_socket(const struct config *cfg)
{
    return socket(AF_STCP, SOCK_STREAM, cfg->protocol);
}

static int send_all(int fd, const uint8_t *buffer, size_t length)
{
    size_t offset = 0;

    while (offset < length) {
        ssize_t written = send(
            fd,
            buffer + offset,
            length - offset,
            MSG_NOSIGNAL
        );

        if (written < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (written == 0) {
            errno = EPIPE;
            return -1;
        }

        offset += (size_t)written;
    }

    return 0;
}

static int recv_all(int fd, uint8_t *buffer, size_t length)
{
    size_t offset = 0;

    while (offset < length) {
        ssize_t received = recv(
            fd,
            buffer + offset,
            length - offset,
            0
        );

        if (received < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (received == 0) {
            errno = ECONNRESET;
            return -1;
        }

        offset += (size_t)received;
    }

    return 0;
}

static int connect_once(
    const struct config *cfg,
    struct sockaddr_in *addr
)
{
    int fd = create_socket(cfg);

    if (fd < 0)
        return -1;

    if (connect(fd, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }

    return fd;
}

static void *churn_worker(void *opaque)
{
    struct worker_arg *arg = opaque;
    const struct config *cfg = arg->cfg;
    struct sockaddr_in addr;
    uint8_t *tx;
    uint8_t *rx;
    int iteration;

    if (make_addr(cfg, &addr) < 0)
        return NULL;

    tx = malloc(cfg->payload_size);
    rx = malloc(cfg->payload_size);

    if (!tx || !rx) {
        free(tx);
        free(rx);
        return NULL;
    }

    for (iteration = 0;
         iteration < cfg->iterations && !stop_requested;
         ++iteration) {
        uint64_t seed =
            ((uint64_t)(unsigned)arg->id << 32) |
            (uint32_t)iteration;
        int fd;

        fill_payload(tx, cfg->payload_size, seed);

        fd = connect_once(cfg, &addr);

        if (fd < 0) {
            atomic_fetch_add(&arg->stats->connections_failed, 1);
            continue;
        }

        atomic_fetch_add(&arg->stats->connections_ok, 1);

        if (send_all(fd, tx, cfg->payload_size) < 0) {
            atomic_fetch_add(&arg->stats->sends_failed, 1);
            close(fd);
            continue;
        }

        atomic_fetch_add(&arg->stats->sends_ok, 1);
        atomic_fetch_add(
            &arg->stats->bytes_sent,
            cfg->payload_size
        );

        if (cfg->echo) {
            if (recv_all(fd, rx, cfg->payload_size) < 0) {
                atomic_fetch_add(&arg->stats->recvs_failed, 1);
                close(fd);
                continue;
            }

            atomic_fetch_add(&arg->stats->recvs_ok, 1);
            atomic_fetch_add(
                &arg->stats->bytes_received,
                cfg->payload_size
            );

            if (cfg->verify &&
                memcmp(tx, rx, cfg->payload_size) != 0) {
                atomic_fetch_add(
                    &arg->stats->verify_failed,
                    1
                );
            }
        }

        shutdown(fd, SHUT_RDWR);
        close(fd);
    }

    free(tx);
    free(rx);
    return NULL;
}

static void *steady_worker(void *opaque)
{
    struct worker_arg *arg = opaque;
    const struct config *cfg = arg->cfg;
    struct sockaddr_in addr;
    uint8_t *tx;
    uint8_t *rx;
    uint64_t deadline;
    uint64_t sequence = 0;
    int fd = -1;

    if (make_addr(cfg, &addr) < 0)
        return NULL;

    tx = malloc(cfg->payload_size);
    rx = malloc(cfg->payload_size);

    if (!tx || !rx) {
        free(tx);
        free(rx);
        return NULL;
    }

    deadline = now_ms() + (uint64_t)cfg->duration * 1000ULL;

    while (!stop_requested && now_ms() < deadline) {
        uint64_t seed =
            ((uint64_t)(unsigned)arg->id << 32) |
            (uint32_t)sequence;

        if (fd < 0) {
            fd = connect_once(cfg, &addr);

            if (fd < 0) {
                atomic_fetch_add(
                    &arg->stats->connections_failed,
                    1
                );
                usleep(10000);
                continue;
            }

            atomic_fetch_add(
                &arg->stats->connections_ok,
                1
            );
        }

        fill_payload(tx, cfg->payload_size, seed);

        if (send_all(fd, tx, cfg->payload_size) < 0) {
            atomic_fetch_add(&arg->stats->sends_failed, 1);
            close(fd);
            fd = -1;
            continue;
        }

        atomic_fetch_add(&arg->stats->sends_ok, 1);
        atomic_fetch_add(
            &arg->stats->bytes_sent,
            cfg->payload_size
        );

        if (cfg->echo) {
            if (recv_all(fd, rx, cfg->payload_size) < 0) {
                atomic_fetch_add(
                    &arg->stats->recvs_failed,
                    1
                );
                close(fd);
                fd = -1;
                continue;
            }

            atomic_fetch_add(&arg->stats->recvs_ok, 1);
            atomic_fetch_add(
                &arg->stats->bytes_received,
                cfg->payload_size
            );

            if (cfg->verify &&
                memcmp(tx, rx, cfg->payload_size) != 0) {
                atomic_fetch_add(
                    &arg->stats->verify_failed,
                    1
                );
            }
        }

        ++sequence;
    }

    if (fd >= 0) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }

    free(tx);
    free(rx);
    return NULL;
}

struct server_client_arg {
    int fd;
    size_t payload_size;
};

static void *server_client_worker(void *opaque)
{
    struct server_client_arg *arg = opaque;
    uint8_t *buffer = malloc(arg->payload_size);

    if (!buffer) {
        close(arg->fd);
        free(arg);
        return NULL;
    }

    while (!stop_requested) {
        ssize_t received = recv(
            arg->fd,
            buffer,
            arg->payload_size,
            0
        );

        if (received < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        if (received == 0)
            break;

        if (send_all(arg->fd, buffer, (size_t)received) < 0)
            break;
    }

    shutdown(arg->fd, SHUT_RDWR);
    close(arg->fd);
    free(buffer);
    free(arg);
    return NULL;
}

static int run_server(const struct config *cfg)
{
    struct sockaddr_in addr;
    int listener;

    if (make_addr(cfg, &addr) < 0) {
        fprintf(stderr, "invalid address\n");
        return 1;
    }

    listener = create_socket(cfg);

    if (listener < 0) {
        perror("socket");
        return 1;
    }

    if (bind(
            listener,
            (struct sockaddr *)&addr,
            sizeof(addr)
        ) < 0) {
        perror("bind");
        close(listener);
        return 1;
    }

    if (listen(listener, 256) < 0) {
        perror("listen");
        close(listener);
        return 1;
    }

    printf(
        "stcp-stress server: host=%s port=%d protocol=%d\n",
        cfg->host,
        cfg->port,
        cfg->protocol
    );

    while (!stop_requested) {
        struct server_client_arg *client_arg;
        pthread_t thread;
        int client = accept(listener, NULL, NULL);

        if (client < 0) {
            if (errno == EINTR)
                continue;

            perror("accept");
            break;
        }

        client_arg = calloc(1, sizeof(*client_arg));

        if (!client_arg) {
            close(client);
            continue;
        }

        client_arg->fd = client;
        client_arg->payload_size = cfg->payload_size;

        if (pthread_create(
                &thread,
                NULL,
                server_client_worker,
                client_arg
            ) != 0) {
            close(client);
            free(client_arg);
            continue;
        }

        pthread_detach(thread);
    }

    close(listener);
    return 0;
}

static void print_stats(
    const struct config *cfg,
    const struct stats *stats,
    uint64_t elapsed_ms
)
{
    unsigned long long bytes_sent =
        atomic_load(&stats->bytes_sent);
    unsigned long long bytes_received =
        atomic_load(&stats->bytes_received);
    double seconds = elapsed_ms / 1000.0;
    double tx_mib_s = seconds > 0.0
        ? (double)bytes_sent / (1024.0 * 1024.0) / seconds
        : 0.0;
    double rx_mib_s = seconds > 0.0
        ? (double)bytes_received / (1024.0 * 1024.0) / seconds
        : 0.0;

    printf("\n=== STCP stress result ===\n");
    printf("protocol:            %d\n", cfg->protocol);
    printf("clients:             %d\n", cfg->clients);
    printf("payload:             %zu bytes\n", cfg->payload_size);
    printf("elapsed:             %.3f s\n", seconds);
    printf("connections ok:      %llu\n",
           atomic_load(&stats->connections_ok));
    printf("connections failed:  %llu\n",
           atomic_load(&stats->connections_failed));
    printf("sends ok:            %llu\n",
           atomic_load(&stats->sends_ok));
    printf("sends failed:        %llu\n",
           atomic_load(&stats->sends_failed));
    printf("recvs ok:            %llu\n",
           atomic_load(&stats->recvs_ok));
    printf("recvs failed:        %llu\n",
           atomic_load(&stats->recvs_failed));
    printf("verify failures:     %llu\n",
           atomic_load(&stats->verify_failed));
    printf("bytes sent:          %llu\n", bytes_sent);
    printf("bytes received:      %llu\n", bytes_received);
    printf("TX throughput:       %.2f MiB/s\n", tx_mib_s);
    printf("RX throughput:       %.2f MiB/s\n", rx_mib_s);
}

static int run_clients(const struct config *cfg)
{
    struct stats stats = {0};
    struct worker_arg *args;
    pthread_t *threads;
    uint64_t started;
    uint64_t elapsed;
    int i;
    int failed = 0;

    threads = calloc((size_t)cfg->clients, sizeof(*threads));
    args = calloc((size_t)cfg->clients, sizeof(*args));

    if (!threads || !args) {
        free(threads);
        free(args);
        return 1;
    }

    started = now_ms();

    for (i = 0; i < cfg->clients; ++i) {
        args[i].cfg = cfg;
        args[i].stats = &stats;
        args[i].id = i;

        if (pthread_create(
                &threads[i],
                NULL,
                cfg->mode == MODE_CHURN
                    ? churn_worker
                    : steady_worker,
                &args[i]
            ) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            stop_requested = 1;
            failed = 1;
            break;
        }
    }

    for (int j = 0; j < i; ++j)
        pthread_join(threads[j], NULL);

    elapsed = now_ms() - started;
    print_stats(cfg, &stats, elapsed);

    if (atomic_load(&stats.connections_failed) ||
        atomic_load(&stats.sends_failed) ||
        atomic_load(&stats.recvs_failed) ||
        atomic_load(&stats.verify_failed)) {
        failed = 1;
    }

    free(threads);
    free(args);
    return failed;
}

static void usage(const char *program)
{
    fprintf(stderr,
        "usage: %s MODE [options]\n"
        "\n"
        "MODE:\n"
        "  server       run threaded echo server\n"
        "  churn        repeated connect/send/recv/close\n"
        "  steady       persistent connections for duration\n"
        "\n"
        "options:\n"
        "  --host ADDR       default 127.0.0.1\n"
        "  --port PORT       default 7777\n"
        "  --protocol ID     STCP carrier protocol id\n"
        "  --clients N       default 8\n"
        "  --duration SEC    steady duration, default 30\n"
        "  --iterations N    churn iterations/client, default 1000\n"
        "  --payload BYTES   default 4096, max 1048576\n"
        "  --no-verify       disable echo verification\n"
        "  --no-echo         send-only mode\n",
        program
    );
}

int main(int argc, char **argv)
{
    struct config cfg = {
        .mode = MODE_SERVER,
        .host = DEFAULT_HOST,
        .port = DEFAULT_PORT,
        .protocol = DEFAULT_TCP_PROTO,
        .clients = DEFAULT_CLIENTS,
        .duration = DEFAULT_DURATION,
        .iterations = DEFAULT_ITERATIONS,
        .payload_size = DEFAULT_PAYLOAD,
        .random_payload = true,
        .verify = true,
        .echo = true,
    };
    static const struct option options[] = {
        {"host", required_argument, NULL, 'h'},
        {"port", required_argument, NULL, 'p'},
        {"protocol", required_argument, NULL, 'P'},
        {"clients", required_argument, NULL, 'c'},
        {"duration", required_argument, NULL, 'd'},
        {"iterations", required_argument, NULL, 'i'},
        {"payload", required_argument, NULL, 's'},
        {"no-verify", no_argument, NULL, 'V'},
        {"no-echo", no_argument, NULL, 'E'},
        {NULL, 0, NULL, 0},
    };
    int option;

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGPIPE, SIG_IGN);

    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }

    if (!strcmp(argv[1], "server"))
        cfg.mode = MODE_SERVER;
    else if (!strcmp(argv[1], "churn"))
        cfg.mode = MODE_CHURN;
    else if (!strcmp(argv[1], "steady"))
        cfg.mode = MODE_STEADY;
    else {
        usage(argv[0]);
        return 2;
    }

    optind = 2;

    while ((option = getopt_long(
                argc,
                argv,
                "h:p:P:c:d:i:s:VE",
                options,
                NULL
            )) != -1) {
        switch (option) {
        case 'h':
            cfg.host = optarg;
            break;
        case 'p':
            cfg.port = atoi(optarg);
            break;
        case 'P':
            cfg.protocol = atoi(optarg);
            break;
        case 'c':
            cfg.clients = atoi(optarg);
            break;
        case 'd':
            cfg.duration = atoi(optarg);
            break;
        case 'i':
            cfg.iterations = atoi(optarg);
            break;
        case 's':
            cfg.payload_size = (size_t)strtoull(
                optarg,
                NULL,
                10
            );
            break;
        case 'V':
            cfg.verify = false;
            break;
        case 'E':
            cfg.echo = false;
            cfg.verify = false;
            break;
        default:
            usage(argv[0]);
            return 2;
        }
    }

    if (cfg.clients < 1 ||
        cfg.duration < 1 ||
        cfg.iterations < 1 ||
        cfg.payload_size < 1 ||
        cfg.payload_size > MAX_PAYLOAD ||
        cfg.port < 1 ||
        cfg.port > 65535) {
        fprintf(stderr, "invalid arguments\n");
        return 2;
    }

    if (cfg.mode == MODE_SERVER)
        return run_server(&cfg);

    return run_clients(&cfg);
}
