#ifndef MQTT_CODEC_H
#define MQTT_CODEC_H
#include <stddef.h>
#include <stdint.h>
int mqtt_send_connect(int fd,const char *client_id,uint16_t keepalive);
int mqtt_wait_connack(int fd,int timeout_ms);
int mqtt_send_subscribe(int fd,const char *topic,uint16_t packet_id);
int mqtt_send_publish_qos1(int fd,const char *topic,const void *payload,size_t len,uint16_t packet_id);
int mqtt_send_puback(int fd,uint16_t packet_id);
int mqtt_send_pingreq(int fd);
int mqtt_recv_packet(int fd,uint8_t *type,uint8_t *flags,uint8_t *buf,size_t cap,size_t *len,int timeout_ms);
#endif
