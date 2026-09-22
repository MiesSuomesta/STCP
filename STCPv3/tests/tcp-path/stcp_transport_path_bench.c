#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#ifndef AF_STCP
#define AF_STCP 45
#endif
#define STCP_TCP_PROTO 253
#define STCP_UDP_PROTO 254
#define MAGIC 0x53544350u
#define MODE_TX 1u
#define MODE_RX 2u
#define MODE_QUIT 3u
#define UDP_MAX_CHUNK 60000u

struct ctrl { uint32_t magic, mode, chunk, count; };
struct ack { uint32_t magic, mode; uint64_t bytes; uint64_t ns; uint64_t calls; };

enum transport_kind {
    TRANSPORT_STCP_TCP,
    TRANSPORT_TCP,
    TRANSPORT_TLS,
    TRANSPORT_UDP,
    TRANSPORT_STCP_UDP
};

static uint64_t ns_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static void die(const char *s) { perror(s); exit(1); }

static void ssl_die(const char *s) {
    fprintf(stderr, "%s\n", s);
    ERR_print_errors_fp(stderr);
    exit(1);
}

static const char *transport_name(enum transport_kind t) {
    switch (t) {
        case TRANSPORT_TCP: return "tcp";
        case TRANSPORT_TLS: return "tls";
        case TRANSPORT_UDP: return "udp";
        case TRANSPORT_STCP_UDP: return "stcp-udp";
        default: return "stcp-tcp";
    }
}

static int is_datagram(enum transport_kind t) {
    return t == TRANSPORT_UDP;
}

static enum transport_kind parse_transport(const char *s) {
    if (!strcmp(s, "stcp") || !strcmp(s, "stcp-tcp")) return TRANSPORT_STCP_TCP;
    if (!strcmp(s, "tcp")) return TRANSPORT_TCP;
    if (!strcmp(s, "tls") || !strcmp(s, "tls-tcp")) return TRANSPORT_TLS;
    if (!strcmp(s, "udp")) return TRANSPORT_UDP;
    if (!strcmp(s, "stcp-udp")) return TRANSPORT_STCP_UDP;
    fprintf(stderr, "bad transport: %s (expected tcp, stcp-tcp, tls, udp, stcp-udp)\n", s);
    exit(2);
}

static int transport_socket(enum transport_kind t) {
    int fd;
    if (t == TRANSPORT_TCP || t == TRANSPORT_TLS)
        fd = socket(AF_INET, SOCK_STREAM, 0);
    else if (t == TRANSPORT_UDP)
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    else if (t == TRANSPORT_STCP_UDP)
        fd = socket(AF_STCP, SOCK_STREAM, STCP_UDP_PROTO);
    else
        fd = socket(AF_STCP, SOCK_STREAM, STCP_TCP_PROTO);
    if (fd < 0) die("socket");
    return fd;
}

static void fill_addr(struct sockaddr_in *a, const char *ip, int port) {
    memset(a, 0, sizeof(*a));
    a->sin_family = AF_INET;
    a->sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &a->sin_addr) != 1) {
        fprintf(stderr, "bad ip: %s\n", ip);
        exit(2);
    }
}

static void stats(const char *tag, uint64_t bytes, uint64_t calls, uint64_t ns) {
    double sec = (double)ns / 1e9, mib = (double)bytes / (1024.0 * 1024.0);
    printf("%-16s bytes=%"PRIu64" calls=%"PRIu64" bytes/call=%.1f time=%.6f s throughput=%.3f MiB/s calls/s=%.1f\n",
           tag, bytes, calls, calls ? (double)bytes/calls : 0.0,
           sec, sec ? mib/sec : 0.0, sec ? (double)calls/sec : 0.0);
    fflush(stdout);
}

/* Stream I/O wrappers. TLS uses SSL_read/SSL_write; TCP/STCP-TCP use recv/send. */
static ssize_t stream_send(int fd, SSL *ssl, const void *buf, size_t len) {
    if (ssl) {
        int n = SSL_write(ssl, buf, (int)len);
        if (n <= 0) ssl_die("SSL_write failed");
        return n;
    }
    ssize_t n;
    do n = send(fd, buf, len, 0); while (n < 0 && errno == EINTR);
    return n;
}

static ssize_t stream_recv(int fd, SSL *ssl, void *buf, size_t len) {
    if (ssl) {
        int n = SSL_read(ssl, buf, (int)len);
        if (n <= 0) return n;
        return n;
    }
    ssize_t n;
    do n = recv(fd, buf, len, 0); while (n < 0 && errno == EINTR);
    return n;
}

