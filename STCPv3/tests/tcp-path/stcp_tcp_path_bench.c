#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
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
#define MAGIC 0x53544350u
#define MODE_TX 1u
#define MODE_RX 2u
#define MODE_QUIT 3u

struct ctrl { uint32_t magic, mode, chunk, count; };
struct ack { uint32_t magic, mode; uint64_t bytes; uint64_t ns; uint64_t calls; };

static uint64_t ns_now(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec*1000000000ull + (uint64_t)ts.tv_nsec;
}
static void die(const char *s) { perror(s); exit(1); }
static void send_msg(int fd, const void *buf, size_t len) {
    const char *p = buf;
    size_t done = 0;
    while(done < len) {
        ssize_t n;
        do n = send(fd, p + done, len - done, 0); while(n < 0 && errno == EINTR);
        if(n < 0) die("send");
        if(n == 0) { fprintf(stderr, "send returned 0\n"); exit(2); }
        done += (size_t)n;
    }
}
static ssize_t recv_msg(int fd, void *buf, size_t len) {
    ssize_t n; do n=recv(fd,buf,len,0); while(n<0 && errno==EINTR); return n;
}
static int recv_exact_msg(int fd, void *buf, size_t len) {
    char *p = buf;
    size_t done = 0;
    while(done < len) {
        ssize_t n = recv_msg(fd, p + done, len - done);
        if(n < 0) die("recv");
        if(n == 0) {
            if(done == 0) return 0;
            fprintf(stderr, "EOF after %zu/%zu bytes\n", done, len);
            exit(2);
        }
        done += (size_t)n;
    }
    return 1;
}
enum transport_kind { TRANSPORT_STCP, TRANSPORT_TCP };

static const char *transport_name(enum transport_kind t) {
    return t == TRANSPORT_TCP ? "tcp" : "stcp";
}
static enum transport_kind parse_transport(const char *s) {
    if (!strcmp(s, "stcp")) return TRANSPORT_STCP;
    if (!strcmp(s, "tcp")) return TRANSPORT_TCP;
    fprintf(stderr, "bad transport: %s (expected tcp or stcp)\n", s);
    exit(2);
}
static int transport_socket(enum transport_kind t) {
    int fd = t == TRANSPORT_TCP
        ? socket(AF_INET, SOCK_STREAM, 0)
        : socket(AF_STCP, SOCK_STREAM, STCP_TCP_PROTO);
    if(fd < 0) die(t == TRANSPORT_TCP ? "socket(AF_INET)" : "socket(AF_STCP)");
    return fd;
}
static void fill_addr(struct sockaddr_in *a, const char *ip, int port) {
    memset(a,0,sizeof(*a)); a->sin_family=AF_INET; a->sin_port=htons((uint16_t)port);
    if(inet_pton(AF_INET,ip,&a->sin_addr)!=1){ fprintf(stderr,"bad ip: %s\n",ip); exit(2); }
}
static void stats(const char *tag, uint64_t bytes, uint64_t calls, uint64_t ns) {
    double sec=(double)ns/1e9, mib=(double)bytes/(1024.0*1024.0);
    printf("%-14s bytes=%"PRIu64" calls=%"PRIu64" bytes/call=%.1f time=%.6f s throughput=%.3f MiB/s calls/s=%.1f\n",
        tag,bytes,calls,calls?(double)bytes/calls:0.0,sec,sec?mib/sec:0.0,sec?(double)calls/sec:0.0);
    fflush(stdout);
}
static void server_conn(int fd) {
    for(;;){
        struct ctrl c;
        if(!recv_exact_msg(fd, &c, sizeof(c))) return;
        if(c.magic!=MAGIC) {fprintf(stderr,"bad magic\n");return;} if(c.mode==MODE_QUIT) return;
        size_t chunk=c.chunk; uint32_t count=c.count; char *buf=malloc(chunk); if(!buf) die("malloc"); memset(buf,0xA5,chunk);
        uint64_t bytes=0,calls=0,t0=0,t1=0;
        if(c.mode==MODE_TX){
            uint64_t target = (uint64_t)chunk * count;
            while(bytes < target) {
                size_t want = chunk;
                uint64_t left = target - bytes;
                if(left < want) want = (size_t)left;
                ssize_t r = recv_msg(fd, buf, want);
                if(r <= 0) { fprintf(stderr, "data recv failed %zd\n", r); free(buf); return; }
                if(calls == 0) t0 = ns_now();
                bytes += (uint64_t)r;
                calls++;
            }
            t1=ns_now(); struct ack a={MAGIC,MODE_TX,bytes,t1-t0,calls}; send_msg(fd,&a,sizeof(a));
            stats("SERVER-RX",bytes,calls,t1-t0);
        } else if(c.mode==MODE_RX){
            t0=ns_now();
            for(uint32_t i=0;i<count;i++){ send_msg(fd,buf,chunk); bytes+=chunk; calls++; }
            t1=ns_now(); struct ack a; recv_exact_msg(fd,&a,sizeof(a));
            stats("SERVER-TX",bytes,calls,t1-t0);
        }
        free(buf);
    }
}
static void run_server(enum transport_kind transport,const char *ip,int port){
    int s=transport_socket(transport); struct sockaddr_in a; fill_addr(&a,ip,port);
    int one=1; if(transport==TRANSPORT_TCP) setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
    if(bind(s,(struct sockaddr*)&a,sizeof(a))<0) die("bind");
    if(listen(s,16)<0) die("listen");
    printf("STCP TCP path bench server transport=%s %s:%d proto=%d\n",
           transport_name(transport),ip,port,transport==TRANSPORT_TCP?0:STCP_TCP_PROTO); fflush(stdout);
    for(;;){int fd=accept(s,NULL,NULL); if(fd<0){if(errno==EINTR)continue;die("accept");} server_conn(fd); close(fd);} }
