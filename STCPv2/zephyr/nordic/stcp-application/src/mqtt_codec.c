#include <errno.h>
#include <string.h>
#include <zephyr/net/socket.h>
#include "mqtt_codec.h"
static int send_all(int fd,const uint8_t *p,size_t n){while(n){ssize_t r=zsock_send(fd,p,n,0);if(r<0)return -errno;p+=r;n-=r;}return 0;}
static size_t enc_rem(uint8_t *o,size_t n){size_t i=0;do{uint8_t b=n%128;n/=128;if(n)b|=0x80;o[i++]=b;}while(n&&i<4);return i;}
static size_t put_str(uint8_t *o,const char*s){size_t n=strlen(s);o[0]=n>>8;o[1]=n;memcpy(o+2,s,n);return n+2;}
int mqtt_send_connect(int fd,const char *id,uint16_t ka){uint8_t b[256],v[220],h[5];size_t n=0;n+=put_str(v+n,"MQTT");v[n++]=4;v[n++]=2;v[n++]=ka>>8;v[n++]=ka;n+=put_str(v+n,id);h[0]=0x10;size_t hn=1+enc_rem(h+1,n);memcpy(b,h,hn);memcpy(b+hn,v,n);return send_all(fd,b,hn+n);}
int mqtt_wait_connack(int fd,int t){uint8_t ty,fl,b[8];size_t n;int r=mqtt_recv_packet(fd,&ty,&fl,b,sizeof(b),&n,t);if(r<0)return r;return (ty==2&&n>=2&&b[1]==0)?0:-EPROTO;}
int mqtt_send_subscribe(int fd,const char *topic,uint16_t id){uint8_t b[256],p[220],h[5];size_t n=0;p[n++]=id>>8;p[n++]=id;n+=put_str(p+n,topic);p[n++]=0;h[0]=0x82;size_t hn=1+enc_rem(h+1,n);memcpy(b,h,hn);memcpy(b+hn,p,n);return send_all(fd,b,hn+n);}
int mqtt_send_publish_qos1(int fd,const char *topic,const void *payload,size_t len,uint16_t id){uint8_t b[768],h[5];size_t n=0;n+=put_str(b+n,topic);b[n++]=id>>8;b[n++]=id;if(n+len>sizeof(b)-5)return -EMSGSIZE;memcpy(b+n,payload,len);n+=len;uint8_t out[773];out[0]=0x32;size_t hn=1+enc_rem(out+1,n);memcpy(out+hn,b,n);return send_all(fd,out,hn+n);}
int mqtt_send_puback(int fd,uint16_t id){uint8_t b[]={0x40,0x02,(uint8_t)(id>>8),(uint8_t)id};return send_all(fd,b,sizeof(b));}
int mqtt_send_pingreq(int fd){uint8_t b[]={0xC0,0};return send_all(fd,b,2);}
int mqtt_recv_packet(int fd,uint8_t *type,uint8_t *flags,uint8_t *buf,size_t cap,size_t *len,int timeout_ms){struct zsock_pollfd p={.fd=fd,.events=ZSOCK_POLLIN};int r=zsock_poll(&p,1,timeout_ms);if(r==0)return -ETIMEDOUT;if(r<0)return -errno;uint8_t h;if(zsock_recv(fd,&h,1,MSG_WAITALL)!=1)return -ECONNRESET;size_t m=1,v=0;uint8_t d;do{if(zsock_recv(fd,&d,1,MSG_WAITALL)!=1)return -ECONNRESET;v+=(d&127)*m;m*=128;}while(d&128);if(v>cap)return -EMSGSIZE;if(v&&zsock_recv(fd,buf,v,MSG_WAITALL)!=(ssize_t)v)return -ECONNRESET;*type=h>>4;*flags=h&15;*len=v;return 0;}