static void send_exact(int fd, SSL *ssl, const void *buf, size_t len) {
    const char *p = buf;
    size_t done = 0;
    while (done < len) {
        ssize_t n = stream_send(fd, ssl, p + done, len - done);
        if (n < 0) die("send");
        if (n == 0) { fprintf(stderr, "send returned 0\n"); exit(2); }
        done += (size_t)n;
    }
}

static int recv_exact(int fd, SSL *ssl, void *buf, size_t len) {
    char *p = buf;
    size_t done = 0;
    while (done < len) {
        ssize_t n = stream_recv(fd, ssl, p + done, len - done);
        if (n < 0) die("recv");
        if (n == 0) return done == 0 ? 0 : -1;
        done += (size_t)n;
    }
    return 1;
}

static void stream_server_conn(int fd, SSL *ssl) {
    for (;;) {
        struct ctrl c;
        if (recv_exact(fd, ssl, &c, sizeof(c)) <= 0) return;
        if (c.magic != MAGIC) { fprintf(stderr, "bad magic\n"); return; }
        if (c.mode == MODE_QUIT) return;

        size_t chunk = c.chunk;
        uint32_t count = c.count;
        char *buf = malloc(chunk);
        if (!buf) die("malloc");
        memset(buf, 0xA5, chunk);
        uint64_t bytes = 0, calls = 0, t0 = 0, t1 = 0;

        if (c.mode == MODE_TX) {
            uint64_t target = (uint64_t)chunk * count;
            while (bytes < target) {
                size_t want = chunk;
                uint64_t left = target - bytes;
                if (left < want) want = (size_t)left;
                ssize_t r = stream_recv(fd, ssl, buf, want);
                if (r <= 0) { free(buf); return; }
                if (calls == 0) t0 = ns_now();
                bytes += (uint64_t)r;
                calls++;
            }
            t1 = ns_now();
            struct ack a = {MAGIC, MODE_TX, bytes, t1-t0, calls};
            send_exact(fd, ssl, &a, sizeof(a));
            stats("SERVER-RX", bytes, calls, t1-t0);
        } else if (c.mode == MODE_RX) {
            t0 = ns_now();
            for (uint32_t i=0; i<count; i++) {
                send_exact(fd, ssl, buf, chunk);
                bytes += chunk;
                calls++;
            }
            t1 = ns_now();
            struct ack a;
            recv_exact(fd, ssl, &a, sizeof(a));
            stats("SERVER-TX", bytes, calls, t1-t0);
        }
        free(buf);
    }
}

static void run_stream_server(enum transport_kind t, const char *ip, int port,
                              const char *cert, const char *key) {
    SSL_CTX *ctx = NULL;
    if (t == TRANSPORT_TLS) {
        ctx = SSL_CTX_new(TLS_server_method());
        if (!ctx) ssl_die("SSL_CTX_new");
        SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
        SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
        if (!cert || !key) { fprintf(stderr, "TLS server requires cert.pem key.pem\n"); exit(2); }
        if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) != 1) ssl_die("certificate");
        if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) != 1) ssl_die("private key");
    }

    int s = transport_socket(t);
    struct sockaddr_in a;
    fill_addr(&a, ip, port);
    int one=1;
    if (t == TRANSPORT_TCP || t == TRANSPORT_TLS)
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(s, (struct sockaddr*)&a, sizeof(a)) < 0) die("bind");
    if (listen(s, 16) < 0) die("listen");
    printf("path bench server transport=%s %s:%d\n", transport_name(t), ip, port);
    fflush(stdout);

    for (;;) {
        int fd = accept(s, NULL, NULL);
        if (fd < 0) { if (errno == EINTR) continue; die("accept"); }
        SSL *ssl = NULL;
        if (t == TRANSPORT_TLS) {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, fd);
            uint64_t h0=ns_now();
            if (SSL_accept(ssl) != 1) ssl_die("SSL_accept");
            uint64_t h1=ns_now();
            printf("TLS-SERVER-HANDSHAKE time=%.3f ms version=%s cipher=%s\n",
                   (double)(h1-h0)/1e6, SSL_get_version(ssl), SSL_get_cipher(ssl));
        }
        stream_server_conn(fd, ssl);
        if (ssl) { SSL_shutdown(ssl); SSL_free(ssl); }
        close(fd);
    }
}