static void run_client(enum transport_kind transport,const char *ip,int port,size_t chunk,uint32_t count,int rounds){
    int fd=transport_socket(transport); struct sockaddr_in a; fill_addr(&a,ip,port); if(connect(fd,(struct sockaddr*)&a,sizeof(a))<0)die("connect");
    char *buf=malloc(chunk); if(!buf)die("malloc"); memset(buf,0x5A,chunk);
    printf("STCP TCP path bench client transport=%s %s:%d chunk=%zu count=%u total=%.2f MiB rounds=%d\n",
           transport_name(transport),ip,port,chunk,count,(double)chunk*count/1048576.0,rounds);
    for(int r=1;r<=rounds;r++){
        printf("\nROUND %d/%d\n",r,rounds);
        struct ctrl c={MAGIC,MODE_TX,(uint32_t)chunk,count}; send_msg(fd,&c,sizeof(c));
        uint64_t t0=ns_now(); for(uint32_t i=0;i<count;i++) send_msg(fd,buf,chunk); uint64_t ts=ns_now();
        struct ack a1; recv_exact_msg(fd,&a1,sizeof(a1)); uint64_t td=ns_now();
        uint64_t bytes=(uint64_t)chunk*count; stats("CLIENT-TX-send",bytes,count,ts-t0); stats("CLIENT-TX-drain",bytes,count,td-t0); stats("PEER-RX",a1.bytes,a1.calls,a1.ns);
        c.mode=MODE_RX; send_msg(fd,&c,sizeof(c));
        uint64_t rb=0,rc=0,rt0=0,rt1=0;
        uint64_t target=(uint64_t)chunk*count;
        while(rb < target) {
            size_t want=chunk;
            uint64_t left=target-rb;
            if(left < want) want=(size_t)left;
            ssize_t n=recv_msg(fd,buf,want);
            if(n<=0){fprintf(stderr,"RX failed %zd\n",n);exit(3);}
            if(rc==0)rt0=ns_now();
            rb+=(uint64_t)n;
            rc++;
        }
        rt1=ns_now();
        struct ack a2={MAGIC,MODE_RX,rb,rt1-rt0,rc}; send_msg(fd,&a2,sizeof(a2)); stats("CLIENT-RX",rb,rc,rt1-rt0);
    }
    struct ctrl q={MAGIC,MODE_QUIT,0,0}; send_msg(fd,&q,sizeof(q)); free(buf); close(fd);
}
static void usage(const char *p){
    fprintf(stderr,
        "Usage:\n"
        "  %s server [tcp|stcp] [bind-ip] [port]\n"
        "  %s client [tcp|stcp] <server-ip> [port] [chunk] [count] [rounds]\n"
        "\nBackward compatible forms default to stcp.\n"
        "Defaults: port=19953 chunk=128044 count=256 rounds=3\n",p,p);
    exit(2);
}
int main(int argc,char **argv){
    signal(SIGPIPE,SIG_IGN);
    if(argc<2) usage(argv[0]);

    if(!strcmp(argv[1],"server")){
        enum transport_kind t=TRANSPORT_STCP; int base=2;
        if(argc>2 && (!strcmp(argv[2],"tcp") || !strcmp(argv[2],"stcp"))){
            t=parse_transport(argv[2]); base=3;
        }
        run_server(t, argc>base?argv[base]:"0.0.0.0",
                   argc>base+1?atoi(argv[base+1]):19953);
        return 0;
    }

    if(!strcmp(argv[1],"client")){
        enum transport_kind t=TRANSPORT_STCP; int base=2;
        if(argc>2 && (!strcmp(argv[2],"tcp") || !strcmp(argv[2],"stcp"))){
            t=parse_transport(argv[2]); base=3;
        }
        if(argc<=base) usage(argv[0]);
        run_client(t, argv[base],
                   argc>base+1?atoi(argv[base+1]):19953,
                   argc>base+2?strtoul(argv[base+2],0,0):128044,
                   argc>base+3?strtoul(argv[base+3],0,0):256,
                   argc>base+4?atoi(argv[base+4]):3);
        return 0;
    }

    usage(argv[0]);
}
