#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
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
#define BEN2 0x42454e32U
#define BUD2 0x42554432U
#define MTR2 0x4d545232U
struct io {
    int fd, datagram, plain_udp;
    struct sockaddr_in peer;
    uint32_t run, tx_seq, rx_seq;
    uint8_t pending[2081]; size_t pending_len;
};
struct direction { uint64_t bytes, calls, us; };
static uint64_t now_us(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t);
    return (uint64_t)t.tv_sec*1000000 + (uint64_t)t.tv_nsec/1000;
}
static uint32_t word(const void *p) { uint32_t v; memcpy(&v,p,4); return ntohl(v); }
static uint64_t wide(const uint8_t *p) { return ((uint64_t)word(p)<<32)|word(p+4); }
static uint8_t pattern(uint32_t off,uint8_t salt) {return (uint8_t)(off*31U+salt);}
static ssize_t tx(struct io *io,const void *data,size_t n) {
    if (!io->datagram) return send(io->fd,data,n,MSG_NOSIGNAL);
    if (!n || n>1024) {errno=EMSGSIZE;return -1;}
    uint8_t packet[1040];
    uint32_t h[4]={htonl(BUD2),htonl(io->run),htonl(io->tx_seq),htonl((uint32_t)n)};
    memcpy(packet,h,16);memcpy(packet+16,data,n);
    ssize_t sent=io->plain_udp ? sendto(io->fd,packet,n+16,MSG_NOSIGNAL,(void*)&io->peer,sizeof(io->peer)) : send(io->fd,packet,n+16,MSG_NOSIGNAL);
    if(sent<0)return sent;
    if((size_t)sent!=n+16){errno=EIO;return -1;}
    io->tx_seq++;return (ssize_t)n;
}
static ssize_t rx(struct io *io,void *data,size_t capacity,int first) {
    if(!io->datagram)return recv(io->fd,data,capacity,0);
    uint8_t datagram[1041], *packet = io->plain_udp ? datagram : io->pending;
    struct sockaddr_in peer; socklen_t size=sizeof(peer);
    for (;;) {
        ssize_t n;
        if(io->plain_udp) {
            n=recvfrom(io->fd,packet,1041,0,(void*)&peer,&size);
            if(n<0)return n;
            if(!first && (peer.sin_addr.s_addr!=io->peer.sin_addr.s_addr || peer.sin_port!=io->peer.sin_port))continue;
        } else {
            for (;;) {
                if(io->pending_len>=16) {
                    uint32_t len=word(packet+12);
                    if(!len || len>1024){errno=EBADMSG;return -1;}
                    if(io->pending_len>=len+16){n=(ssize_t)len+16;break;}
                }
                ssize_t got=recv(io->fd,packet+io->pending_len,1041,0);
                if(got<=0)return got;
                io->pending_len+=(size_t)got;
            }
        }
        if(n<16 || word(packet)!=BUD2) {if(first && io->plain_udp)continue;errno=EBADMSG;return -1;}
        uint32_t run=word(packet+4),seq=word(packet+8),len=word(packet+12);
        if(first && (seq || len!=20 || n!=36)) {if(io->plain_udp)continue;errno=EBADMSG;return -1;}
        if(!first && io->plain_udp && run!=io->run)continue;
        if(!len || len>1024 || len>(uint32_t)capacity || n!=(ssize_t)len+16 ||
           (!first && (run!=io->run || seq!=io->rx_seq))) {errno=EBADMSG;return -1;}
        if(first){io->run=run;io->rx_seq=0;if(io->plain_udp)io->peer=peer;}
        memcpy(data,packet+16,len);io->rx_seq++;
        if(!io->plain_udp){io->pending_len-=(size_t)n;memmove(packet,packet+n,io->pending_len);}
        return len;
    }
}
static int read_all(struct io *io,void *buf,size_t n) {
    size_t off=0;while(off<n){ssize_t r=rx(io,(uint8_t*)buf+off,n-off,0);
        if(r>0){off+=(size_t)r;continue;}if(r<0 && errno==EINTR)continue;
        return r==0 ? -ECONNRESET : -errno;}return 0;
}
static int write_all(struct io *io,const void *buf,size_t n,struct direction *d) {
    size_t off=0;while(off<n){ssize_t r=tx(io,(const uint8_t*)buf+off,n-off);if(d)d->calls++;
        if(r>0){off+=(size_t)r;if(d)d->bytes+=(uint64_t)r;continue;}
        if(r<0 && errno==EINTR)continue;
        return r==0 ? -EPIPE : -errno;}return 0;
}
struct tx_job {struct io *io;uint32_t total,chunk;struct direction d;int rc;};
static void *send_pattern(void *arg) {
    struct tx_job *j=arg;uint8_t data[1024];uint32_t off=0;uint64_t start=now_us();
    while(off<j->total){uint32_t n=j->chunk;if(n>1024)n=1024;if(n>j->total-off)n=j->total-off;
        for(uint32_t i=0;i<n;i++)data[i]=pattern(off+i,0x53);
        j->rc=write_all(j->io,data,n,&j->d);if(j->rc)break;off+=n;}
    j->d.us=now_us()-start;return NULL;
}
static int receive_pattern(struct io *io,uint32_t total,uint32_t chunk,struct direction *d) {
    /* TCP may combine chunks; UDP envelopes must fit the negotiated app chunk. */
    uint8_t *buf=malloc(chunk);if(!buf)return -ENOMEM;
    uint32_t off=0;uint64_t start=now_us();int rc=0;
    while(off<total){size_t n=chunk;if(n>total-off)n=total-off;ssize_t r=rx(io,buf,n,0);d->calls++;
        if(r<0 && errno==EINTR)continue;
        if(r<=0){rc=r==0 ? -ECONNRESET : -errno;break;}
        d->bytes+=(uint64_t)r;
        for(ssize_t i=0;i<r;i++)if(buf[i]!=pattern(off+(uint32_t)i,0x17)){rc=-EBADMSG;break;}
        if(rc)break;
        off+=(uint32_t)r;
    }
    d->us=now_us()-start;free(buf);return rc;
}
static double rate(uint64_t n,uint64_t us){return us ? (double)n*1e6/(1048576.0*(double)us) : 0.0;}
static void result(const char *transport,uint64_t cid,uint32_t mode,uint32_t chunk,uint32_t total,int rc,
                   struct direction r,struct direction t,uint64_t elapsed,const uint8_t *metrics) {
    const char *status=rc ? "FAIL" : "PASS";
    char client[1800]="null";
    if(metrics){uint64_t v[12];for(int i=0;i<12;i++)v[i]=wide(metrics+16+i*8);
        const char *carrier=word(metrics+12)==1 ? "ethernet" : word(metrics+12)==2 ? "lte" : "unknown";
        snprintf(client,sizeof(client),"{\"carrier\":\"%s\",\"tx_bytes\":%"PRIu64",\"rx_bytes\":%"PRIu64",\"tx_us\":%"PRIu64",\"rx_us\":%"PRIu64",\"send_calls\":%"PRIu64",\"recv_calls\":%"PRIu64",\"eagain\":%"PRIu64",\"errors\":%"PRIu64",\"stack_used_bytes\":%"PRIu64",\"stack_free_bytes\":%"PRIu64",\"connect_us\":%"PRIu64",\"elapsed_us\":%"PRIu64",\"tx_mib_s\":%.6f,\"rx_mib_s\":%.6f}",carrier,v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],v[10],v[11],rate(v[0],v[2]),rate(v[1],v[3]));}
    struct timespec stamp;clock_gettime(CLOCK_REALTIME,&stamp);
    char row[3500];snprintf(row,sizeof(row),"{\"schema\":1,\"protocol\":\"BEN2\",\"version\":2,\"timestamp_ms\":%"PRIu64",\"sequence\":%"PRIu64",\"transport\":\"%s\",\"mode\":%u,\"chunk_bytes\":%u,\"total_bytes\":%u,\"status\":\"%s\",\"error\":%d,\"server\":{\"rx_bytes\":%"PRIu64",\"tx_bytes\":%"PRIu64",\"rx_window_us\":%"PRIu64",\"tx_window_us\":%"PRIu64",\"elapsed_us\":%"PRIu64",\"receive_callbacks\":%"PRIu64",\"send_callbacks\":%"PRIu64",\"rx_mib_s\":%.6f,\"tx_mib_s\":%.6f,\"aggregate_mib_s\":%.6f},\"client\":%s}",(uint64_t)stamp.tv_sec*1000+(uint64_t)stamp.tv_nsec/1000000,cid,transport,mode,chunk,total,status,rc,r.bytes,t.bytes,r.us,t.us,elapsed,r.calls,t.calls,rate(r.bytes,r.us),rate(t.bytes,t.us),rate(r.bytes+t.bytes,elapsed),client);
    printf("BEN2_RESULT %s\n",row);fflush(stdout);
    const char *path=getenv("BEN2_RESULTS");if(!path)path="zephyr-ben2.jsonl";
    FILE *f=fopen(path,"a");if(!f){perror(path);return;}fprintf(f,"%s\n",row);if(fclose(f))perror(path);
}
static int handle(struct io *io,uint64_t cid,const char *transport,const uint8_t *first) {
    uint8_t q[20],ack[20],metrics[112];int rc=0,have_metrics=0;
    struct direction received={0};struct tx_job job={.io=io};uint64_t started=now_us();
    if(first)memcpy(q,first,20);else {rc=read_all(io,q,20);if(rc)return rc;}
    uint32_t mode=word(q+8),chunk=word(q+12),total=word(q+16);
    if(word(q)!=BEN2 || word(q+4)!=2 || mode<1 || mode>3 || !chunk || chunk>1024*1024 || !total || total>256*1024*1024)return -EPROTO;
    job.total=total;job.chunk=chunk;
    if(mode==3){pthread_t thread;int e=pthread_create(&thread,NULL,send_pattern,&job);if(e)rc=-e;
        else {rc=receive_pattern(io,total,chunk,&received);if(rc && !io->plain_udp)shutdown(io->fd,SHUT_WR);
            e=pthread_join(thread,NULL);if(!rc && e)rc=-e;if(!rc)rc=job.rc;}}
    else if(mode==1)rc=receive_pattern(io,total,chunk,&received);
    else {send_pattern(&job);rc=job.rc;}
    if(!rc && mode!=1){rc=read_all(io,ack,20);
        if(!rc && (word(ack)!=BEN2 || word(ack+4)!=mode || word(ack+8) || word(ack+12)!=job.d.bytes || word(ack+16)!=received.bytes))rc=-EBADMSG;}
    if(!rc){rc=read_all(io,metrics,112);
        if(!rc && (word(metrics)!=MTR2 || word(metrics+4)!=1 || word(metrics+8)!=112 || word(metrics+12)>2 || wide(metrics+16)!=received.bytes || wide(metrics+24)!=job.d.bytes))rc=-EBADMSG;
        if(!rc){have_metrics=1;int times[]={2,3,10,11};for(size_t i=0;i<4;i++)if(wide(metrics+16+times[i]*8)>86400000000ULL)rc=-EBADMSG;
            if(wide(metrics+16+7*8))rc=-EIO;}}
    if(!rc){uint32_t reply[5]={htonl(BEN2),htonl(mode),0,htonl((uint32_t)received.bytes),htonl((uint32_t)job.d.bytes)};rc=write_all(io,reply,20,NULL);}
    result(transport,cid,mode,chunk,total,rc,received,job.d,now_us()-started,have_metrics ? metrics : NULL);return rc;
}
static void usage(const char *exe){fprintf(stderr,"%s --transport tcp|udp|stcp-tcp|stcp-udp [--bind IPv4] [--port 19000] [--timeout-ms 60000]\n",exe);}
static int number(const char *s,long min,long max){char *end;errno=0;long v=strtol(s,&end,10);return errno || *end || end==s || v<min || v>max ? -1 : (int)v;}
int main(int argc,char **argv) {
    const char *transport=NULL,*address="0.0.0.0";int port=19000,timeout=60000;
    for(int i=1;i<argc;i++){if(!strcmp(argv[i],"--transport") && i+1<argc)transport=argv[++i];
        else if(!strcmp(argv[i],"--bind") && i+1<argc)address=argv[++i];
        else if(!strcmp(argv[i],"--port") && i+1<argc)port=number(argv[++i],1,65535);
        else if(!strcmp(argv[i],"--timeout-ms") && i+1<argc)timeout=number(argv[++i],1,86400000);
        else if(!strcmp(argv[i],"--help")){usage(argv[0]);return 0;}else {usage(argv[0]);return 2;}}
    if(!transport || port<0 || timeout<0){usage(argv[0]);return 2;}
    int family=AF_INET,type=SOCK_STREAM,protocol=IPPROTO_TCP,datagram=0,plain_udp=0;
    if(!strcmp(transport,"udp")){type=SOCK_DGRAM;protocol=IPPROTO_UDP;datagram=plain_udp=1;}
    else if(!strcmp(transport,"stcp-tcp")){family=AF_STCP;protocol=253;}
    /* SOCK_STREAM+254 is the module's documented legacy ABI; payloads remain messages. */
    else if(!strcmp(transport,"stcp-udp")){family=AF_STCP;protocol=254;datagram=1;}
    else if(strcmp(transport,"tcp")){usage(argv[0]);return 2;}
    signal(SIGPIPE,SIG_IGN);int listener=socket(family,type,protocol);if(listener<0){perror("socket (STCP modes require loaded stcp.ko)");return 2;}
    int one=1,buffer=4*1024*1024;setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
    setsockopt(listener,SOL_SOCKET,SO_RCVBUF,&buffer,sizeof(buffer));setsockopt(listener,SOL_SOCKET,SO_SNDBUF,&buffer,sizeof(buffer));
    struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons((uint16_t)port)};
    if(inet_pton(AF_INET,address,&a.sin_addr)!=1){usage(argv[0]);close(listener);return 2;}
    if(bind(listener,(void*)&a,sizeof(a)) || (!plain_udp && listen(listener,16))){perror("bind/listen");close(listener);return 2;}
    fprintf(stderr,"BENCH_SERVER_READY transport=%s bind=%s port=%d\n",transport,address,port);
    uint64_t cid=0;for(;;){struct io io={.fd=listener,.datagram=datagram,.plain_udp=plain_udp};uint8_t q[20];const uint8_t *first=NULL;
        if(plain_udp){/* Idle waiting is unlimited; active run reads get bounded below. */
            struct timeval zero={0};setsockopt(listener,SOL_SOCKET,SO_RCVTIMEO,&zero,sizeof(zero));
            ssize_t n=rx(&io,q,20,1);if(n<0){perror("recvfrom");continue;}first=q;
        }else {io.fd=accept(listener,NULL,NULL);if(io.fd<0){if(errno==EINTR || errno==EAGAIN || errno==ETIMEDOUT)continue;perror("accept");break;}}
        struct timeval tv={.tv_sec=timeout/1000,.tv_usec=(timeout%1000)*1000};
        setsockopt(io.fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));setsockopt(io.fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
        if(datagram && !plain_udp){ssize_t n=rx(&io,q,20,1);if(n<0){perror("STCP UDP request");close(io.fd);continue;}first=q;}
        int rc=handle(&io,++cid,transport,first);if(rc)fprintf(stderr,"BENCH_SERVER_ERROR cid=%"PRIu64" rc=%d\n",cid,rc);
        if(!plain_udp)close(io.fd);
    }
    close(listener);return 0;
}