static void run_stream_client(enum transport_kind t, const char *ip, int port,
                              size_t chunk, uint32_t count, int rounds) {
    int fd = transport_socket(t);
    struct sockaddr_in a;
    fill_addr(&a, ip, port);

    uint64_t c0=ns_now();
    if (connect(fd, (struct sockaddr*)&a, sizeof(a)) < 0) die("connect");
    uint64_t c1=ns_now();

    SSL_CTX *ctx=NULL;
    SSL *ssl=NULL;
    uint64_t h0=0,h1=0;
    if (t == TRANSPORT_TLS) {
        ctx=SSL_CTX_new(TLS_client_method());
        if(!ctx) ssl_die("SSL_CTX_new");
        SSL_CTX_set_min_proto_version(ctx,TLS1_3_VERSION);
        SSL_CTX_set_max_proto_version(ctx,TLS1_3_VERSION);
        SSL_CTX_set_verify(ctx,SSL_VERIFY_NONE,NULL); /* benchmark only */
        ssl=SSL_new(ctx);
        SSL_set_fd(ssl,fd);
        h0=ns_now();
        if(SSL_connect(ssl)!=1) ssl_die("SSL_connect");
        h1=ns_now();
        printf("TLS-CLIENT-HANDSHAKE time=%.3f ms version=%s cipher=%s\n",
               (double)(h1-h0)/1e6,SSL_get_version(ssl),SSL_get_cipher(ssl));
    }
    printf("CONNECT transport=%s tcp_connect=%.3f ms",
           transport_name(t),(double)(c1-c0)/1e6);
    if (ssl) printf(" tls_handshake=%.3f ms",(double)(h1-h0)/1e6);
    printf("\n");

    char *buf=malloc(chunk);
    if(!buf) die("malloc");
    memset(buf,0x5A,chunk);
    printf("path bench client transport=%s %s:%d chunk=%zu count=%u total=%.2f MiB rounds=%d\n",
           transport_name(t),ip,port,chunk,count,(double)chunk*count/1048576.0,rounds);

    for(int r=1;r<=rounds;r++){
        printf("\nROUND %d/%d\n",r,rounds);
        struct ctrl c={MAGIC,MODE_TX,(uint32_t)chunk,count};
        send_exact(fd,ssl,&c,sizeof(c));
        uint64_t t0=ns_now();
        for(uint32_t i=0;i<count;i++) send_exact(fd,ssl,buf,chunk);
        uint64_t ts=ns_now();
        struct ack a1;
        recv_exact(fd,ssl,&a1,sizeof(a1));
        uint64_t td=ns_now();
        uint64_t bytes=(uint64_t)chunk*count;
        stats("CLIENT-TX-send",bytes,count,ts-t0);
        stats("CLIENT-TX-drain",bytes,count,td-t0);
        stats("PEER-RX",a1.bytes,a1.calls,a1.ns);

        c.mode=MODE_RX;
        send_exact(fd,ssl,&c,sizeof(c));
        uint64_t rb=0,rc=0,rt0=0,rt1=0,target=bytes;
        while(rb<target){
            size_t want=chunk;
            uint64_t left=target-rb;
            if(left<want) want=(size_t)left;
            ssize_t n=stream_recv(fd,ssl,buf,want);
            if(n<=0){fprintf(stderr,"RX failed %zd\n",n);exit(3);}
            if(rc==0) rt0=ns_now();
            rb+=(uint64_t)n;
            rc++;
        }
        rt1=ns_now();
        struct ack a2={MAGIC,MODE_RX,rb,rt1-rt0,rc};
        send_exact(fd,ssl,&a2,sizeof(a2));
        stats("CLIENT-RX",rb,rc,rt1-rt0);
    }
    struct ctrl q={MAGIC,MODE_QUIT,0,0};
    send_exact(fd,ssl,&q,sizeof(q));
    free(buf);
    if(ssl){SSL_shutdown(ssl);SSL_free(ssl);SSL_CTX_free(ctx);}
    close(fd);
}

/*
 * Datagram benchmark:
 * Each datagram is one payload chunk. The server ACKs TX after count datagrams,
 * and for RX sends count datagrams back. This keeps message boundaries intact.
 */
