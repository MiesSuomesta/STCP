#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/random/random.h>
#include <stcp/stcp.h>
#include "mqtt_codec.h"
LOG_MODULE_REGISTER(mqtt_proxy,3);
#define HOST "lja.fi"
#define PORT "7777"
#define UP "stcp2/nrf9151/up"
#define DOWN "stcp2/nrf9151/down"
static int connect_stcp(void){struct zsock_addrinfo h={.ai_family=AF_INET,.ai_socktype=SOCK_STREAM};struct zsock_addrinfo *r=NULL;int e=zsock_getaddrinfo(HOST,PORT,&h,&r);if(e)return -EHOSTUNREACH;int fd=zsock_socket(AF_STCP,SOCK_STREAM,STCP_PROTO_TCP);if(fd<0){zsock_freeaddrinfo(r);return -errno;}if(zsock_connect(fd,r->ai_addr,r->ai_addrlen)<0){e=-errno;zsock_close(fd);zsock_freeaddrinfo(r);return e;}zsock_freeaddrinfo(r);return fd;}
int mqtt_proxy_run(void){uint32_t backoff=1000;for(;;){int fd=connect_stcp();if(fd<0){LOG_WRN("STCP connect failed: %d",fd);k_sleep(K_MSEC(backoff));backoff=MIN(backoff*2,60000U);continue;}backoff=1000;char cid[40];snprintk(cid,sizeof(cid),"nrf9151-%08x",sys_rand32_get());int rc=mqtt_send_connect(fd,cid,60);if(!rc)rc=mqtt_wait_connack(fd,15000);if(!rc)rc=mqtt_send_subscribe(fd,DOWN,1);if(rc){LOG_ERR("MQTT setup failed: %d",rc);zsock_close(fd);continue;}LOG_INF("MQTT over STCP connected to %s:%s",HOST,PORT);uint16_t pid=2;int64_t last_tx=k_uptime_get();for(;;){uint8_t type,flags,b[512];size_t len;rc=mqtt_recv_packet(fd,&type,&flags,b,sizeof(b),&len,1000);if(rc==-ETIMEDOUT){if(k_uptime_get()-last_tx>30000){rc=mqtt_send_pingreq(fd);last_tx=k_uptime_get();if(rc)break;}continue;}if(rc<0)break;if(type==3&&len>=2){size_t tlen=((size_t)b[0]<<8)|b[1];if(2+tlen>len)break;size_t pos=2+tlen;uint16_t rxid=0;if(flags&0x06){if(pos+2>len)break;rxid=((uint16_t)b[pos]<<8)|b[pos+1];pos+=2;}size_t plen=len-pos;LOG_INF("MQTT downlink bytes=%u",(unsigned)plen);rc=mqtt_send_publish_qos1(fd,UP,b+pos,plen,pid++);last_tx=k_uptime_get();if(rxid)mqtt_send_puback(fd,rxid);if(rc)break;}}
LOG_WRN("MQTT/STCP disconnected: %d",rc);zsock_close(fd);} }
