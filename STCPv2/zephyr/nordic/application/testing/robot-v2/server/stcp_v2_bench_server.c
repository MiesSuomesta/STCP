#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef AF_STCP
#define AF_STCP 45
#endif
#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif
#define BENCH_MAGIC 0x42454e32U
#define BENCH_VERSION 1U
#define MODE_UPLOAD 1U
#define MODE_DOWNLOAD 2U
#define MODE_FULL 3U

struct __attribute__((packed)) request {
    uint32_t magic, version, mode, chunk_size, total_bytes;
};
struct __attribute__((packed)) reply {
    uint32_t magic, mode, status, bytes_received, bytes_sent;
};

static volatile sig_atomic_t stop_flag;
static void on_signal(int sig) { (void)sig; stop_flag = 1; }

static int read_all(int fd, void *buf, size_t len) {
    uint8_t *p=buf; size_t off=0;
    while (off<len) { ssize_t n=read(fd,p+off,len-off); if(n>0){off+=(size_t)n;continue;} if(n==0)return -ECONNRESET; if(errno==EINTR)continue; return -errno; }
    return 0;
}
static int write_all(int fd, const void *buf, size_t len) {
    const uint8_t *p=buf; size_t off=0;
    while (off<len) { ssize_t n=send(fd,p+off,len-off,MSG_NOSIGNAL); if(n>0){off+=(size_t)n;continue;} if(n==0)return -EPIPE; if(errno==EINTR)continue; return -errno; }
    return 0;
}
static uint8_t pattern(uint32_t offset, uint8_t salt) { return (uint8_t)((offset*31U+salt)&0xffU); }
static int receive_pattern(int fd, uint32_t total, uint32_t chunk, uint8_t salt) {
    uint8_t *buf=malloc(chunk); if(!buf)return -ENOMEM; uint32_t off=0; int rc=0;
    while(off<total){ size_t want=chunk; if(want>total-off)want=total-off; ssize_t n=read(fd,buf,want); if(n<=0){rc=n==0?-ECONNRESET:-errno;break;} for(ssize_t i=0;i<n;i++){if(buf[i]!=pattern(off+(uint32_t)i,salt)){rc=-EBADMSG;goto out;}} off+=(uint32_t)n; }
out: free(buf); return rc;
}
static int send_pattern(int fd, uint32_t total, uint32_t chunk, uint8_t salt) {
    uint8_t *buf=malloc(chunk); if(!buf)return -ENOMEM; uint32_t off=0; int rc=0;
    while(off<total){ uint32_t n=chunk; if(n>total-off)n=total-off; for(uint32_t i=0;i<n;i++)buf[i]=pattern(off+i,salt); rc=write_all(fd,buf,n); if(rc<0)break; off+=n; }
    free(buf); return rc;
}
static int send_reply(int fd,uint32_t mode,uint32_t status,uint32_t br,uint32_t bs){ struct reply r={htonl(BENCH_MAGIC),htonl(mode),htonl(status),htonl(br),htonl(bs)}; return write_all(fd,&r,sizeof(r)); }
static int handle(int fd) {
    struct request q; int rc=read_all(fd,&q,sizeof(q)); if(rc<0)return rc;
    uint32_t magic=ntohl(q.magic), ver=ntohl(q.version), mode=ntohl(q.mode), chunk=ntohl(q.chunk_size), total=ntohl(q.total_bytes);
    fprintf(stderr,"[server] request mode=%u chunk=%u total=%u\n",mode,chunk,total);
    if(magic!=BENCH_MAGIC||ver!=BENCH_VERSION||chunk==0||total==0)return -EPROTO;
    if(mode==MODE_UPLOAD){ rc=receive_pattern(fd,total,chunk,0x17); if(rc<0)return rc; return send_reply(fd,mode,0,total,0); }
    if(mode==MODE_DOWNLOAD){ rc=send_pattern(fd,total,chunk,0x53); if(rc<0)return rc; struct reply ack; rc=read_all(fd,&ack,sizeof(ack)); if(rc<0)return rc; return send_reply(fd,mode,0,0,total); }
    if(mode==MODE_FULL){
        /* For the Robot smoke matrix use modest payloads. Interleave chunks to avoid pipe deadlock. */
        uint8_t *rx=malloc(chunk), *tx=malloc(chunk); if(!rx||!tx){free(rx);free(tx);return -ENOMEM;} uint32_t ro=0,so=0;
        while(ro<total || so<total){
            if(so<total){uint32_t n=chunk;if(n>total-so)n=total-so;for(uint32_t i=0;i<n;i++)tx[i]=pattern(so+i,0x53);rc=write_all(fd,tx,n);if(rc<0)break;so+=n;}
            if(ro<total){uint32_t n=chunk;if(n>total-ro)n=total-ro;ssize_t got=read(fd,rx,n);if(got<=0){rc=got==0?-ECONNRESET:-errno;break;}for(ssize_t i=0;i<got;i++){if(rx[i]!=pattern(ro+(uint32_t)i,0x17)){rc=-EBADMSG;break;}}if(rc<0)break;ro+=(uint32_t)got;}
        }
        free(rx);free(tx); if(rc<0)return rc; struct reply ack;rc=read_all(fd,&ack,sizeof(ack));if(rc<0)return rc;return send_reply(fd,mode,0,total,total);
    }
    return -EINVAL;
}
int main(int argc,char **argv){
    const char *bind_ip=argc>1?argv[1]:"0.0.0.0"; int port=argc>2?atoi(argv[2]):19000;
    signal(SIGINT,on_signal);signal(SIGTERM,on_signal);
    int s=socket(AF_STCP,SOCK_STREAM,IPPROTO_STCP);if(s<0){perror("socket(AF_STCP)");return 2;}
    int one=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
    struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons((uint16_t)port)};if(inet_pton(AF_INET,bind_ip,&a.sin_addr)!=1){fprintf(stderr,"bad bind ip\n");return 2;}
    if(bind(s,(struct sockaddr*)&a,sizeof(a))<0){perror("bind");return 2;}if(listen(s,8)<0){perror("listen");return 2;}
    fprintf(stderr,"[server] STCPv2 BEN2 listening %s:%d\n",bind_ip,port);fflush(stderr);
    while(!stop_flag){int c=accept(s,NULL,NULL);if(c<0){if(errno==EINTR)continue;perror("accept");break;}int rc=handle(c);fprintf(stderr,"[server] client rc=%d\n",rc);close(c);}close(s);return 0;
}