static void run_dgram_server(enum transport_kind t,const char *ip,int port) {
    int fd=transport_socket(t);
    struct sockaddr_in a,peer;
    fill_addr(&a,ip,port);
    if(bind(fd,(struct sockaddr*)&a,sizeof(a))<0) die("bind");
    printf("path bench server transport=%s %s:%d\n",transport_name(t),ip,port);
    fflush(stdout);

    char *buf=malloc(UDP_MAX_CHUNK);
    if(!buf) die("malloc");
    for(;;){
        socklen_t plen=sizeof(peer);
        struct ctrl c;
        ssize_t n=recvfrom(fd,&c,sizeof(c),0,(struct sockaddr*)&peer,&plen);
        if(n!=(ssize_t)sizeof(c) || c.magic!=MAGIC) continue;
        if(c.mode==MODE_QUIT) continue;
        if(c.chunk==0 || c.chunk>UDP_MAX_CHUNK) continue;

        uint64_t bytes=0,calls=0,t0=0,t1=0;
        if(c.mode==MODE_TX){
            for(uint32_t i=0;i<c.count;i++){
                n=recvfrom(fd,buf,c.chunk,0,NULL,NULL);
                if(n<0) die("recvfrom");
                if(calls==0)t0=ns_now();
                bytes+=(uint64_t)n; calls++;
            }
            t1=ns_now();
            struct ack a1={MAGIC,MODE_TX,bytes,t1-t0,calls};
            sendto(fd,&a1,sizeof(a1),0,(struct sockaddr*)&peer,plen);
            stats("SERVER-RX",bytes,calls,t1-t0);
        } else if(c.mode==MODE_RX){
            memset(buf,0xA5,c.chunk);
            t0=ns_now();
            for(uint32_t i=0;i<c.count;i++){
                n=sendto(fd,buf,c.chunk,0,(struct sockaddr*)&peer,plen);
                if(n<0) die("sendto");
                bytes+=(uint64_t)n; calls++;
            }
            t1=ns_now();
            struct ack a2;
            recvfrom(fd,&a2,sizeof(a2),0,NULL,NULL);
            stats("SERVER-TX",bytes,calls,t1-t0);
        }
    }
}

static void run_dgram_client(enum transport_kind t,const char *ip,int port,
                             size_t chunk,uint32_t count,int rounds) {
    if(chunk==0 || chunk>UDP_MAX_CHUNK){
        fprintf(stderr,"datagram chunk must be 1..%u (requested %zu)\n",UDP_MAX_CHUNK,chunk);
        exit(2);
    }
    int fd=transport_socket(t);
    struct sockaddr_in a;
    fill_addr(&a,ip,port);
    if(connect(fd,(struct sockaddr*)&a,sizeof(a))<0) die("connect");
    char *buf=malloc(chunk);
    if(!buf)die("malloc");
    memset(buf,0x5A,chunk);
    printf("path bench client transport=%s %s:%d chunk=%zu count=%u total=%.2f MiB rounds=%d\n",
           transport_name(t),ip,port,chunk,count,(double)chunk*count/1048576.0,rounds);

    for(int r=1;r<=rounds;r++){
        printf("\nROUND %d/%d\n",r,rounds);
        struct ctrl c={MAGIC,MODE_TX,(uint32_t)chunk,count};
        if(send(fd,&c,sizeof(c),0)!=(ssize_t)sizeof(c)) die("send ctrl");
        uint64_t t0=ns_now();
        for(uint32_t i=0;i<count;i++)
            if(send(fd,buf,chunk,0)!=(ssize_t)chunk) die("send datagram");
        uint64_t ts=ns_now();
        struct ack a1;
        if(recv(fd,&a1,sizeof(a1),0)!=(ssize_t)sizeof(a1)) die("recv ack");
        uint64_t td=ns_now();
        uint64_t bytes=(uint64_t)chunk*count;
        stats("CLIENT-TX-send",bytes,count,ts-t0);
        stats("CLIENT-TX-drain",bytes,count,td-t0);
        stats("PEER-RX",a1.bytes,a1.calls,a1.ns);

        c.mode=MODE_RX;
        if(send(fd,&c,sizeof(c),0)!=(ssize_t)sizeof(c)) die("send ctrl");
        uint64_t rb=0,rc=0,rt0=0,rt1=0;
        for(uint32_t i=0;i<count;i++){
            ssize_t n=recv(fd,buf,chunk,0);
            if(n<0) die("recv datagram");
            if(rc==0)rt0=ns_now();
            rb+=(uint64_t)n; rc++;
        }
        rt1=ns_now();
        struct ack a2={MAGIC,MODE_RX,rb,rt1-rt0,rc};
        if(send(fd,&a2,sizeof(a2),0)!=(ssize_t)sizeof(a2)) die("send ack");
        stats("CLIENT-RX",rb,rc,rt1-rt0);
    }
    struct ctrl q={MAGIC,MODE_QUIT,0,0};
    send(fd,&q,sizeof(q),0);
    free(buf); close(fd);
}

static void usage(const char *p) {
    fprintf(stderr,
      "STCP transport path benchmark\n"
      "\n"
      "Usage:\n"
      "  %s server <transport> [bind-ip] [port] [cert.pem] [key.pem]\n"
      "  %s client <transport> <server-ip> [port] [chunk] [count] [rounds]\n"
      "  %s --help\n"
      "\n"
      "Transports:\n"
      "  tcp        Native TCP\n"
      "  stcp-tcp   STCP stream transport (alias: stcp)\n"
      "  tls        TLS 1.3 over native TCP (alias: tls-tcp)\n"
      "  udp        Native UDP\n"
      "  stcp-udp   STCP datagram transport\n"
      "\n"
      "Defaults:\n"
      "  port       19953\n"
      "  chunk      128044 bytes for tcp/stcp-tcp/stcp-udp/tls\n"
      "             48000 bytes for native udp\n"
      "  count      256\n"
      "  rounds     3\n"
      "\n"
      "Datagram limit:\n"
      "  native udp chunk must be 1..60000 bytes.\n"
      "  stcp-udp uses AF_STCP + SOCK_STREAM + protocol 254.\n"
      "\n"
      "TLS server:\n"
      "  cert.pem and key.pem are required after the port.\n"
      "  The benchmark forces TLS 1.3. Client certificate verification is\n"
      "  disabled because this program measures transport performance only.\n"
      "\n"
      "Examples:\n"
      "  %s server tcp 0.0.0.0 19953\n"
      "  %s client tcp 192.168.1.20 19953 128044 256 3\n"
      "\n"
      "  %s server stcp-tcp 0.0.0.0 19953\n"
      "  %s client stcp-tcp 192.168.1.20 19953 128044 256 3\n"
      "\n"
      "  %s server tls 0.0.0.0 19954 cert.pem key.pem\n"
      "  %s client tls 192.168.1.20 19954 128044 256 3\n"
      "\n"
      "  %s server udp 0.0.0.0 19955\n"
      "  %s client udp 192.168.1.20 19955 48000 256 3\n"
      "\n"
      "  %s server stcp-udp 0.0.0.0 19956\n"
      "  %s client stcp-udp 192.168.1.20 19956 128044 256 3\n"
      "\n"
      "TLS test certificate example:\n"
      "  openssl req -x509 -newkey rsa:2048 -nodes -keyout key.pem "
      "-out cert.pem -days 1 -subj '/CN=stcp-benchmark'\n",
      p,p,p,p,p,p,p,p,p,p,p,p,p);
    exit(2);
}

int main(int argc,char **argv) {
    signal(SIGPIPE,SIG_IGN);
    SSL_library_init();
    SSL_load_error_strings();

    if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        usage(argv[0]);
    }
    if(argc<3) usage(argv[0]);
    enum transport_kind t=parse_transport(argv[2]);

    if(!strcmp(argv[1],"server")){
        const char *ip=argc>3?argv[3]:"0.0.0.0";
        int port=argc>4?atoi(argv[4]):19953;
        if(is_datagram(t)) run_dgram_server(t,ip,port);
        else run_stream_server(t,ip,port,argc>5?argv[5]:NULL,argc>6?argv[6]:NULL);
        return 0;
    }
    if(!strcmp(argv[1],"client")){
        if(argc<=3) usage(argv[0]);
        size_t defchunk=is_datagram(t)?48000:128044;
        if(is_datagram(t))
            run_dgram_client(t,argv[3],argc>4?atoi(argv[4]):19953,
                             argc>5?strtoul(argv[5],0,0):defchunk,
                             argc>6?strtoul(argv[6],0,0):256,
                             argc>7?atoi(argv[7]):3);
        else
            run_stream_client(t,argv[3],argc>4?atoi(argv[4]):19953,
                              argc>5?strtoul(argv[5],0,0):defchunk,
                              argc>6?strtoul(argv[6],0,0):256,
                              argc>7?atoi(argv[7]):3);
        return 0;
    }
    usage(argv[0]);
}
